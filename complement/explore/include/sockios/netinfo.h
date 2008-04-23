// -*- C++ -*- Time-stamp: <07/09/06 23:42:19 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005-2008
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __SOCKIOS_NETINFO_H
#define __SOCKIOS_NETINFO_H

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifdef __FIT_NONREENTRANT
extern "C" int x_getaddrinfo(const char *, const char *, const struct addrinfo *, struct addrinfo **);
extern "C" void x_freeaddrinfo( struct addrinfo * );
extern "C" int x_res_search(const char *, int, int, unsigned char *, int);
extern "C" int x_res_init(void);
#endif

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <string>
#include <stdexcept>

#ifdef WIN32
#  include <winsock2.h>
#else // WIN32
#  include <unistd.h>
#  include <sys/types.h>
#  if defined(__hpux) && !defined(_INCLUDE_XOPEN_SOURCE_EXTENDED)
#    define _INCLUDE_XOPEN_SOURCE_EXTENDED
#  endif
#  include <sys/socket.h>
#  if !defined(__UCLIBC__) && !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
#   include <stropts.h>
#  endif
#  ifdef __sun
#    include <sys/conf.h>
#  endif
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  ifdef __hpux
// #    ifndef socklen_t // HP-UX 10.01
// typedef int socklen_t;
// #    endif
#  endif
#  include <cerrno>
#endif // !WIN32

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

in_addr findhost( const char *hostname ) throw( std::domain_error );
std::string hostname( unsigned long inet_addr );
std::string hostname();

int service( const char *name, const char *proto ) throw( std::domain_error );
std::string service( int port, const char *proto ) throw( std::domain_error );

/*
 * Expected host name, return (via back insert iterator)
 * all IPs (in_addr) for specified host name; nothing will be added in case of
 * failure.
 */
template <class BackInsertIterator>
void gethostaddr( const char *hostname, BackInsertIterator bi )
{
  int _errno = 0;

#ifndef __FIT_GETHOSTBYADDR
  hostent _host;
#  ifndef __hpux
  char tmpbuf[4096];
#  else // __hpux
  hostent_data tmpbuf;
#  endif // __hpux
#  ifdef __linux
  hostent *host = 0;
  gethostbyname_r( hostname, &_host, tmpbuf, 4096, &host, &_errno );
#  elif defined(__hpux)
  _errno = gethostbyname_r( hostname, &_host, &tmpbuf );
  hostent *host = &_host;
#  elif defined(__sun)
  hostent *host = gethostbyname_r( hostname, &_host, tmpbuf, 4096, &_errno );
#  else // !__linux !__hpux !__sun
#    error "Check port of gethostbyname_r"
#  endif // __linux __hpux __sun
#else // __FIT_GETHOSTBYADDR
  hostent *host = gethostbyname( hostname );
#  ifdef WIN32
  if ( host == 0 ) {
    _errno = WSAGetLastError();

    // specific to Wins only:
    // cool M$ can't resolve IP address in gethostbyname, try once more
    // via inet_addr() and gethostbyaddr()
    // Returned _errno depend upon WinSock version, and applied patches,
    // with some of it even gethostbyname may be succeed.
    if ( _errno == WSAHOST_NOT_FOUND || _errno == WSATRY_AGAIN ) {
      unsigned long ipaddr = ::inet_addr( hostname );
      if ( ipaddr != INADDR_NONE ) {
        host = gethostbyaddr( (const char *)&ipaddr, sizeof(ipaddr), AF_INET );
        if ( host != 0 ) { // Oh, that's was IP indeed...
          memcpy( (char *)&inet, (char *)host->h_addr, host->h_length );
          WSASetLastError( 0 ); // clear error
          _errno = 0;
        } else {
          _errno = WSAGetLastError();
        }
      }
    }
  }
#  endif // WIN32
#endif // __FIT_GETHOSTBYADDR

  if ( host != 0 && host->h_length == sizeof(in_addr) ) {
    for ( char **_inet = host->h_addr_list; *_inet != 0; ++_inet ) {
      *bi++ = *((in_addr *)*_inet);
    }
  }
}

/*
 * Expected host name, return (via back insert iterator)
 * all sockaddr for specified host name; nothing will be added in case of
 * failure. (Alternative implementation to gethostaddr above; it most
 * useful on systems without reentrant gethostbyname_r, but that has
 * reentrant getaddrinfo, like FreeBSD >= 5.3)
 */
template <class BackInsertIterator>
void gethostaddr2( const char *hostname, BackInsertIterator bi )
{
  // addrinfo hints;
  addrinfo *hosts_list = 0;

#ifndef __FIT_NONREENTRANT
  int _errno = getaddrinfo( hostname, 0, 0, &hosts_list );
#else
  int _errno = x_getaddrinfo( hostname, 0, 0, &hosts_list );
#endif
  if ( _errno == 0 ) {
    addrinfo *host = hosts_list;
    if ( host != 0 && host->ai_addr != 0 ) {
      while ( host != 0 ) {
        // *bi++ = ((in_addr *)host->ai_addr->sa_data)->s_addr;
        *bi++ = *host->ai_addr;
        host = host->ai_next;
      }
    }
  }
  if ( hosts_list != 0 ) {
#ifndef __FIT_NONREENTRANT
    freeaddrinfo( hosts_list );
#else
    x_freeaddrinfo( hosts_list );
#endif
  }
}

struct net_iface
{
    net_iface()
      { }

    net_iface( const char *nm, unsigned f, const sockaddr& address ) :
        name( nm ),
        flags( f )
      {
        addr.any = address;
      }

    std::string name;
    union {
        sockaddr_in inet;
        sockaddr    any;
    } addr;
    // struct in_addr mask;
    unsigned flags;
};

template <class C>
void get_ifaces( C& lst )
{
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  struct ifconf ifc;
  char st_buf[100 * sizeof(struct ifreq)];
  char *buf = 0;
  int len = sizeof( st_buf );
  ifc.ifc_len = len;
  ifc.ifc_buf = st_buf;
  for ( ; ; ) {
    if ( ioctl(sock, SIOCGIFCONF, &ifc) < 0 ) {
      if ( errno != EINVAL ) {
        if ( ifc.ifc_buf != st_buf ) {
          free( ifc.ifc_buf );
        }
        close( sock );
        throw std::runtime_error( std::string("SIOCGIFCONF ioctl error getting list of interfaces") );
      }
    } else {
      if ( ifc.ifc_len < sizeof(struct ifreq) ) {
        if ( ifc.ifc_buf != st_buf ) {
          free( ifc.ifc_buf );
        }
        close( sock );
        throw std::runtime_error( std::string("SIOCGIFCONF ioctl gave too small return buffer") );
      }
    }
    if ( ifc.ifc_len >= len ) {
      len = ifc.ifc_len + 10 * sizeof(struct ifreq);
      if ( ifc.ifc_buf != st_buf ) {
        free( ifc.ifc_buf );
      }
      ifc.ifc_buf = (char *)malloc( len );
      ifc.ifc_len = len;
    } else {
      break;
    }
  }
  struct ifreq* ifr = (struct ifreq *) ifc.ifc_req;
  struct ifreq* last = (struct ifreq *) ((char *) ifr + ifc.ifc_len);
  struct ifreq ifrflags;
  while ( ifr < last ) {
    memset(&ifrflags, 0, sizeof(ifrflags) );
    strncpy( ifrflags.ifr_name, ifr->ifr_name, sizeof( ifrflags.ifr_name ) );
    if ( ioctl(sock, SIOCGIFFLAGS, (char *)&ifrflags) < 0 ) {
      if ( errno != ENXIO ) {
        if ( ifc.ifc_buf != st_buf ) {
          free( ifc.ifc_buf );
        }
        close( sock );
        throw std::runtime_error( std::string("SIOCGIFFLAGS error getting flags for interface") );
      }
    }
    lst.push_back( net_iface( ifr->ifr_name, ifrflags.ifr_flags, ifr->ifr_addr ) );
    ifr = (struct ifreq *) ((char *) ifr + sizeof(struct ifreq));
  }

  if ( ifc.ifc_buf != st_buf ) {
    free( ifc.ifc_buf );
  }
  close( sock );
}

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif

#endif // __SOCKSIOS_NETINFO_H