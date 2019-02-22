#include "clip_fns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void show_both_clipboards( void)
{
   char *text;

   text = get_x_selection( CLIPTYPE_CLIPBOARD);
   printf( "Clipboard: '%s'\n", text);
   free( text);
   text = get_x_selection( CLIPTYPE_PRIMARY);
   printf( "Selection: '%s'\n", text);
   free( text);
}

int main( const int argc, const char **argv)
{
   show_both_clipboards( );
   if( argc > 1)
      {
      char *tptr = (char *)malloc( strlen( argv[1]) + 1);

      strcpy( tptr, argv[1]);
      copy_text_to_clipboard( argv[1], argc > 2);
      printf( "Copied to %s selection\n",
                                   (argc > 2 ? "primary" : "clipboard"));
      printf( "Hit enter when done\n");
      getchar( );
      free( tptr);
      }
   return( 0);
}
