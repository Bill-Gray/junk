#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>

/* Demonstrates low-level terminal access to keyboard and, via VT100
commands,  to the screen.  I'd not rely on this to be at all portable.
Certainly not to MS Windows (R),  and I've only tried it on Linux.
Modified from the accepted answer at

https://stackoverflow.com/questions/33025599/move-the-cursor-in-a-c-program

kbhit( ) returns -1 if no key has been hit,  and the keycode if one
has been hit.  It pauses for 1/10 seconds for a keyhit;  you can
just loop on it until the return value is >= 0 (as is done in the
'main' below).  Hitting a function or arrow or similar key will cause
27 (escape) to be returned,  followed by cryptic codes that depend
on what terminal emulation is in place.  So portability is probably
terrible.

   Further info on VT100/ANSI control sequences is at

https://www.gnu.org/software/screen/manual/html_node/Control-Sequences.html

   Also:  see 'sigwinch.c' for example of how to handle window resizing.
*/

static int kbhit(void)
{
    int c = -1;
    static struct termios term, oterm;
    const int STDIN = 0;
    struct timeval timeout;
    fd_set rdset;

    tcgetattr( STDIN, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr( STDIN, TCSANOW, &term);
    FD_ZERO( &rdset);
    FD_SET( STDIN, &rdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;
    if( select( STDIN + 1, &rdset, NULL, NULL, &timeout) > 0)
       {
       c = getchar();
       if( c < 0)
           c += 256;
       }
    tcsetattr( STDIN, TCSANOW, &oterm);
    return( c);
}

/* Code to determine console size,  from

https://stackoverflow.com/questions/6812224/getting-terminal-size-in-c-for-windows
*/

#include <sys/ioctl.h>

#define VT_NORMAL        "\033[0m"
#define VT_BOLD          "\033[1m"

#define VT_BLACK         "\033[30m"
#define VT_RED           "\033[31m"
#define VT_GREEN         "\033[32m"
#define VT_YELLOW        "\033[33m"
#define VT_BLUE          "\033[34m"
#define VT_MAGENTA       "\033[35m"
#define VT_CYAN          "\033[36m"
#define VT_WHITE         "\033[37m"
#define VT_FG_DEFAULT       "\033[39m"

#define VT_BG_BLACK         "\033[40m"
#define VT_BG_RED           "\033[41m"
#define VT_BG_GREEN         "\033[42m"
#define VT_BG_YELLOW        "\033[43m"
#define VT_BG_BLUE          "\033[44m"
#define VT_BG_MAGENTA       "\033[45m"
#define VT_BG_CYAN          "\033[46m"
#define VT_BG_WHITE         "\033[47m"
#define VT_BG_DEFAULT       "\033[49m"

#define DIM_ON        "\033[2m"
#define DOUBLE_UNDER  "\033[21m"
#define DIM_OFF       "\033[22m"

/* Code to determine the cursor location or the screen width/height has
a lot of overlap,  and is combined in this function.  If 'size' is non-zero,
ESC [ 18t is sent to request width/height;  otherwise,  ESC [ ?6n
requests the cursor location.  We skip irrelevant returned bytes,
then look for (numbers)(char)(numbers).   */

static void get_console_loc( int *x, int *y, const int size)
{
   int c, i;

   printf( size ? "\033[18t" : "\033[?6n");
   *x = *y = 0;
#ifdef JUST_SHOW_RETURNED_STRING
   while( (c = kbhit( )) > 0)
      printf( "<%d>\n", c);
   return;
#endif
   for( i = (size ? 5 : 4); i; i--)    /* Disregard four  bytes (for size) */
      c = kbhit( );                    /* or three (for cursor loc) */
   while( c >= '0' && c <= '9')
      {
      *x = *x * 10 + c - '0';
      c = kbhit( );
      }
   c = kbhit( );
   while( c >= '0' && c <= '9')
      {
      *y = *y * 10 + c - '0';
      c = kbhit( );
      }
}

int main( const int argc, const char **argv)
{
   int i, x, y, xsize, ysize, txt[10];
   int mouse_tracking = 1000;    /* "standard" mouse tracking */
   int mouse_tracking_2 = 0;
   struct winsize max;

   for( i = 1; i < argc; i++)
      if( argv[i][0] == '-')
         switch( argv[i][1])
            {
            case 'm':
               mouse_tracking = atoi( argv[i] + 2);
               break;
            case 'M':
               mouse_tracking_2 = atoi( argv[i] + 2);
               break;
            }
   setbuf( stdin, NULL);
   setbuf( stdout, NULL);
   ioctl(0, TIOCGWINSZ , &max);
   printf ("lines %d\n", max.ws_row);
   printf ("columns %d\n", max.ws_col);

   printf( VT_BLUE "This is blue." VT_NORMAL  "(Plus some 'normal' text.)\n");
   printf( VT_RED "With a red foreground. " VT_NORMAL "(And again returning to 'normal'.)\n");
   printf( VT_GREEN VT_BOLD "Bold green foreground. ");
   printf( VT_BG_BLACK VT_WHITE "White text on black.\n");
   printf( VT_BG_RED VT_YELLOW "Red background, brown foreground.\n" VT_NORMAL);
   printf( "\033[30;47m  Black foreground (background set white)\033[0m\n");

   for( i = 0; i < 2; i++)
      {
      if( i)
         printf( "\033[1mBold versions:\n");
      printf( VT_RED   "  Red foreground\n");
      printf( VT_GREEN "  Green\n");
      printf( "\033[33m  'Brown' (actually shows up yellow)\n");
      printf( "\033[34m  Blue\n");
      printf( "\033[35m  Magenta/purple\n");
      printf( VT_FG_DEFAULT "  Foreground is default color\n");
      printf( VT_BG_RED "  Still default foreground,  red background\n");
      printf( "\033[36m  Cyan\n");
      printf( "\033[37m  Gray\n");
      printf( VT_BG_BLACK "  Switch to black background\n");
      }
   printf( VT_BG_DEFAULT "  Switch to default background\n");
   printf( "\033[0m\033[5mBlinking text (if supported)\n");
   printf( "\033[0m\033[7mReverse video (if supported)\n");
   printf( "\033[10A\033[30CHey!  I moved up ten lines and right 30 columns!\n");
   printf( "\033[10BAnd now I'm back down on the next line.\n" VT_NORMAL);
   for( i = 0; i < 60; i++)
      {
      const char *text = "Text gradually goes from red fg/black bg to white fg/blue bg\n";

//    printf( "\033[38;5;%dm", i);
      printf( "\033[38;2;255;%d;%dm", i * 4, i * 4);
      printf( "\033[48;2;0;0;%dm", i * 4);
      printf( "%c", text[i]);
      }
   printf( "\n\x1b[38;2;255;100;0mTRUECOLOR\x1b[0m\n");
   for( i = 0; i < 256; i++)
      printf( "\033[48;5;%dm%d%s", i, i % 10, (i % 64 == 63) ? "\n" : "");

   printf( "\033[0mDon't forget to return to normalcy before exiting.\n");
   get_console_loc( &xsize, &ysize, 1);
   printf( "\033[4mUnderlined\033[0m text\n");
   printf( "\033[3mStandout\033[0m text\n");
   printf( "Terminal size is (%d, %d)\n", xsize, ysize);
   printf( DIM_ON "Dimmed text" VT_NORMAL "\n");
   printf( DOUBLE_UNDER "Double underlined text" VT_NORMAL "\n");
   printf( "(The above produces double underlining,  one underline,  or no underline.)\n");
   printf( "Hit keys:\n");
   printf( "\033[?%dh", mouse_tracking);
   if( mouse_tracking_2)
      printf( "\033[?%dh", mouse_tracking_2);
   do
      {
      int n = 0;

      printf( ".");
      while( n < 6 && (txt[n] = kbhit()) >= 0)
         n++;
      if( n == 6 && txt[0] == 27 && txt[1] == '[' && txt[2] == 'M')
         printf( "Mouse button %d at (%d, %d)\n",
                  txt[3], txt[4] - ' ', txt[5] - ' ');
      else if( n == 6 && txt[0] == 27 && txt[1] == '[' && txt[2] == '<')
         {
         printf( "SGR mouse <%c%c%c", txt[3], txt[4], txt[5]);
         while( n != 'm' && n != 'M')
            {
            n = kbhit( );
            printf( "%c", n);
            }
         printf( "\n");
         }
      else if( n)
         {
         printf( "(");
         for( i = 0; i < n; i++)
            printf( "%d [%c],", txt[i], (txt[i] >= ' ' && txt[i] <= 126) ? txt[i] : ' ');
         printf( ")\n");
         }
      }
      while( txt[0] != 'q');
   printf( VT_BG_BLACK "  Switch to black background\n");
   if( mouse_tracking_2)
      printf( "\033[?%dl", mouse_tracking_2);
   printf( "\033[?%dl", mouse_tracking);     /* end mouse events on xterm */
   printf( "Cursor is at ");
   get_console_loc( &x, &y, 0);
   printf( "(%d, %d)\n", x, y);
   return( 0);
}

