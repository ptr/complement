#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include <stdio.h>

int main( int argc, char **argv )
{
  struct addrinfo *hosts_list = 0;
  /* sockaddr sa; */
  int i;

  int _errno = getaddrinfo( "1.0.0.127.sbl-xbl.spamhaus.org", 0, 0, &hosts_list );

  if ( _errno == 0 ) {
    struct addrinfo *host = hosts_list;
    if ( host != 0 && host->ai_addr != 0 ) {
      printf( "---- 1\n" );
      while ( host != 0 ) {
        /* *bi++ = *host->ai_addr; */
        for ( i = 0; i < 14; ++i ) {
          printf( "'%x'", (*host->ai_addr).sa_data[i] );
        }
        printf( "\n" );
        host = host->ai_next;
      }
      printf( "---- 2\n" );
    }
  } else {
    printf( "Errno: %d\n", _errno );
  }

  if ( hosts_list != 0 ) {
    freeaddrinfo( hosts_list );
  }

  printf( "+++++++++++\n", _errno );

  _errno = getaddrinfo( "localhost", 0, 0, &hosts_list );

  if ( _errno == 0 ) {
    struct addrinfo *host = hosts_list;
    if ( host != 0 && host->ai_addr != 0 ) {
      printf( "---- 1\n" );
      while ( host != 0 ) {
        /* *bi++ = *host->ai_addr; */
        for ( i = 0; i < 14; ++i ) {
          printf( "'%x'", (*host->ai_addr).sa_data[i] );
        }
        printf( "\n" );
        host = host->ai_next;
      }
      printf( "---- 2\n" );
    }
  } else {
    printf( "Errno: %d\n", _errno );
  }

  if ( hosts_list != 0 ) {
    freeaddrinfo( hosts_list );
  }

  return 0;
}
