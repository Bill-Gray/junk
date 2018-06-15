/**
 Linux (POSIX) implementation of _kbhit().
 Morgan McGuire, morgan@cs.brown.edu
 */
/* http://flipcode.com/archives/_kbhit_for_Linux.shtml */
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    setbuf(stdin, NULL);
    printf("Press any key");
    int c;
    do
       {
          c = getchar( );
          if( c > 0)
             printf( "%c (%d)\n", c, c);
          else
             printf( ".");
          fflush(stdout);
          usleep(10000);
       } while( c != 'q');

    return 0;
}
