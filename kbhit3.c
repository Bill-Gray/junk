/**
 Linux (POSIX) implementation of _kbhit().
 Morgan McGuire, morgan@cs.brown.edu
 */
/* http://flipcode.com/archives/_kbhit_for_Linux.shtml */
#include <stdio.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stropts.h>

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
    /*
        // Use termios to turn off line buffering
        struct termios term;

        tcgetattr(STDIN, &term);
        term.c_lflag &= ~(ICANON | ECHO);
        term.c_lflag |= ECHO;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    */
//  setvbuf( stdin, NULL, _IONBF, 0);
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

//////////////////////////////////////////////
//    Simple demo of _kbhit()

#include <unistd.h>

int main(int argc, char** argv) {
    printf("Press any key");
    int c;
    do
       {
       while (! _kbhit()) {
          printf(".");
          fflush(stdout);
          usleep(10000);
       }
       c = getchar( );
       printf( "%c (%d)\n", c, c);
    } while( c != 'q');

    return 0;
}
