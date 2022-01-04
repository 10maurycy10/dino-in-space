#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UISTATE_START 0
#define UISTATE_MENU 1
#define UISTATE_HIGHSCORE 2

#define SCREEN_LEN 32
#define SCREEN_HIGHT 4
#define DRAW_OFFSET 4

#define SHIP_EMPTY "▻"
#define SHIP_FULL "►"
#define AMMO "◆"
#define ROCK "□"

unsigned int rng = 539772580;
int ui_state;
int framectr;
int char_width;
int char_hight;
// 0 is top 3 is bottom 
int ship_pos;
// flag set if the player has fired.
int has_fired;
// flag whenter the player has moved since asteriod move
int has_moved;

// row major grid
int asteroids[SCREEN_HIGHT*SCREEN_LEN];
int asteroid_counters[SCREEN_HIGHT];

#define ASTEROIDS_AIR 0
#define ASTEROIDS_AMMO 1
#define ASTEROIDS_ROCK 2

// a simple and crapy prngs
unsigned int rng_extract() {
    rng += 142;
    rng /= 95;
    rng *= rng;
    return rng;
}

void rng_seed(int i) {
    rng *= i;
    rng += 42;
}

int aidx(int x, int y) {
    return x + y*SCREEN_LEN;
}

void drawstringpixel(XftDraw* xftdraw, XftColor color, XftFont* font, int x, int y, char* msg) {
    XftDrawStringUtf8(xftdraw, &color, font, x, y, (XftChar8 *)msg, strlen(msg));
}

void drawstring(XftDraw* xftdraw, XftColor color, XftFont* font, int x, int y, char* msg) {
    drawstringpixel(
        xftdraw,color,font,
        x * char_width,
        y * char_hight + char_hight,
        msg);
}

void advance_asteroids() {
    // copy all asteriods to the left, discarding left most col
    for (int x = 1; x < SCREEN_LEN; x++) {
        for (int y = 0; y < SCREEN_HIGHT; y++) {
            asteroids[aidx(x-1,y)] = asteroids[aidx(x,y)];
        }
    }
    
    for (int y = 0; y < SCREEN_HIGHT; y++) {
            asteroids[aidx(SCREEN_LEN-1,y)] = ASTEROIDS_AIR;
            if (asteroid_counters[y])
                asteroid_counters[y]--;
            else {
                asteroids[aidx(SCREEN_LEN-1,y)] = ASTEROIDS_ROCK;
                asteroid_counters[y] = rng_extract() % 10 + 3;
            }
    }
}

void run(int s, Window w, Display* d) {    
    ui_state = UISTATE_START;
    framectr = 0;
    ship_pos = 0;
    
    for (int i = 0; i < SCREEN_HIGHT*SCREEN_LEN; i++)
        asteroids[i] = ASTEROIDS_AIR;
    for (int i = 0; i < SCREEN_HIGHT; i++) {
        asteroid_counters[i] = (i*3)%5;
    }
    
    // load the system default monospace font
    XftFont* font = XftFontOpen(d,0,XFT_FAMILY, XftTypeString, "monospace",XFT_SIZE, XftTypeDouble, 12.0,NULL);
    XftDraw* xft_context = XftDrawCreate(d,w,DefaultVisual(d,0),DefaultColormap(d,0));
    
    // setup a xftcolor as #FFFFFF for text drawing
    XftColor     xftcolor;
    XRenderColor xrcolor;
    xrcolor.red  =255*255;
    xrcolor.green=255*255;
    xrcolor.blue =255*255;
    xrcolor.alpha=255*255;
    XftColorAllocValue(d,DefaultVisual(d,0),DefaultColormap(d,0),&xrcolor,&xftcolor);
    
    // setup a xft drawing context
    XftDraw      *xftdraw;
    xftdraw = XftDrawCreate(d,w,DefaultVisual(d,0),DefaultColormap(d,0));
    
    // calculate char spacing
    XGlyphInfo glyphinfo;
    XftTextExtentsUtf8(d, font, (FcChar8*)"A", strlen("A"), &glyphinfo);
    char_width = glyphinfo.xOff;
    char_hight = font->ascent + font->descent;
        
    while (1) {
        // Read inputs
        XEvent e;
        has_fired = 0;
        while (XPending(d)) {
            XNextEvent(d, &e);
            if (e.type == KeyPress) {
                // seed rng on keypress
                rng_seed(framectr);
                // prosses input
                XKeyEvent key = e.xkey;
                if (25==key.keycode && !has_moved) {
                    has_moved = 1;
                    ship_pos--;
                    if (asteroids[aidx(0, ship_pos)] == ASTEROIDS_ROCK)
                        ship_pos++;
                    if (ship_pos < 0) {
                        ship_pos = 0;
                    }
                }
                if (39==key.keycode && !has_moved) {
                    has_moved = 1;
                    ship_pos ++;
                    if (asteroids[aidx(0, ship_pos)] == ASTEROIDS_ROCK)
                        ship_pos--;
                    if (ship_pos==4) {
                        ship_pos = 3;
                    }
                }
                if (65==key.keycode) {
                    has_fired = 1;
                }
            }
            if (e.type == DestroyNotify) {
                break;
            }
        }
        
        // advance asterids if the frame is divisable by 55
        if (framectr%4 == 0) {
            advance_asteroids();
            has_moved = 0;
        }
        
        framectr++;
        // drawing code ...
        XClearWindow(d,w);
        // draw title and boaders
        drawstring(xftdraw,xftcolor,font,0,DRAW_OFFSET - 2,"XDino in space! version 0.1");
        drawstring(xftdraw,xftcolor,font,0,DRAW_OFFSET - 1,"################################");
        drawstring(xftdraw,xftcolor,font,0,DRAW_OFFSET + SCREEN_HIGHT,"################################");
        
        // draw ship
        drawstring(xftdraw,xftcolor,font,0,DRAW_OFFSET+ship_pos,SHIP_EMPTY);
        //drawstring(xftdraw,xftcolor,font,0,DRAW_OFFSET+ship_pos,SHIP_FULL);
        
        // draw asteriod feild
        for (int y = 0; y < SCREEN_HIGHT; y++)
            for (int x = 0; x < SCREEN_LEN; x++) {
                if (asteroids[aidx(x, y)] == ASTEROIDS_AMMO)
                    drawstring(xftdraw,xftcolor,font,x,DRAW_OFFSET + y,AMMO);
                if (asteroids[aidx(x, y)] == ASTEROIDS_ROCK)
                    drawstring(xftdraw,xftcolor,font,x,DRAW_OFFSET + y,ROCK);
            }
        
        // draw line and destroy asteroid
        if (has_fired)
            for (int x = 0; x < SCREEN_LEN; x++) {
                if (x < (SCREEN_LEN - 1)) {
                    if (asteroids[aidx(x+1,ship_pos)]) {
                        drawstring(xftdraw,xftcolor,font,x,DRAW_OFFSET+ship_pos,"-");
                        asteroids[aidx(x+1,ship_pos)] = ASTEROIDS_AIR;
                        break;
                    }
                }
                drawstring(xftdraw,xftcolor,font,x,DRAW_OFFSET+ship_pos,"-");
            }
        XFlush(d);
        
        if (asteroids[aidx(0,ship_pos)]) {
            return;
        }
        
        usleep(100*1000);
    }
    
    // TODO XftFont unloading
    //XUnloadFont(d,font);
    return;
}
            

int main(void) {
    Display *d;
    Window w;
    int s;

    d = XOpenDisplay(NULL);
    if (d == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    s = DefaultScreen(d);
    w = XCreateSimpleWindow(
        d, RootWindow(d, s),  // display and root window
        10, 10, 100, 100,1,  // x,y, width, hight, boader hight,
        WhitePixel(d, s),
        BlackPixel(d, s)
    );
    XSelectInput(d, w, ExposureMask | KeyPressMask);
    XMapWindow(d, w);
    
    run(s,w,d);
    
    XCloseDisplay(d);
    
    return 0;
}
