/* -*- C -*- Time-stamp: <02/12/02 19:00:41 ptr> */

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

struct in_addr findhost( const char *hostname )
{
  struct in_addr inet;
  int _errno;

#ifndef __GETHOSTBYADDR__
  struct hostent _host;
#  ifndef __hpux
  char tmpbuf[1024];
#  else /* __hpux */
  struct hostent_data tmpbuf;
#  endif /* __hpux */
#  ifdef __linux
  struct hostent *host = 0;
  gethostbyname_r( hostname, &_host, tmpbuf, 1024, &host, &_errno );
#  elif defined(__hpux)
  _errno = gethostbyname_r( hostname, &_host, &tmpbuf );
  struct hostent *host = &_host;
#  elif defined(__sun)
  struct hostent *host = gethostbyname_r( hostname, &_host, tmpbuf, 1024, &_errno );
#  else /* !__linux !__hpux !__sun */
#    error "Check port of gethostbyname_r"
#  endif /* __linux __hpux __sun */
  if ( host != 0 ) {
    memcpy( (char *)&inet, (char *)host->h_addr, host->h_length );
  }
#else /* __GETHOSTBYADDR__ */
  struct hostent *host = gethostbyname( hostname );
  if ( host != 0 ) {
    memcpy( (char *)&inet, (char *)host->h_addr, host->h_length );
  }
#  ifdef WIN32
    else {
    _errno = WSAGetLastError();
    /*
       specific to Wins only:
       cool M$ can't resolve IP address in gethostbyname, try once more
       via inet_addr() and gethostbyaddr()
       Returned _errno depend upon WinSock version, and applied patches,
       with some of it even gethostbyname may be succeed.
    */
    if ( _errno == WSAHOST_NOT_FOUND || _errno == WSATRY_AGAIN ) {
      unsigned long ipaddr = ::inet_addr( hostname );
      if ( ipaddr != INADDR_NONE ) {
        host = gethostbyaddr( (const char *)&ipaddr, sizeof(ipaddr), AF_INET );
        if ( host != 0 ) { /* Oh, that's was IP indeed... */
          memcpy( (char *)&inet, (char *)host->h_addr, host->h_length );
          WSASetLastError( 0 ); /* clear error */
          _errno = 0;
        } else {
          _errno = WSAGetLastError();
        }
      }
    }
  }
#  endif /* WIN32 */
#endif /* __GETHOSTBYADDR__ */
  if ( host == 0 ) {
    /* return 0; */ /* Should return something... */
  }

  return inet;
}

int main( int argc, char * const *argv )
{
  int port = 1990;
  char defhost[] = "localhost";
  char *host = defhost;
  int ni = 10000;
  int bs = 1024;
  char *self = argv[0];
  int def_ni = ni;
  int def_bs = bs;
  int def_port = port;
  char *b = 0;
  int sd;
  union {
      struct sockaddr_in inet;
      struct sockaddr    any;
  } _address;

  if ( argc > 1 ) {
    while ( argc > 1 ) {
      if ( strcmp( argv[1], "-h" ) == 0 ) {
        printf( "This is %s, Performance measure client\n\
\n\
Copyright (C) Petr Ovtchenkov, 2002\n\
\n\
Usage:\n\
  %s\n\
\n\
Options:\n\
  -h                      print this help message\n\
  -p=<int>                Connect to tcp port (%d)\n\
  -host=<string>          Connect to host (%s)\n\
  -n=<int>                number of iterations (%d)\n\
  -b=<int>                block size (%d)\n\
\n", self, self, def_port, defhost, def_ni, def_bs );
        return 0;
      } else if ( argv[1][0] == '-' && argv[1][1] == 'p' &&
                  argv[1][2] == '=' ) {
        sscanf( argv[1] + 3, "%d", &port );
        --argc;
        ++argv;
      } else if ( argv[1][0] == '-' && argv[1][1] == 'n' &&
                  argv[1][2] == '=' ) {
        sscanf( argv[1] + 3, "%d", &ni );
        --argc;
        ++argv;
      } else if ( argv[1][0] == '-' && argv[1][1] == 'b' &&
                  argv[1][2] == '=' ) {
        sscanf( argv[1] + 3, "%d", &bs );
        --argc;
        ++argv;
      } else if ( argv[1][0] == '-' && argv[1][1] == 'h' &&
                  argv[1][2] == 'o' && argv[1][3] == 's' &&
                  argv[1][4] == 't' &&
                  argv[1][5] == '=' ) {
        host = malloc( strlen( argv[1] + 6 ) );
        strcpy( host, argv[1] + 6 );
        --argc;
        ++argv;
      } else {
        break;
      }
    }
  } else {
    return 0;
  }

  /* printf( "Client: %d %d\n", ni, bs ); */

  b = malloc( bs );
  sd = socket( PF_INET, SOCK_STREAM, 0 );

  _address.inet.sin_family = AF_INET;
  /* htons is a define at least in Linux 2.2.5-15, and it's expantion fail
     for gcc 2.95.3 */
#if defined(linux) && defined(htons) && defined(__bswap_16)
  _address.inet.sin_port = ((((port) >> 8) & 0xff) | (((port) & 0xff) << 8));
#else
  _address.inet.sin_port = htons( port );
#endif /* linux && htons */
  _address.inet.sin_addr = findhost( host );

  /* printf( "Client: X1\n" ); */

  if ( connect( sd, &_address.any, sizeof( _address ) ) == 0 ) {
    int i;
    int wb;
    /* printf( "Client: X2\n" ); */
    errno = 0;
    for ( i = 0; i < ni; ++i ) {
      if ( errno == 0 ) {
        /*
        if ( i % 102400 == 0 ) {
          printf( "%d\n", i );
        }
        */

        /* if ( bs < 4 * 1024 ) { */
          wb = write( sd, b, bs );
          /* } else {
          int sbs = 0;
          wb = 0;
          while ( sbs < bs ) {
            wb += write( sd, b + sbs, (4 * 1024 < (bs-sbs) ? 4*1024 : ( bs - sbs )) );
            sbs += 4 * 1024;
          }
          */
        }
        if ( wb != bs ) {
          printf( "Something wrong (%d)/ %d\n", i, errno );
          break;
        }
      } else {
        printf( "Something wrong (%d), %d\n", i, errno );
        break;
      }
    }   
  }

  close( sd );

  free( b );

  return 0;
}
