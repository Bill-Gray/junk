all: x11copy xclipget

CFLAGS=-Wall -O3 -Wextra -pedantic

x11copy : x11copy.c
	$(CC) $(CFLAGS) -o x11copy x11copy.c -lX11 -lpthread

xclipget : xclipget.c
	$(CC) $(CFLAGS) -o xclipget xclipget.c -lX11 -lpthread

clean:
	rm xclipget x11copy
