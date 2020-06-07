#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* Code to read Unifont ASCII fonts,  with data such as

0034:00000000040C142444447E0404040000
0035:000000007E4040407C020202423C0000
0036:000000001C2040407C424242423C0000
0037:000000007E0202040404080808080000
0038:000000003C4242423C424242423C0000
0039:000000003C4242423E02020204380000
003A:00000000000018180000001818000000

   and output a PSF type 2 font,  as described at

https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html      */

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

#define MAX_UNICODE_INFO 1048576

   /* Ripped,  intact,  from 'pdcurses/util.c' in PDCurses. */

int PDC_wc_to_utf8( char *dest, const int32_t code)
{
   int n_bytes_out;

   if (code < 0x80)
   {
       dest[0] = (char)code;
       n_bytes_out = 1;
   }
   else
       if (code < 0x800)
       {
           dest[0] = (char) (((code >> 6) & 0x1f) | 0xc0);
           dest[1] = (char) ((code & 0x3f) | 0x80);
           n_bytes_out = 2;
       }
       else if( code < 0x10000)
       {
           dest[0] = (char) (((code >> 12) & 0x0f) | 0xe0);
           dest[1] = (char) (((code >> 6) & 0x3f) | 0x80);
           dest[2] = (char) ((code & 0x3f) | 0x80);
           n_bytes_out = 3;
       }
       else       /* Unicode past 64K,  i.e.,  SMP */
       {
           dest[0] = (char) (((code >> 18) & 0x0f) | 0xf0);
           dest[1] = (char) (((code >> 12) & 0x3f) | 0x80);
           dest[2] = (char) (((code >> 6) & 0x3f) | 0x80);
           dest[3] = (char) ((code & 0x3f) | 0x80);
           n_bytes_out = 4;
       }
   return( n_bytes_out);
}

/* At present,  just looks for 8x16 fonts */

static int32_t get_font_bits( char *buff, struct psf2_header *hdr)
{
   int i, rval = -1;
   char *tptr = buff + 4;

   if( *tptr != ':')
      tptr++;
   if( *tptr != ':')
      tptr++;
   if( *tptr == ':' && tptr[33] < ' ')
      {
      sscanf( buff, "%x", &rval);
      tptr++;
      for( i = 0; i < 16; i++)
         {
         int digit1 = *tptr++ - '0';
         int digit2 = *tptr++ - '0';

         if( digit1 >= 10)
            digit1 += 10 + '0' - 'A';
         if( digit2 >= 10)
            digit2 += 10 + '0' - 'A';
         buff[i] = (char)( digit1 * 16 + digit2);
         }
      }
   return( (int32_t)rval);
}

int main( const int argc, const char **argv)
{
   FILE *ifile = fopen( argv[1], "rb"), *ofile;
   struct psf2_header hdr;
   int32_t unicode_pt;
   int n_glyphs = 0;
   char buff[100];
   char *unicode_info = (char *)malloc( MAX_UNICODE_INFO);
   char *uptr = unicode_info;

   assert( argc == 3);
   assert( ifile);
   ofile = fopen( argv[2], "wb");
   assert( ofile);
   fread( &hdr, 1, sizeof( hdr), ifile);
   hdr.magic[0] = PSF2_MAGIC0;
   hdr.magic[1] = PSF2_MAGIC1;
   hdr.magic[2] = PSF2_MAGIC2;
   hdr.magic[3] = PSF2_MAGIC3;
   hdr.flags = PSF2_HAS_UNICODE_TABLE;
   hdr.width = 8;
   hdr.height = hdr.charsize = 16;
   hdr.version = 0;
   hdr.headersize = sizeof( hdr);
   fwrite( &hdr, 1, sizeof( hdr), ofile);
   while( fgets( buff, sizeof( buff), ifile))
      if( (unicode_pt = get_font_bits( buff, &hdr)) >= 0)
         {
         const int bytes_written = PDC_wc_to_utf8( uptr, unicode_pt);

         fwrite( buff, hdr.charsize, 1, ofile);
         n_glyphs++;
         uptr += bytes_written;
         *uptr++ = (unsigned char)0xff;
         }
   fclose( ifile);
   fwrite( unicode_info, uptr - unicode_info, 1, ofile);
   free( unicode_info);
   hdr.length = n_glyphs;
   fseek( ofile, 0L, SEEK_SET);
   fwrite( &hdr, 1, sizeof( hdr), ofile);
   fclose( ofile);
   return( 0);
}
