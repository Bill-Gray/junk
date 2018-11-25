/* Shameless copy of

https://stackoverflow.com/questions/27378318/c-get-string-from-clipboard-on-linux

See also : https://github.com/exebook/x11clipboard
*/

// gcc -o xclipget xclipget.c -lX11
#include <stdio.h>
#include <limits.h>
#include <X11/Xlib.h>

Bool PrintSelection(Display *display, Window window, const char *bufname, const char *fmtname)
{
  char *result;
  unsigned long ressize, restail;
  int resbits;
  Atom bufid = XInternAtom(display, bufname, False),
       fmtid = XInternAtom(display, fmtname, False),
       propid = XInternAtom(display, "XSEL_DATA", False),
       incrid = XInternAtom(display, "INCR", False);
  XEvent event;

  XConvertSelection(display, bufid, fmtid, propid, window, CurrentTime);
  do {
    XNextEvent(display, &event);
  } while (event.type != SelectionNotify || event.xselection.selection != bufid);

  if (event.xselection.property)
  {
    XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, False, AnyPropertyType,
      &fmtid, &resbits, &ressize, &restail, (unsigned char**)&result);

    if (fmtid == incrid)
      printf("Buffer is too large and INCR reading is not implemented yet.\n");
    else
      printf("%.*s", (int)ressize, result);

    XFree(result);
    XDeleteProperty( display, window, propid);
    return True;
  }
  else // request failed, e.g. owner can't convert to the target format
  {
    printf( "Can't get format '%s'\n", fmtname);
    return False;
  }
}

int main( const int argc, const char **argv)
{
  Display *display = XOpenDisplay(NULL);
  unsigned long color = BlackPixel(display, DefaultScreen(display));
  Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0,0, 1,1, 0, color, color);
  const char *desired_buff = (argc == 1 ? "CLIPBOARD" : "PRIMARY");
  Bool result = PrintSelection(display, window, desired_buff, "UTF8_STRING") ||
                PrintSelection(display, window, desired_buff, "STRING");
  XDestroyWindow(display, window);
  XCloseDisplay(display);
  return !result;
}
