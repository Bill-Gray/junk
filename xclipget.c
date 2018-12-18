/* Shameless copy of

https://stackoverflow.com/questions/27378318/c-get-string-from-clipboard-on-linux

(except broken out as a modular function and some bug fixes... actually,  not
much of a 'copy' anymore.)

See also : https://github.com/exebook/x11clipboard . Compile with -lX11.
*/

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include "clip_fns.h"

char *get_x_selection( const int clip_type)
{
   Display *display = XOpenDisplay( 0);
   int N = DefaultScreen( display);
   unsigned long color = BlackPixel( display, N);
   Window window = XCreateSimpleWindow( display, RootWindow( display, N),
              0, 0, 1, 1, 0, color, color);
   Atom UTF8 = XInternAtom( display, "UTF8_STRING", 1);
   const Atom XA_STRING = 31;
   char *result, *rval = NULL;
   unsigned long ressize, restail;
   int resbits;
   const Atom bufid = XInternAtom(display, (clip_type ? "PRIMARY" : "CLIPBOARD"), False),
        propid = XInternAtom(display, "XSEL_DATA", False),
        incrid = XInternAtom(display, "INCR", False);
   XEvent event;

   if (UTF8 == None)
       UTF8 = XA_STRING;
   XConvertSelection(display, bufid, UTF8, propid, window, CurrentTime);
   do {
      XNextEvent(display, &event);
   } while (event.type != SelectionNotify || event.xselection.selection != bufid);

   if (event.xselection.property)
      {
      XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, False, AnyPropertyType,
             &UTF8, &resbits, &ressize, &restail, (unsigned char**)&result);

      if( UTF8 != incrid)
         {
         rval = (char *)malloc( ressize + 1);
         memcpy( rval, result, ressize);
         rval[ressize] = '\0';
         }
/*    else
         printf("Buffer is too large and INCR reading is not implemented yet.\n");
*/    XFree(result);
      XDeleteProperty( display, window, propid);
      }
   XDestroyWindow(display, window);
   XCloseDisplay(display);
   return( rval);
}
