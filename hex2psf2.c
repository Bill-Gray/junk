#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* Code to read Unifont fonts in the ASCII .hex format,  and output
them in PSF type 2 format.  Unifont is available in .hex (and other)
formats at

https://unifoundry.com/unifont/index.html

   The .hex format provides each glyph on a line giving the Unicode
point,  then the pixels for that glyph encoded in hexadecimal.  Example:

0034:00000000040C142444447E0404040000
0035:000000007E4040407C020202423C0000
0036:000000001C2040407C424242423C0000
0037:000000007E0202040404080808080000
0038:000000003C4242423C424242423C0000
0039:000000003C4242423E02020204380000
003A:00000000000018180000001818000000

   The PSF type 2 font format is described at

https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html

   The font will default to being output as a plain binary .psf file.
However,  it can be useful to have the font be output as a C array,
for direct inclusion in a program.  If you run

./hex2psf2 input_file.hex output_file.psf2

   you'll get a binary file.  If you add one more command line argument,
such as

./hex2psf2 input_file.hex output_file.c any_random_arg

   the output will be a decently well-structured C array.  Note that if
you run the full BMP Unifont .hex file,  or concatenate it with the SMP
file,  you can make a relatively humongous .psf or .c file.  I usually
edit the input to contain some subset of Unifont.

   At present,  this is limited to 8x16 fonts.  In the following structure,
'bits' contains 32 bytes,  to allow for possible 16x16 versions.  (PSF2
assumes all glyphs are the same size.  For fullwidth characters and
emoji,  we'll either need to make two fonts -- one 8x16,  one 16x16 --
or modify the PSF2 format.) */

typedef struct
{
   uint32_t code_point;
   uint8_t bits[32];
   int height, width;
} glyph_t;

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

static int get_font_bits( const char *buff, glyph_t *glyph)
{
   int i, rval = -1;
   const char *tptr = buff + 4;

   if( *tptr != ':')
      tptr++;
   if( *tptr != ':')
      tptr++;
   if( *tptr == ':' && tptr[33] < ' ')
      {
      unsigned code_point;

      sscanf( buff, "%x", &code_point);
      glyph->code_point = code_point;
      glyph->height = 16;
      glyph->width = 8;
      tptr++;
      for( i = 0; i < 16; i++)
         {
         int digit1 = *tptr++ - '0';
         int digit2 = *tptr++ - '0';

         if( digit1 >= 10)
            digit1 += 10 + '0' - 'A';
         if( digit2 >= 10)
            digit2 += 10 + '0' - 'A';
         glyph->bits[i] = (char)( digit1 * 16 + digit2);
         }
      rval = 0;
      }
   return( rval);
}

static void _output_header( const struct psf2_header *hdr, FILE *ofile)
{
   fprintf( ofile, "    0x72, 0xB5, 0x4A, 0x86,   /* 'magic' PSF2 bytes */\n");
   fprintf( ofile, "    0x00, 0x00, 0x00, 0x00,   /* version */\n");
   fprintf( ofile, "    0x20, 0x00, 0x00, 0x00,   /* this header is 0x20 bytes long */\n");
   fprintf( ofile, "    0x01, 0x00, 0x00, 0x00,   /* flagged as 'has Unicode tbl' */\n");
   fprintf( ofile,
        "    0x%02x, 0x%02x, 0x%02x, 0x00,   /* %d glyphs */\n",
            (unsigned)( hdr->length & 0xff),
            (unsigned)( hdr->length >> 8) & 0xff,
            (unsigned)( hdr->length >> 16), hdr->length);
   fprintf( ofile, "    0x10, 0x00, 0x00, 0x00,   /* bytes/glyph */\n");
   fprintf( ofile, "    0x10, 0x00, 0x00, 0x00,   /* glyph height (=16 pixels) */\n");
   fprintf( ofile, "    0x08, 0x00, 0x00, 0x00,   /* glyph width (=8 pixels) */\n");
}

static void _output_glyph( const glyph_t *glyph, FILE *ofile)
{
   int i;

   fprintf( ofile, " /* U+%x */ ", glyph->code_point);
   for( i = 0; i < glyph->height; i++)
      fprintf( ofile, " 0x%02x,", (unsigned char)glyph->bits[i]);
   fprintf( ofile, "\n");
}

static void _output_utf8_info( const int code_point, const char *bytes,
                                     const int size, FILE *ofile)
{
   int i;

   fprintf( ofile, " /* U+%x */ ", code_point);
   for( i = 0; i < size; i++)
      fprintf( ofile, "0x%02x, ", (unsigned char)bytes[i]);
   fprintf( ofile, "\n");
}

#define IS_POWER_OF_TWO( n)    (((n) & ((n)-1)) == 0)

int main( const int argc, const char **argv)
{
   FILE *ifile = fopen( argv[1], "rb"), *ofile;
   struct psf2_header hdr;
   int i, n_glyphs = 0, write_c_array = (argc == 4);
   int array_size = sizeof( struct psf2_header);
   char buff[100];
   glyph_t *glyph = (glyph_t *)malloc( sizeof( glyph_t));

   assert( argc == 3 || argc == 4);
   assert( ifile);
   if( fread( &hdr, 1, sizeof( hdr), ifile) != sizeof( hdr))
      {
      fprintf( stderr, "Couldn't read header from %s\n", argv[1]);
      return( -1);
      }
   hdr.magic[0] = PSF2_MAGIC0;
   hdr.magic[1] = PSF2_MAGIC1;
   hdr.magic[2] = PSF2_MAGIC2;
   hdr.magic[3] = PSF2_MAGIC3;
   hdr.flags = PSF2_HAS_UNICODE_TABLE;
   hdr.width = 8;
   hdr.height = hdr.charsize = 16;
   hdr.version = 0;
   while( fgets( buff, sizeof( buff), ifile))
      if( !get_font_bits( buff, glyph + n_glyphs))
         {
         char utf8[5];

         array_size += hdr.charsize + 1 +
                  PDC_wc_to_utf8( utf8, glyph[n_glyphs].code_point);
         n_glyphs++;
         if( IS_POWER_OF_TWO( n_glyphs))
            glyph = (glyph_t *)realloc( glyph, 2 * n_glyphs * sizeof( glyph_t));
         }
   fclose( ifile);

   ofile = fopen( argv[2], "wb");
   assert( ofile);
   hdr.headersize = sizeof( hdr);
   hdr.length = n_glyphs;
   if( write_c_array)
      {
      fprintf( ofile, "const unsigned char psf2_font[%d] = {\n", array_size);
      _output_header( &hdr, ofile);
      }
   else
      fwrite( &hdr, 1, sizeof( hdr), ofile);
   for( i = 0; i < n_glyphs; i++)
      if( write_c_array)
         _output_glyph( glyph + i, ofile);
      else
         fwrite( glyph[i].bits, hdr.charsize, 1, ofile);
   for( i = 0; i < n_glyphs; i++)
      {
      char utf8[5];
      const int osize = PDC_wc_to_utf8( utf8, glyph[i].code_point);

      utf8[osize] = (char)PSF2_SEPARATOR;
      if( write_c_array)
         _output_utf8_info( glyph[i].code_point, utf8, osize + 1, ofile);
      else
         fwrite( utf8, osize + 1, 1, ofile);
      }
   free( glyph);
   if( write_c_array)
      fprintf( ofile, "};\n");
   fclose( ofile);
   return( 0);
}
