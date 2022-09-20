#include <stdio.h>
#include <assert.h>
#include <string.h>

/* Code to read a file (specified on the command line) containing -,
+,  and | and convert it to box characters.  An example 'before :'

   +-----------------+-----+
   |  Charlie+dog    |     +--+
   |             +---+   +-+  |
 +-+    Easy |   | Baker |Able|
 +---------------+-------+----+

and 'after' :

   ┌─────────────────┬─────┐
   │  Charlie+dog    │     ├──┐
   │             ┌───┘   ┌─┘  │
 ┌─┘    Easy |   │ Baker │Able│
 └───────────────┴───────┴────┘

   This requires looking at each line and the lines before and
after it.  'show_line' takes a set of three lines and outputs the
middle one.         */

static void show_line( const char *line0, const char *line1, const char *line2)
{
   const size_t len0 = strlen( line0), len1 = strlen( line1), len2 = strlen( line2);
   size_t i;

   for( i = 0; i < len1; i++)
      if( line1[i] == '+' || line1[i] == '-' || line1[i] == '|')
         {
         int mask = 0;

         if( i && (line1[i - 1] == '-' || line1[i - 1] == '+'))
            mask = 1;         /* line to the left */
         if( line1[i + 1] == '-' || line1[i + 1] == '+')
            mask |= 2;        /* line to the right */
         if( i < len0 && (line0[i] == '|' || line0[i] == '+'))
            mask |= 4;        /* line sticking up */
         if( i < len2 && (line2[i] == '|' || line2[i] == '+'))
            mask |= 8;        /* line sticking down */
         if( line1[i] == '+')
            {
            const unsigned char extra_char[16] = {
                  0, 0, 0, 0, 0, 0x98, 0x94, 0xb4,
                  0, 0x90, 0x8c, 0xac, 0, 0xa4, 0x9c, 0xbc };

            if( !extra_char[mask])     /* just an isolated '+' */
               printf( "+");
            else
               printf( "\xe2\x94%c", extra_char[mask]);
            }
         else if( line1[i] == '-')
            {
            if( !(mask & 3))     /* isolated '-' */
               printf( "-");
            else
               printf( "\xe2\x94\x80");         /* horiz line */
            }
         else
            {
            assert( line1[i] == '|');
            if( !(mask & 0xc))     /* isolated '|' */
               printf( "|");
            else
               printf( "\xe2\x94\x82");         /* vert  line */
            }
         }
      else
         putc( (char)line1[i], stdout);
}

#define LINE_SIZE 300

int main( const int argc, const char **argv)
{
   char line0[LINE_SIZE], line1[LINE_SIZE], line2[LINE_SIZE];
   FILE *ifile = fopen( argv[1], "rb");

   assert( argc == 2);
   assert( ifile);
   if( !fgets( line0, LINE_SIZE, ifile) || !fgets( line1, LINE_SIZE, ifile))
      {
      fprintf( stderr, "Couldn't read lines\n");
      return( 0);
      }
   show_line( "", line0, line1);
   while( fgets( line2, LINE_SIZE, ifile))
      {
      show_line( line0, line1, line2);
      strcpy( line0, line1);
      strcpy( line1, line2);
      }
   fclose( ifile);
   show_line( line0, line1, "");
   return( 0);
}
