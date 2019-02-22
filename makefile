all: testclip pend vt100

CFLAGS=-Wall -O3 -Wextra -pedantic

.c.o:
	$(CC) $(CFLAGS) -c $<

pend : pend.c
	$(CC) $(CFLAGS) -o pend pend.c -lm

testclip : testclip.o x11copy.o xclipget.o
	$(CC) $(CFLAGS) -o testclip testclip.o x11copy.o xclipget.o -lX11 -lpthread

vt100 : vt100.o

clean:
	rm xclipget.o x11copy.o testclip.o pend testclip
