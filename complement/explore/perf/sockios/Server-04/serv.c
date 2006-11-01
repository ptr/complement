/* -*- C -*- Time-stamp: <02/12/03 00:21:12 ptr> */

/*
 *
 * Copyright (c) 2002
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef WIN32
#  include <winsock.h>
#else /* WIN32 */
#  include <unistd.h>
#  include <sys/types.h>
#  if defined(__hpux) && !defined(_INCLUDE_XOPEN_SOURCE_EXTENDED)
#    define _INCLUDE_XOPEN_SOURCE_EXTENDED
#  endif
#  include <sys/socket.h>
#  include <stropts.h>
#  ifdef __sun
#    include <sys/conf.h>
#  endif
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  ifdef __hpux
/*
#    ifndef socklen_t // HP-UX 10.01
typedef int socklen_t;
#    endif
*/
#  endif
#  include <errno.h>
#endif /* !WIN32 */

int count = 0;
int scount = 0;

int main( int argc, char * const *argv )
{
  int port = 1990;
  int bs = 1024;
  char *self = argv[0];
  int def_bs = bs;
  int def_port = port;
  char *b = 0;
  int sd;
  union _xsockaddr {
      struct sockaddr_in inet;
      struct sockaddr    any;
  } _address;

  if ( argc > 1 ) {
    while ( argc > 1 ) {
      if ( strcmp( argv[1], "-h" ) == 0 ) {
        printf( "This is %s, Performance measure server\n\
\n\
Copyright (C) Petr Ovtchenkov, 2002\n\
\n\
Usage:\n\
  %s\n\
\n\
Options:\n\
  -h                      print this help message\n\
  -p=<int>                Listen tcp port (%d)\n\
  -b=<int>                Listen tcp port (%d)\n\
\n", self, self, def_port, def_bs );
        return 0;
      } else if ( argv[1][0] == '-' && argv[1][1] == 'p' &&
                  argv[1][2] == '=' ) {
        sscanf( argv[1] + 3, "%d", &port );
        --argc;
        ++argv;
      } else if ( argv[1][0] == '-' && argv[1][1] == 'b' &&
                  argv[1][2] == '=' ) {
        sscanf( argv[1] + 3, "%d", &bs );
        --argc;
        ++argv;
      } else {
        break;
      }
    }
  } else {
    return 0;
  }

  b = malloc( bs );
  sd = socket( PF_INET, SOCK_STREAM, 0 );

  _address.inet.sin_family = AF_INET;
  _address.inet.sin_port = htons( port );
  _address.inet.sin_addr.s_addr = htons( INADDR_ANY );

  if ( bind( sd, &_address.any, sizeof( _address ) ) == 0 ) {
    union _xsockaddr addr;
    size_t sz = sizeof( struct sockaddr_in );
    int csd;

    listen( sd, SOMAXCONN );


    csd = accept( sd, &addr.any, &sz );
    if ( csd != 0 ) {
      printf( "Server see connection\n" );
      errno = 0;
      while ( errno == 0 ) {
        int rb;
        /* printf( "Read: %d\n", bs ); */
        rb = read( csd, b, bs );
        if ( rb == 0 ) {
          count += bs;
          ++scount;
          break;
        }
        /* printf( "Read done: %d\n", rb ); */
        while ( rb < bs && errno == 0 ) {
          rb += read( csd, b + rb, bs - rb );          
        }
        if ( errno != 0 ) {
          count += rb;
          ++scount;
          break;
        }
        count += bs;
        ++scount;
      }
      printf( "Server see: client close connection (%d/%d)\n", count, scount );
    }
  } else {
    printf( "Can't bind socket\n" );
  }

  printf( "End of main\n" );
  return 0;
}
