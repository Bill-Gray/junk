#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* Code to read the 'new' (psf2) version of the PSF font format,
described at

https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html

   At present,  this reads the header,  skips the actual font bits,
and dumps out the Unicode data at the end.  The Unicode data is
a little strange to parse.  Each glyph can correspond to one or
more Unicode values.   */

#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04

#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE 0x01

/* max version recognized so far */
#define PSF2_MAXVERSION 0

/* UTF8 separators */
#define PSF2_SEPARATOR  0xFF
#define PSF2_STARTSEQ   0xFE

struct psf2_header {
        uint8_t magic[4];
        uint32_t version;
        uint32_t headersize;    /* offset of bitmaps in file */
        uint32_t flags;
        uint32_t length;        /* number of glyphs */
        uint32_t charsize;      /* number of bytes for each character */
        uint32_t height, width; /* max dimensions of glyphs */
        /* charsize = height * ((width + 7) / 8) */
};

int main( const int argc, const char **argv)
{
   FILE *ifile = fopen( argv[1], "rb");
   struct psf2_header hdr;

   assert( argc == 2);
   assert( ifile);
   fread( &hdr, 1, sizeof( hdr), ifile);
   if( hdr.magic[0] == PSF1_MAGIC0 && hdr.magic[1] == PSF1_MAGIC1)
      {
      printf( "'%s' appears to be a psf1 (old type) font file\n", argv[1]);
      return( -1);
      }
   if( hdr.magic[0] != PSF2_MAGIC0 || hdr.magic[1] != PSF2_MAGIC1
            || hdr.magic[2] != PSF2_MAGIC2 || hdr.magic[3] != PSF2_MAGIC3)
      {
      printf( "'%s' is not a psf font file\n", argv[1]);
      return( -1);
      }
   printf( "Bitmaps start at %u\n", (unsigned)hdr.headersize);
   printf( "%u glyphs,  each %u x %u pixels\n", (unsigned)hdr.length,
                  (unsigned)hdr.height, (unsigned)hdr.width);
   if( hdr.flags & PSF2_HAS_UNICODE_TABLE)
      {
      uint8_t *buff;
      long filesize, offset;
      size_t i, len;

      fseek( ifile, 0L, SEEK_END);
      filesize = ftell( ifile);
      offset = (long)( hdr.headersize + hdr.length * hdr.charsize);
      fseek( ifile, offset, SEEK_SET);
      assert( offset < filesize);
      len = (size_t)( filesize - offset);
      buff = (uint8_t *)malloc( len);
      fread( buff, len, 1, ifile);
      for( i = 0; i < len; i++)
         if( buff[i] == 0xff)
            printf( "\n");
         else if( buff[i] == 0xfe)
            printf( "(seq) ");
         else if( !(buff[i] & 0x80))
            printf( "%02x ", (unsigned)buff[i]);
         else
            {
            unsigned cval;

            if( (buff[i] & 0xe0) == 0xc0)
               {
               cval = ((buff[i] & 0x1f) << 6) | (buff[i + 1] & 0x3f);
               i++;
               }
            else
               {
               cval = ((buff[i] & 0x0f) << 12) | ((buff[i + 1] & 0x3f) << 6) | (buff[i + 2] & 0x3f);
               if( (buff[i] & 0xf0) == 0xf0)
                  {
                  cval = (cval << 6) | (buff[i + 3] & 0x3f);
                  i++;
                  }
               i += 2;
               }
            printf( "%x ", cval);
            }
      printf( "\n");
      free( buff);
      }
   fclose( ifile);
   return( 0);
}
