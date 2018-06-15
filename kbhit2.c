/* Based in part on... */
/* http://flipcode.com/archives/_kbhit_for_Linux.shtml */

#include <sys/select.h>
#include <stdio.h>
#include <termios.h>

int kbhit( void)
{
   struct timeval timeout;
   fd_set rdset;
   const int STDIN = 0;

   FD_ZERO( &rdset);
   FD_SET( STDIN, &rdset);
   timeout.tv_sec = 0;
   timeout.tv_usec = 50000;
   return( select( STDIN + 1, &rdset, NULL, NULL, &timeout));
}

int main( const int argc, const char **argv)
{
   const int STDIN = 0;
   int c = 0;
   // Use termios to turn off line buffering
   struct termios term, old_term;

   tcgetattr(STDIN, &term);
   old_term = term;
   term.c_lflag &= ~(ICANON | ECHO);
// term.c_lflag |= ECHO;
   tcsetattr(STDIN, TCSANOW, &term);

   setbuf(stdin, NULL);
   setbuf( stdout, NULL);
   while( c != 'q')
      {
      if( !kbhit( ))
         printf( ".");
      else
         {
         c = getchar( );
         printf( " %c (%d)\n", c, c);
         }
      }
   tcsetattr(STDIN, TCSANOW, &old_term);
   return( 0);
}
