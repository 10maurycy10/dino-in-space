objects = main.o

main : $(objects)
	cc -o main $(objects) -lX11 -lXft

main.o : main.c
	gcc -c main.c -Wall -I/usr/include/freetype2

run : main
	./main

clean :
	rm main $(objects)
