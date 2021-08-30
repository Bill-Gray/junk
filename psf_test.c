#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "psf.h"

/* Example code to test out the PSF/vgafont routines */

int main( const int argc, const char **argv)
{
   FILE *ifile = fopen( argv[1], "rb");
   uint8_t *buff;
   long len;
   struct font_info f;
   int i;

   assert( argc >= 2);
   assert( ifile);
   fseek( ifile, 0L, SEEK_END);
   len = ftell( ifile);
   fseek( ifile, 0L, SEEK_SET);
   buff = (uint8_t *)malloc( (size_t)len + 1);
   assert( buff);
   buff[len] = '\0';
   if( fread( buff, len, 1, ifile) != 1)
      {
      fprintf( stderr, "failed read\n");
      return( -1);
      }
   fclose( ifile);
   if( load_psf_or_vgafont( &f, buff, len))
      fprintf( stderr, "'%s' is neither PSF1 or PSF2 or vgafont\n", argv[1]);
   else
      {
      printf( "Font type %d\n", (int)f.font_type);
      printf( "Font contains %u glyphs,  each %ux%u\n",
                  f.n_glyphs, f.width, f.height);
      printf( "%u Unicode references found\n", f.unicode_info_size);
      for( i = 0; i < (int)f.unicode_info_size; i++)
         printf( "%x: %x\n", f.unicode_info[i + i], f.unicode_info[i + i + 1]);
      for( i = 2; i < argc; i++)
         {
         unsigned unicode_point;
         int glyph_num;

         if( sscanf( argv[i], "%x", &unicode_point) != 1)
            unicode_point = argv[i][0];
         glyph_num = find_psf_or_vgafont_glyph( &f, (uint32_t)unicode_point);
         printf( "%x -> glyph num dec %d = hex %x\n", unicode_point,
                           glyph_num, glyph_num);
         if( glyph_num >= 0)
            {
            int x, y;

            for( y = 0; y < (int)f.height; y++)
               {
               const uint8_t *tptr = f.glyphs + glyph_num * f.charsize
                        + y * ((f.width + 7) / 8);

               for( x = 0; x < (int)f.width; x++)
                  if( (tptr[x >> 3] << (x & 7)) & 0x80)
                     printf( "**");
                  else
                     printf( "  ");
               printf( "|\n");
               }
            }
         }
      }
   free( buff);
   return( 0);
}
