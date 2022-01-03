#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UISTATE_START 0
#define UISTATE_MENU 1
#define UISTATE_HIGHSCORE 2

int ui_state;
int framectr;
int char_width;
int char_hight;
// 0 is top 
// 3 is bottom
int ship_pos;
// flag set if the player has fired.
int has_fired;

void drawstringpixel(XftDraw* xftdraw, XftColor color, XftFont* font, int x, int y, char* msg) {
    XftDrawString8(xftdraw, &color, font, x, y, (XftChar8 *)msg, strlen(msg));
}

void drawstring(XftDraw* xftdraw, XftColor color, XftFont* font, int x, int y, char* msg) {
    drawstringpixel(
        xftdraw,color,font,
        x * char_width,
        y * char_hight + char_hight,
        msg);
}

void run(int s, Window w, Display* d) {    
    ui_state = UISTATE_START;
    framectr = 0;
    ship_pos = 0;
    
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
    
    printf("%d %d\n",char_width,char_hight);
    
    while (1) {
        XEvent e;
        has_fired = 0;
        while (XPending(d)) {
            XNextEvent(d, &e);
            if (e.type == KeyPress) {
                XKeyEvent key = e.xkey;
                if (25==key.keycode) {
                    ship_pos--;
                    if (ship_pos < 0) {
                        ship_pos = 0;
                    }
                }
                if (39==key.keycode) {
                    ship_pos ++;
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
        
        framectr++;
        XClearWindow(d,w);
        
        drawstring(xftdraw,xftcolor,font,0,5+ship_pos,">");
        if (has_fired)
            drawstring(xftdraw,xftcolor,font,1,5+ship_pos,"--------------");
        XFlush(d);
        
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
