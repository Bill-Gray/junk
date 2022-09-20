#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/* Fairly straightforward code to read in JSON formatted WGSBN asteroid
name files,  such as

https://www.wgsbn-iau.org/files/json/V001/WGSBNBull_V001_007_html.json

   and extract non-ASCII names to go into

https://www.projectpluto.com/astnames.htm

   See also 'utf8hmtl.c',  code to read UTF8 files and output HTML
literals.  (Useful for generating a straight ASCII Web page that
decodes to the proper Unicode points.)   */

/* UTF8 decoder ripped,  with some modifications,  from PDCursesMod
(/pdcurses/util.c).        */

#define IS_CONTINUATION_BYTE( c) (((c) & 0xc0) == 0x80)

int PDC_mbtowc( uint32_t *pwc, const char *s, size_t n)
{
    uint32_t key;
    int i = -1;
    const unsigned char *string;
    char tbuff[4];

    assert( s);
    assert( pwc);
    if (!s || (n < 1))
        return -1;

    if (!*s)
        return 0;

    if( n < 4)       /* avoid read past end of buffer */
    {
        size_t j;

        for( j = 0; j < n; j++)
            tbuff[j] = *s++;
        tbuff[n] = '\0';
        s = tbuff;
    }

    string = (const unsigned char *)s;

    key = string[0];

    /* Simplistic UTF-8 decoder -- a little validation */

    if( !(key & 0x80))      /* 'ordinary' 7-bit ASCII */
        i = 1;
    else if ((key & 0xc0) == 0xc0 && IS_CONTINUATION_BYTE( string[1]))
    {
        if ((key & 0xe0) == 0xc0)
        {
            key = ((key & 0x1f) << 6) | (string[1] & 0x3f);
            i = 2;      /* two-byte sequence : U+0080 to U+07FF */
        }
        else if ((key & 0xf0) == 0xe0
                  && IS_CONTINUATION_BYTE( string[2]))
        {
            key = ((key & 0x0f) << 12) | ((string[1] & 0x3f) << 6) |
                  (string[2] & 0x3f);
            i = 3;      /* three-byte sequence : U+0800 to U+FFFF */
        }
        else if ((key & 0xf8) == 0xf0             /* SMP:  Unicode past 64K */
                  && IS_CONTINUATION_BYTE( string[2])
                  && IS_CONTINUATION_BYTE( string[3]))
        {
            key = ((key & 0x07) << 18) | ((string[1] & 0x3f) << 12) |
                  ((string[2] & 0x3f) << 6) | (string[2] & 0x3f);
            if( key <= 0x10ffff)
                i = 4;     /* four-byte sequence : U+10000 to U+10FFFF */
        }
    }

    if( i > 0)
       *pwc = key;

    return i;
}

static void utf8_to_html( char *obuff, const char *ibuff)
{
   while( *ibuff)
      {
      uint32_t key;
      int bytes_read = PDC_mbtowc( &key, ibuff, (int)strlen( ibuff));
      const char *forbidden = "~<>\"&";   /* don't show literally in HTML */

      assert( bytes_read > 0 && bytes_read < 5);
      if( key < 0x7e && !strchr( forbidden, (char)key))
         *obuff++ = *ibuff++;
      else
         {
         sprintf( obuff, "&#x%x;", key);
         obuff += strlen( obuff);
         ibuff += bytes_read;
         }
      }
   *obuff = '\0';
}

int main( const int argc, const char **argv)
{
   int i;

   setbuf( stdout, NULL);
   for( i = 1; i < argc; i++)
      {
      FILE *ifile = fopen( argv[i], "rb");
      char buff[300], *tptr;
      int ast_number = 0;
      const char *ver_text = strstr( argv[i], "WGSBNBull_V");

      assert( ifile);
      assert( ver_text);
      ver_text += 10;
      while( fgets( buff, sizeof( buff), ifile))
         if( (tptr = strstr( buff, "mp_number\": ")) != NULL)
            ast_number = atoi( tptr + 13);
         else if( (tptr = strstr( buff, "name\": ")) != NULL)
            {
            int j;
            bool is_ascii = true;

            tptr += 8;
            for( j = 0; tptr[j] && tptr[j] != '"'; j++)
               if( (unsigned char)tptr[j] & 0x80)
                  is_ascii = false;
            assert( tptr[j] == '"');
            tptr[j] = '\0';
            if( !is_ascii)
               {
               char obuff[300];

               utf8_to_html( obuff, tptr);
               printf( " %.8s %7d %s\n", ver_text, ast_number, obuff);
               }
            }
      fclose( ifile);
      }
}
