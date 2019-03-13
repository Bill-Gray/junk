/* copied from https://github.com/exebook/x11clipboard */

#include <limits.h>
// #include <stdbool.h>
#include <stdlib.h>
#include <string.h> // strlen
#include <X11/Xlib.h>
#include <assert.h>

/* Programs using these functions should link with -lX11 -lpthread.

   The X11 clipboard is display-specific,  so our first task is to open
the root display.  The clipboard is "owned" by a window,  so we make a
dummy window,  and try to set it as the selection owner.  That may
conceivably fail,  so we check to make sure we really got it.

   Then we go into a loop waiting for somebody to request the selection.
If they do,  and they're looking for UTF8 or ASCII text,  we give them
what they're looking for.  If we're notified that the selection has been
cleared (somebody else wants to own it),  we're done and return 0.

   Ideally,  this whole looping-waiting-for-requests should take place in
a separate thread so that the rest of our program can go on with its
work,  and the following 'XCopy_threaded' does just that.  The loop
exits when we get a 'selection cleared' message.  Close down your program,
and the thread shuts down and the selection is lost... but that's par for
the course in X;  to evade it,  use a clipboard manager.  (Which will
notice right away that you own the selection;   it will then request the
selection,  make a copy,  and claim ownership itself.  So our thread will
have a very short life.)

   'cliptype' should be either PRIMARY or CLIPBOARD.   */

static int XCopy( const char *text, const char *cliptype, long length);
int XCopy_threaded( const char *text, const char *cliptype);

static int XCopy( const char *text, const char *cliptype, long length)
{
   Display *display = XOpenDisplay( 0);
   int N = DefaultScreen( display);
   unsigned long color = BlackPixel( display, N);
   Window window = XCreateSimpleWindow( display, RootWindow( display, N),
              0, 0, 1, 1, 0, color, color);
   Atom UTF8 = XInternAtom( display, "UTF8_STRING", 1);
   const Atom XA_ATOM = 4, XA_STRING = 31;
   const Atom targets_atom = XInternAtom( display, "TARGETS", 0);
   const Atom text_atom = XInternAtom(display, "TEXT", 0);
   const Atom selection = XInternAtom(display, cliptype, 0);
   XEvent event;
   int rval = 0;

   length = (long)strlen( text);
   if (UTF8 == None)
       UTF8 = XA_STRING;
   XSetSelectionOwner( display, selection, window, 0);
   if( XGetSelectionOwner( display, selection) != window)
      rval = -1;
   else do
   {
      XNextEvent( display, &event);
      if( event.type == SelectionRequest &&
               event.xselectionrequest.selection == selection)
      {
         XSelectionRequestEvent * xsr = &event.xselectionrequest;
         XSelectionEvent ev = {0};
         int R = 0;

         ev.type = SelectionNotify, ev.display = xsr->display,
         ev.requestor = xsr->requestor, ev.selection = xsr->selection,
         ev.time = xsr->time, ev.target = xsr->target, ev.property = xsr->property;
         if (ev.target == targets_atom)
            R = XChangeProperty (ev.display, ev.requestor, ev.property, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&UTF8, 1);
         else if (ev.target == XA_STRING || ev.target == text_atom)
            R = XChangeProperty(ev.display, ev.requestor, ev.property, XA_STRING, 8,
                        PropModeReplace, (const unsigned char *)text, length);
         else if (ev.target == UTF8)
            R = XChangeProperty(ev.display, ev.requestor, ev.property, UTF8, 8,
                        PropModeReplace, (const unsigned char *)text, length);
         else
            ev.property = None;
         if ((R & 2) == 0)
            XSendEvent (display, ev.requestor, 0, 0, (XEvent *)&ev);
      }
   } while( event.type != SelectionClear);
   XDestroyWindow( display, window);
   XCloseDisplay( display);
   return( rval);
}

#include <pthread.h>

static void *XCopy_thread_func( void *clip_request)
{
   char **args = (char **)clip_request;
   const long *lenptr = (const long *)args[2];

   assert( args);
   assert( args[0]);
   assert( args[1]);
   XCopy( args[0], args[1], *lenptr);
   free( args[0]);
   return( NULL);
}

int PDC_setclipboard( const char *contents, long length)
{
   int rval;
   pthread_t unused_pthread_rval;
   static char *args[4];

   assert( contents);
   args[0] = (char *)malloc( length + 1);
   args[0][length] = '\0';
   memcpy( args[0], contents, length);
   args[1] = "CLIPBOARD";
   args[2] = (char *)&length;
   args[3] = NULL;
   rval = pthread_create( &unused_pthread_rval, NULL, XCopy_thread_func, args);
   return( rval);
}

/* Shameless copy of

https://stackoverflow.com/questions/27378318/c-get-string-from-clipboard-on-linux

(except broken out as a modular function and some bug fixes... actually,  not
much of a 'copy' anymore.)

See also : https://github.com/exebook/x11clipboard . Compile with -lX11.
*/

int PDC_getclipboard( char **contents, long *length)
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
   const Atom bufid = XInternAtom(display, "CLIPBOARD", False),
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
      else
         {
         rval = NULL;
         ressize = 0;
         }
      XFree(result);
      XDeleteProperty( display, window, propid);
      }
   XDestroyWindow(display, window);
   XCloseDisplay(display);
   *contents = rval;
   *length = (long)ressize;
   return( rval ? 0 : -1);
}

int PDC_freeclipboard( char *contents)
{
   free( contents);
   return( 0);
}

int PDC_clearclipboard( void)
{
   return( 0);
}
