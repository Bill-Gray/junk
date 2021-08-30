#define NCURSES_WIDECHAR 1

#include <curses.h>
#include <wchar.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

int wgetnstr_ex(WINDOW *win, char *str, int *loc, int maxlen, const int size);
int wgetn_wstr_ex(WINDOW *win, wint_t *wstr, int *loc, const int maxlen, const int size);
int getnstr_ex( char *str, int *loc, int maxlen, const int size);
int getn_wstr_ex( wint_t *wstr, int *loc, const int maxlen, const int size);

#ifdef __cplusplus
}
#endif  /* #ifdef __cplusplus */

/* Test code for wgetnstr_ex.  See getstrex.c.  Compile with one of

gcc -Wall -Wextra -pedantic -o get_test get_test.c getstrex.c -lncursesw
gcc -Wall -Wextra -pedantic -o get_test get_test.c getstrex.c -I../PDCursesMod -DPDC_WIDE -DPDC_FORCE_UTF8 ~/lib/libpdcurses.a
x86_64-w64-mingw32-gcc -O4 -Wall -pedantic -I ~/PDCursesMod -DPDC_WIDE -DPDC_FORCE_UTF8 -DPDC_DLL_BUILD -mwindows -oget_test.exe get_test.c getstrex.c ~/PDCursesMod/wingui/pdcurses.dll
*/

#include <locale.h>
#include <stdlib.h>

int main( const int argc, const char **argv)
{
   char buff[80];
   int loc = 0, rval;
   const int max_len = (argc < 3 ? (int)sizeof( buff) : atoi( argv[2]));

   setlocale( LC_ALL, "");
   initscr( );
   raw( );
   mousemask( ALL_MOUSE_EVENTS, NULL);
   cbreak( );
   noecho( );
   clear( );
   refresh( );
   start_color( );
   init_pair( 1, COLOR_RED, COLOR_WHITE);
   nodelay( stdscr, TRUE);
   keypad( stdscr, TRUE);
   if( argc == 1)
      *buff = '\0';
   else
      strcpy( buff, argv[1]);
   loc = (int)strlen( buff);
   mvaddstr( 5, 3, "[OK]");
   mvaddstr( 5, 10, "[Cancel]");
   mvaddstr( 5, 20, "[Help]");
   mvaddstr( 3, 3, "Enter text here:");
   attrset( COLOR_PAIR( 1) | A_UNDERLINE);
   do
      {
      char tbuff[80];

      move( 3, 20);
      rval = getnstr_ex( buff, &loc, max_len, 20);
      if( rval == KEY_MOUSE)
         {
         MEVENT mouse_event;

#ifdef __PDCURSES__
         nc_getmouse( &mouse_event);
#else
         getmouse( &mouse_event);
#endif
         sprintf( tbuff, "<%d %d>    ", mouse_event.x, mouse_event.y);
         if( mouse_event.y == 5)
            {
            if( mouse_event.x >= 3 && mouse_event.x < 7)
               rval = 0;      /* OK clicked */
            if( mouse_event.x >= 10 && mouse_event.x < 18)
               rval = 27;      /* Cancel clicked */
            if( mouse_event.x >= 20 && mouse_event.x < 26)
               {        /* Help clicked */
               mvaddstr( 10, 10, "You clicked on help.");
               mvaddstr( 11, 10, "Enter text 'normally'.  Arrow keys,  home/end,  etc.");
               mvaddstr( 12, 10, "should all work.");
               }
            }
         }
      else
         sprintf( tbuff, "Got '%d'   ", rval);
      move( 7, 3);
      addstr( tbuff);
      }
      while( rval > 0 && rval != 27);
   endwin( );
   printf( "String was '%s';  location %d; rval %d\n", buff, loc, rval);
   return( 0);
}
