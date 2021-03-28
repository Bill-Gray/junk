#define NCURSES_WIDECHAR 1

#include <curses.h>
#include <wchar.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

int wgetnstr_ex(WINDOW *win, char *str, int *loc, int maxlen, const int size);
int wgetn_wstr_ex(WINDOW *win, wint_t *wstr, int *loc, const int maxlen, const int size);
int getnstr_ex( char *str, int *loc, int maxlen, const int size);
int getn_wstr_ex( wint_t *wstr, int *loc, const int maxlen, const int size);

#define MAXLINE 255
#define _ECHAR     0x08  /* Erase char       (^H) */
#define _DWCHAR    0x17  /* Delete Word char (^W) */
#define _DLCHAR    0x15  /* Delete Line char (^U) */
#define _ESCAPE    0x1B
#define _TAB       0x09

/* "Extended" wgetn_wstr(),  for both ncurses and PDCurses.  You can
supply an initial string,  location within that string,  and the maximum
number of columns to be consumed on-screen (the text will be clipped
suitably to keep the cursor visible). You can switch to insert mode and
use the cursor keys to move backward/forward within the text and
reposition the cursor using the mouse.

   Other differences :

   -- 'maxlen' includes the trailing '\0' character,  unlike wgetn_wstr().

   -- If Escape is hit,  or any function key,  or the mouse is clicked
      outside the text area,  that key is returned.  The calling routine
      can then handle that key/click and re-start this function if desired.

   -- Unfortunately,  there's no way to tell if echo,  cbreak,  or nodelay
      have been called without getting into Curses internals.  So these are
      only restored in PDCurses.

   -- Currently in wide-char form only.  That's the only one I actually use.

To do : return when Shift-Tab,  etc. are hit;  fullwidth/combining characters. */

static bool _insert_mode = false;

int wgetn_wstr_ex(WINDOW *win, wint_t *wstr, int *loc, const int maxlen, const int size)
{
    int i, x, y, offset = 0;
    int rval = -1;
#ifdef __PDCURSES__
    const bool oldcbreak = SP->cbreak; /* remember states */
    const bool oldecho = SP->echo;
    const bool oldnodelay = win->_nodelay;
#endif

    assert( win);
    assert( wstr);
    if (!win || !wstr)
        return ERR;

    x = getcurx( win);
    y = getcury( win);

    noecho( );              /* we do echo ourselves */
    cbreak();               /* ensure each key is returned immediately */
    nodelay( win, FALSE) ;  /* don't return -1 */

    while( rval < 0)
    {
        int len = 0, wget_wch_rval;
        wint_t ch;

        move( y, x);
        while( len < maxlen && wstr[len])
            len++;
        if( len == maxlen)
            return( -1);
        if( *loc < offset)
            offset = *loc;
        else if( offset < *loc - size + 1)
           offset = *loc + 1 - size;
        for( i = 0; i < size; i++)
            waddch( win, (i + offset >= len) ? (chtype)' ' : (chtype)wstr[i + offset]);
        move( y, x + *loc - offset);
        wrefresh(win);

        wget_wch_rval = wget_wch( win, &ch);
        if( wget_wch_rval == OK && ch == _ECHAR)
        {
            wget_wch_rval = KEY_CODE_YES;
            ch = KEY_BACKSPACE;
        }

        if( wget_wch_rval == KEY_CODE_YES)
            switch( ch)
            {
            case KEY_DC:             /* delete char */
               if( wstr[*loc])
                   for( i = *loc + 1; i <= len; i++)
                       wstr[i - 1] = wstr[i];
               break;

            case KEY_BACKSPACE:        /* CTRL-H -- Delete character */
               if( *loc)
               {
                   for( i = *loc; i <= len; i++)
                       wstr[i - 1] = wstr[i];
                   (*loc)--;
               }
               break;

            case KEY_LEFT:
               if( *loc)
                  (*loc)--;
               break;

            case KEY_RIGHT:
               if( wstr[*loc])
                  (*loc)++;
               break;

            case KEY_IC:
               _insert_mode = !_insert_mode;
               break;

            case KEY_HOME:
               *loc = 0;
               break;

            case KEY_END:
               *loc = len;
               break;

            case KEY_MOUSE:
               {
               MEVENT mouse_event;

#ifdef __PDCURSES__
               nc_getmouse( &mouse_event);
#else
               getmouse( &mouse_event);
#endif
               if( (mouse_event.bstate & BUTTON1_CLICKED) && mouse_event.y == y
                    && mouse_event.x >= x && mouse_event.x < x + size)
                  {
                  *loc = offset + mouse_event.x - x;
                  if( *loc > len)
                      *loc = len;
                  }
               else
                  rval = ch;
               }
               break;

            default:
               rval = ch;
               break;
            }

        if( wget_wch_rval == OK)
            switch( ch)
            {
            case _DLCHAR:       /* CTRL-U -- Delete line */
               *loc = 0;
               wstr[0] = '\0';
               break;

            case _DWCHAR:       /* CTRL-W -- Delete word */
               break;

            case '\n':
            case '\r':
                rval = 0;
                break;

            case _ESCAPE:
            case _TAB:
                rval = ch;
                break;

            default:
                if( ch >= ' ')
                {
                    if( (!_insert_mode && *loc < maxlen - 1)
                        || (_insert_mode && len < maxlen - 1))
                    {
                        if( *loc == len)    /* at end of line */
                            wstr[len + 1] = 0;
                        else if( _insert_mode)
                        {
                            for( i = len; i >= *loc; i--)
                                wstr[i + 1] = wstr[i];
                        }
                        wstr[*loc] = ch;
                        (*loc)++;
                    }
                    else
                        beep( );
                }
                break;
            }
    }
#ifdef __PDCURSES__
    SP->cbreak = oldcbreak;     /* restore states */
    SP->echo = oldecho;
    win->_nodelay = oldnodelay;
#endif
    return rval;
}

int wgetnstr_ex(WINDOW *win, char *str, int *loc, int maxlen, const int size)
{
    wchar_t wstr[MAXLINE + 1];
    const wchar_t *wptr = wstr;
    const char *strptr = str;
    mbstate_t ps;
    int rval;

    if (maxlen < 0 || maxlen > MAXLINE)
        maxlen = MAXLINE;

    memset( &ps, 0, sizeof( ps));
    if( (ssize_t)mbsrtowcs( wstr, &strptr, maxlen, &ps) < 0)
        return( -1);

    rval = wgetn_wstr_ex(win, (wint_t *)wstr, loc, maxlen, size);
    memset( &ps, 0, sizeof( ps));
    if( (ssize_t)wcsrtombs(str, &wptr, maxlen, &ps) < 0)
        return( -1);
    return( rval);
}

int getnstr_ex( char *str, int *loc, int maxlen, const int size)
{
    return( wgetnstr_ex( stdscr, str, loc, maxlen, size));
}

int getn_wstr_ex( wint_t *wstr, int *loc, const int maxlen, const int size)
{
    return( wgetn_wstr_ex( stdscr, wstr, loc, maxlen, size));
}
