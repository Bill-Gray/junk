/* copied from https://github.com/exebook/x11clipboard */

#include <string.h> // strlen
#include <X11/Xlib.h>
#include <stdio.h>

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

   'cliptype' should be either PRIMARY or CLIPBOARD'.   */

int XCopy( const char *text, const char *cliptype);
int XCopy_threaded( const char *text, const char *cliptype);

int XCopy( const char *text, const char *cliptype)
{
   Display *display = XOpenDisplay(0);
   int N = DefaultScreen(display);
   unsigned long color = BlackPixel( display, N);
   Window window = XCreateSimpleWindow(display, RootWindow(display, N), 0, 0, 1, 1, 0,
               color, color);
   const Atom targets_atom = XInternAtom(display, "TARGETS", 0);
   const Atom text_atom = XInternAtom(display, "TEXT", 0);
   Atom UTF8 = XInternAtom(display, "UTF8_STRING", 1);
   const Atom XA_ATOM = 4, XA_STRING = 31;
   const Atom selection = XInternAtom(display, cliptype, 0);
   XEvent event;
   int rval = 0;

   if (UTF8 == None)
       UTF8 = XA_STRING;
   XSetSelectionOwner (display, selection, window, 0);
   if( XGetSelectionOwner (display, selection) != window)
      rval = -1;
   else do
   {
      XNextEvent (display, &event);
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
                        PropModeReplace, (const unsigned char *)text, strlen( text));
         else if (ev.target == UTF8)
            R = XChangeProperty(ev.display, ev.requestor, ev.property, UTF8, 8,
                        PropModeReplace, (const unsigned char *)text, strlen( text));
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
   const char **args = (const char **)clip_request;
   int rval =  XCopy( args[0], args[1]);

   printf( "Done: %d\n", rval);
   return( NULL);
}

int XCopy_threaded( const char *text, const char *cliptype)
{
   int rval;
   pthread_t unused_pthread_rval;
   const char *args[3];

   args[0] = text;
   args[1] = cliptype;
   rval = pthread_create( &unused_pthread_rval, NULL, XCopy_thread_func, args);
   return( rval);
}

int main( const int argc, const char **argv)
{
   const char *selection = "CLIPBOARD";
   int rval;

   if( argc > 2)
      selection = (argv[2][0] == 'm' ? "CLIPBOARD" : "PRIMARY");
   rval = XCopy_threaded( argv[1], selection);
   printf( "Selection '%s' : %d\n", selection, rval);
   printf( "Hit enter when done\n");
   getchar( );
   return( rval);
}
