#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

/* Code to "launder" Yahoo<i>!</i> Groups posts to plain HTML. This
removes a lot of Javascript cruft and gives you a just-the-facts
HTML file.  When Yahoo! yanks its support, you'll still be able to
view the laundered files.

First,  you have to download your Yahoo! Group posts.  The
following is a script written by Karl Battams for the purpose.
Make a directory,  cd into it,  and modify the group name (below,
'guide-user'),  starting message (below,  'ctr=1'),  and last
message (below,  11306).  For me,  that got files 1.html, 2.html,
..., 11306.html.

         ------------ Snip ------------
#!/bin/sh

root='https://groups.yahoo.com/neo/groups/guide-user/conversations/messages/'
ctr=1         # first message i.e. message 1

while [ $ctr -le 11306 ]            # last message number + 1
do
wget $root$ctr
mv $ctr $ctr".html"
ctr=$((ctr + 1))
done
         ------------ Snap ------------

   Running this code as './launder 1 11306' got me files msg1.html, msg2.htm, ...
msg11306.htm,  plus an 'index.htm' file.

   To be done : links to 'next post', 'prev post', 'index',  and perhaps
'next in thread' and 'prev in thread' would be nice.  And maybe 'other posts
by this author.'

   I've tested this code with my groups,  and it appears to work,  but no
warranty is provided. */

static const char *header =
    "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>\n"
    "<HTML>\n"
    "<HEAD>\n"
    "   <TITLE> %s </TITLE>\n"
    "   <META http-equiv=Content-Type content='text/html; charset=utf-8'>\n"
    "   <META NAME='Author' CONTENT='This should be the author'>\n"
    "</HEAD>\n"
    "<BODY>\n" ;

FILE *index_file;

/* Yahoo! messages use <br/> when they should use <br>.  The HTML validator will
warn about this.  It's a minor nuisance,  so we remove the slash.

Yahoo! messages also include various other junk that I'll eventually remove. */

static void remove_junk( char *buff)
{
   char *tptr;

   while( (tptr = strstr( buff, "<br/>")) != NULL)
      memmove( tptr + 3, tptr + 4, strlen( tptr + 3));
}


static void process_yahoo_html( const int post_idx)
{
   char buff[2048];
   FILE *ifile;

   sprintf( buff, "%d.html", post_idx);
   ifile = fopen( buff, "rb");
   if( ifile)
      {
      int n_titles = 0;
      char title[300], time[40], author[80];
      bool got_all_headers = false;

      *title = *time = *author = '\0';
      while( !got_all_headers && fgets( buff, sizeof( buff), ifile))
         {
         char *tptr, *end_ptr;

         tptr = strstr( buff, "data-subject=\"");
         if( tptr)
            {
            tptr += 14;
            end_ptr = strchr( tptr, '"');
            assert( end_ptr);
            if( end_ptr - tptr > (ssize_t)sizeof( title))
               printf( "LONG TITLE : size %d, %.200s\n",
                        (int)( end_ptr - tptr), tptr);
            assert( end_ptr - tptr < (ssize_t)sizeof( title));
            *end_ptr = '\0';
            strcpy( title, tptr);
            n_titles++;
            }
         tptr = strstr( buff, "Message sent time");
         if( tptr)
            {
            tptr += 19;
            end_ptr = strchr( tptr, '<');
            assert( end_ptr);
            assert( end_ptr - tptr < (ssize_t)sizeof( time));
            *end_ptr = '\0';
            strcpy( time, tptr);
            }
         tptr = strstr( buff, "author fleft fw-600");
         if( tptr)
            {
            tptr += 21;
            end_ptr = strchr( tptr, '<');
            assert( end_ptr);
            assert( end_ptr - tptr < (ssize_t)sizeof( author));
            *end_ptr = '\0';
            strcpy( author, tptr);
            }
         got_all_headers = (*author && *time && *title);
         }
      assert( n_titles < 2);
      if( got_all_headers)
         {
         FILE *ofile;

         printf( "%4d (%s) %s: %s\n", post_idx, time, author, title);
         sprintf( buff, "msg%d.htm", post_idx);
         ofile = fopen( buff, "wb");
         assert( ofile);
         fprintf( index_file, "<a href='%s'>%d: %s</a>", buff, post_idx, title);
         fprintf( index_file, " <i>%s</i> %s<br>\n", time, author);
         fprintf( ofile, header, title);
         fprintf( ofile, "<p> <a href='index.htm'> Index</a>");
         if( post_idx > 1)
            fprintf( ofile, " &nbsp;&nbsp;&nbsp; <a href='msg%d.htm'>Previous message</a>",
                              post_idx - 1);
         fprintf( ofile, " &nbsp;&nbsp;&nbsp; <a href='msg%d.htm'> Next message</a>",
                              post_idx + 1);
         fprintf( ofile, "\n<h2> %s </h2>\n", title);
         fprintf( ofile, "<p> <i> %s </i> %s </p>\n", author, time);
         while( fgets( buff, sizeof( buff), ifile))
            if( !memcmp( buff, "class=\"msg-content undoreset", 28))
               {
               char tbuff[2000];
               size_t i;

               strcpy( buff, "<div ");
               while( fgets( tbuff, sizeof( tbuff), ifile) &&
                              memcmp( tbuff, "class=\"msg-inline-video", 23))
                  {
                  remove_junk( tbuff);
                  fprintf( ofile, "%s", buff);
                  strcpy( buff, tbuff);
                  }
               i = strlen( buff);
               while( i && memcmp( buff + i, "<div", 4))
                  i--;
               buff[i] = '\0';
               fprintf( ofile, "%s\n", buff);
               }
         fprintf( ofile, "</body> </html>\n");
         fclose( ofile);
         }
      fclose( ifile);
      }
}

int main( const int argc, const char **argv)
{
   const int lower = (argc > 1 ? atoi( argv[1]) : 1);
   const int upper = (argc > 2 ? atoi( argv[2]) : 10000);
   int i;

   index_file = fopen( "index.htm", "wb");
   fprintf( index_file, header, "List index");
   fprintf( index_file, "\n<h2> List index </h2>\n<p>");

   for( i = lower; i <= upper; i++)
      process_yahoo_html( i);
   fprintf( index_file, "</body> </html>\n");
   fclose( index_file);
   return( 0);
}
