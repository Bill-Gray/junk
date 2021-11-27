#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Anti-robocall program.  Probably only useful to me,  though the bits
about how to communicate with a modem may have some utility to others.

   This will initialize _my_ modem (which is a PCI one on /dev/ttyS4...
you'll note that this is very "me"-specific),  tells it to turn on caller
ID strings,  and await phone calls.  When a call comes in,  the phone
number is run through my list of numbers.  The area code and exchange
are checked against a list (of US and Canadian numbers) downloaded from

https://www.nationalnanpa.com/reports/reports_cocodes_assign.html

   The number is also checked against NoMoRobo's list.  If the
number isn't found there,  it's probably not junk.  If it is,  the
modem picks up the line and answers it with the usual modem beeps
and whistles.  We just hear the phone ring once.

   This runs within a terminal,  so I can see who's calling (and who's
called recently) without looking up from my computer.    */

#define BAUD B57600

int open_port()
{
    int fd;

    fd = open("/dev/ttyS4", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1)
    {
        perror("Unable to open /dev/ttyS4: ");
        exit(1);
    }
    printf("opened port\n");

    int ret;
    struct termios ts;

    bzero(&ts, sizeof(ts));
    cfmakeraw(&ts);
    cfsetspeed(&ts, BAUD);
    ts.c_cflag |= (CLOCAL | CREAD | CSTOPB | CRTSCTS);
    tcflush(fd, TCIOFLUSH);
//  ts.c_cc[VMIN] = 0;
//  ts.c_cc[VTIME] = 2;
//  ts.c_lflag &= ~ICANON;

    ret = tcsetattr(fd, TCSANOW, &ts);
    if (ret == -1)
    {
        perror("tcsetattr: ");
        exit(1);
    }
    printf("set attrs\n");

    return fd;
}

static int send_command( const int fd, const char *cmd, const int delay_us)
{
   int bytes_read, i, bytes_written;
   uint8_t c = 13;

   for( i = 0; cmd[i]; i++)
      {
      bytes_written = write( fd, cmd + i, 1);
      assert( bytes_written == 1);
      usleep( 60);
      }
   bytes_written = write( fd, &c, 1);
   assert( bytes_written == 1);
   printf( "Sent '%s'\n", cmd);
   usleep( delay_us);
   c = ' ';
   while( c != 'O')
      {
      bytes_read = read( fd, &c, 1);
      if( bytes_read == 1)
         printf( "%c", c);
      }
   while( c != 13)
      {
      bytes_read = read( fd, &c, 1);
      if( bytes_read == 1)
         printf( "%c", c);
      }
   return( (int)c);
}

static int check_nomorobo( const char *tel_no)
{
   char buff[150], full_no[20];
   int rval;

   if( strlen( tel_no) == 7)
      snprintf( full_no, sizeof( full_no), "207-%.3s-%s",
                  tel_no, tel_no + 3);
   else if( strlen( tel_no) == 10)
      snprintf( full_no, sizeof( full_no), "%.3s-%.3s-%s",
                  tel_no, tel_no + 3, tel_no + 6);
   else
      {
      fprintf( stderr, "? Bad tel number '%s'\n", tel_no);
      return( -1);
      }
   snprintf( buff, sizeof( buff),
            "wget https://www.nomorobo.com/lookup/%s -q -O /tmp/zq_tel", full_no);
// strcat( buff, "> /dev/null 2>&1");
   rval = system( buff);
// printf( "%d: command '%s'\n", rval, buff);
// unlink( "/tmp/zq_tel");
   return( rval);
}

/* Ripped,  unaltered,  from 'vt100.c'.  See explanation there. */

static int kbhit(void)
{
    int c = -1;
    static struct termios orig_term;
    const int STDIN = 0;
    static struct termios term;
    struct timeval timeout;
    fd_set rdset;

    tcgetattr( STDIN, &orig_term);
    memcpy(&term, &orig_term, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr( STDIN, TCSANOW, &term);
    FD_ZERO( &rdset);
    FD_SET( STDIN, &rdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;
    if( select( STDIN + 1, &rdset, NULL, NULL, &timeout) > 0)
       {
       c = getchar();
       if( c < 0)
           c += 256;
       }
    tcsetattr( STDIN, TCSANOW, &orig_term);
    return( c);
}

/* When reading audio data,  any "real" 0x10 byte (a DLE) is doubled.
A "this is the end of the data" one is not.  The following code takes
a buffer and looks for pairs of 0x10 bytes and eliminates one of them.
The return value will be equal to or less than n_read.  */

static int remove_duplicate_dles( char *buff, int n_read)
{
   char *tptr;
   int rval = n_read;

   while( (tptr = strstr( buff, "\020\020")) != NULL)
      {
      n_read -= (tptr - buff) + 1;
      buff = tptr + 1;
      memmove( buff, buff + 1, n_read - 1);
      rval--;
      }
   return( rval);
}

#define SET_TITLE "\033\1352;"
#define RESIZE_TERM "\033[8;%d;%dt"

int main( const int argc, const char **argv)
{
    int i, j, fd = open_port();
    char buff[200], name[80];

    printf( SET_TITLE "Telephone Interceptor\a");
    printf( RESIZE_TERM, 8, 73);
    setbuf( stdout, NULL);
    setbuf( stdin, NULL);
    for( i = 1; i < argc; i++)
        send_command( fd, argv[i], 100000);

    i = 0;
    *name = '\0';
    for( ;;)
    {
       if( read(fd, buff + i, 1) < 0)
           {
           char c;

           while( (c = kbhit( )) > 0)
              {
              if( c == 10)
                 c = 13;
              if( c == 27)
                 return( -1);
              else if( c == 1)
                 {
                 send_command( fd, "ATH1", 1000000);
                 send_command( fd, "ATA", 100000);
                 }
              else if( c == 2)     /* Ctrl-B = record */
                 {
                 FILE *ofile = fopen( "z.pcm", "wb");
                 int n_shown = 0, total_read = 0;
                 ssize_t n_read;
                 bool prev_byte_was_dle = false;

                 send_command( fd, "AT+FCLASS=8", 100000);
                        /* above sets to use as voice modem */
                 send_command( fd, "AT+VLS=1", 100000);
                        /* above sets for picking up phone & listening/playback */
                 send_command( fd, "AT+VSM=0", 100000);
                        /* above sets for PCM */
                 send_command( fd, "AT+VRX", 100000);
                 printf( "Receiving data.  Hit any key to stop.\n");
                 while( kbhit( ) <= 0)
                    {
                    n_read = read( fd, buff, sizeof( buff) - 1);
                    if( n_read > 0)
                       {
                       if( prev_byte_was_dle)
                          {
                          memmove( buff + 1, buff, n_read);
                          *buff = 0x10;
                          n_read++;
                          }
                       if( buff[n_read - 1] == 0x10)
                          {
                          n_read--;
                          prev_byte_was_dle = true;
                          }
                       else
                          prev_byte_was_dle = false;
                       n_read = remove_duplicate_dles( buff, n_read);
                       fwrite( buff, n_read, 1, ofile);
                       total_read += (int)n_read;
                       }
//                  if( total_read - n_shown > 100)
                       {
                       printf( "[%d]", total_read);
                       n_shown = total_read;
                       }
                    }
                 buff[0] = 3;
                 if( write( fd, buff, 1) != 1)
                    fprintf( stderr, "!! Byte not written !!\n");
                 usleep( 100000);
                 while( (n_read = read( fd, buff, sizeof( buff))) > 0)
                    {
                    fwrite( buff, n_read, 1, ofile);
                    total_read += (int)n_read;
                    }

                 printf( "Done\n");
                 fclose( ofile);
                 }
              else
                 {
                 const int bytes_written = write( fd, &c, 1);

                 assert( bytes_written == 1);
                 }
              }
           usleep( 100000);
           }
       else
           {
//         printf( "From modem: '%d', char %d\n", buff[i], i);
           if( buff[i] < ' ')
               {
               buff[i] = '\0';
               if( i)
                  {
                  bool is_scam = false;
                  time_t t0 = time( NULL);

                  if( !memcmp( buff, "RING", 4))
                     printf( "Ring...");
                  else if( !memcmp( buff, "NAME = ", 7))
                     {
                     FILE *ifile = fopen( "tel_nos.txt", "rb");
                     char ibuff[200];

                     printf( "%.8s: '%s'\n", ctime( &t0) + 11, buff);
                     strcpy( name, buff + 7);
                     printf( "%s%s\a", SET_TITLE, buff + 7);
                     while( fgets( ibuff, sizeof( ibuff), ifile))
                        if( !strncmp( buff, ibuff, i) && ibuff[i] < ' ')
                           is_scam = true;
                     fclose( ifile);
                     }
                  else if( !memcmp( buff, "NMBR = ", 7))
                     {
                     FILE *ifile = fopen( "allutlzd.txt", "rb");
                     char number[20];
                     bool number_is_cleared = false;
                     const time_t t0 = time( NULL);

                     assert( ifile);
                     if( i == 17)
                        strcpy( number, buff + 7);
                     else if( i == 18 && buff[7] == '1')
                        strcpy( number, buff + 8);
                     else if( i == 14)
                        {
                        strcpy( number, "207");
                        strcat( number, buff + 7);
                        }
                     else
                        *number = '\0';
                     assert( ifile);
                     printf( "Searching for '%s'\n", number);
                     while( fgets( buff, sizeof( buff), ifile))
                        if( !memcmp( number, buff + 6, 3) && !memcmp( number + 3, buff + 10, 3))
                           printf( "%.13s %.2s\n", buff + 86, buff);
                     fclose( ifile);

                     ifile = fopen( "tel_nos.txt", "r+b");
                     assert( ifile);
                     while( fgets( buff, sizeof( buff), ifile))
                        {
                        for( i = j = 0; strchr( "0123456789 ()", buff[i]); i++)
                           if( buff[i] >= '0' && buff[i] <= '9')
                              buff[j++] = buff[i];
                        if( j == 4)
                           {
                           memmove( buff + 3, buff, 5);
                           memcpy( buff, "666", 3);
                           j = 7;
                           }
                        if( j == 7)
                           {
                           memmove( buff + 3, buff, 8);
                           memcpy( buff, "207", 3);
                           j = 10;
                           }
                        if( j && j != 10)
                           printf( "Bad entry : %s", buff);
                        if( j == 10 && !memcmp( buff, number, 10))
                           {
                           printf( "%s", buff + i);
                           if( !memcmp( buff + i, "HANG UP", 7))
                              is_scam = true;
                           else
                              number_is_cleared = true;
                           if( memcmp( buff + i, "Passed", 6))
                              printf( "%s%s\a", SET_TITLE, buff + i);
                           }
                        }
                     if( !number_is_cleared && !is_scam)  /* Number is new to us */
                        {
                        if( !check_nomorobo( number))
                           {
                           is_scam = true;
                           printf( "NoMoRoBo says it's junk\n");
                           }
                        else if( !memcmp( number, "207", 3) &&
                                               !memcmp( name + 12, " ME", 3))
                           {
                           is_scam = true;
                           printf( "Pseudo-local scam\n");
                           }
                        }
                     fprintf( ifile, "(%.3s) %.3s %.4s %s %.24s %s\n",
                              number, number + 3, number + 6,
                              is_scam ? "HANG UP" : "Passed", ctime( &t0), name);
                     fclose( ifile);
                     }
                  else    /* not 'number', 'name',  or 'ring' */
                     printf( "Got string '%s'\n", buff);
                  if( is_scam)
                     {
                     send_command( fd, "ATH1", 1000000);
/*                   send_command( fd, "ATA", 1000000);        */
                     send_command( fd, "ATH0", 100000);
                     }
                  }
               i = 0;
               }
           else
               i++;
           }
    }
    return( 1);
}
