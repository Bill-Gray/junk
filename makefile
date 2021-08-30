
CC=gcc

ifdef _w64
	CC=x86_64-w64-mingw32-gcc
	EXE=.exe
endif

ifdef CLANG
	CC=clang
endif

all: pend$(EXE) vt100$(EXE) test_def$(EXE) fb fbclock psf_test$(EXE)

CFLAGS=-Wall -O3 -Wextra -pedantic

.c.o:
	$(CC) $(CFLAGS) -c $<

fb: fb.c
	$(CC) $(CFLAGS) -o fb fb.c

fbclock: fbclock.c
	$(CC) $(CFLAGS) -o fbclock fbclock.c -lz

launder: launder.c
	$(CC) $(CFLAGS) -o launder$(EXE) launder.c

pend$(EXE) : pend.c
	$(CC) $(CFLAGS) -o pend$(EXE) pend.c -lm

testclip$(EXE) : testclip.o xclip.o
	$(CC) $(CFLAGS) -o testclip$(EXE) testclip.o xclip.o -lX11 -lpthread

vt100$(EXE) : vt100.c
	$(CC) $(CFLAGS) -o vt100$(EXE) vt100.c

test_def$(EXE) : test_def.o
	$(CC) $(CFLAGS) -o test_def$(EXE) test_def.o

psf_test$(EXE) : psf_test.o psf.o
	$(CC) $(CFLAGS) -o psf_test$(EXE) psf_test.o psf.o

clean:
	-rm xclip.o testclip.o pend$(EXE) testclip$(EXE) test_def$(EXE) vt100$(EXE)
	-rm fbclock fb psf.o psf_test$(EXE)
