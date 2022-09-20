#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Code to read in UTF8 data and output it with HTML entities */

static FILE *ifile;

static int grab_utf( int c)
{
   if( !(c & 0x80))        /* plain ASCII */
      return( c);
   else if( (c & 0xe0) == 0xc0) /* two-byte UTF8 : code */
      {                               /* points U+80 to U+7FF */
      c = ((c & 0x1f) << 6) | (fgetc( ifile) & 0x3f);
      }
   else
      {           /* three-byte UTF: U+800 to U+FFFF */
      int c1 = fgetc( ifile), rval;

      rval = ((c & 0x0f) << 12) | ((c1 & 0x3f) << 6) | (fgetc( ifile) & 0x3f);
      if( (c & 0xf0) == 0xf0)           /* Four-byte UTF:  */
         {                              /* U+10000 and beyond (SMP) */
         rval = (rval << 6) | (fgetc( ifile) & 0x3f);
         }
      c = rval;
      }
   return( c);
}

int main( const int argc, const char **argv)
{
   int c;

   assert( argc == 2);
   ifile = fopen( argv[1], "rb");
   assert( ifile);
   while( (c = fgetc( ifile)) != EOF)
      {
      const char *forbidden = "~<>\"&";   /* don't show literally in HTML */

      c = grab_utf( c);
      if( c >= 0x80 || strchr( forbidden, c))
         printf( "&#x%x;", c);
      else
         printf( "%c", c);
      }
   fclose( ifile);
   return( 0);
}
