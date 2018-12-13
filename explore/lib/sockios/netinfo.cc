// -*- C++ -*- Time-stamp: <09/04/02 16:34:54 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2005, 2009
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2000
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <string>
#include <sockios/netinfo.h>

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

::in_addr_t findhost( const char *hostname )
{
  in_addr inet;
  int _errno;

#ifndef __FIT_GETHOSTBYADDR
  hostent _host;
#  ifndef __hpux
  char tmpbuf[1024];
#  else // __hpux
  hostent_data tmpbuf;
#  endif // __hpux
#  ifdef __linux
  hostent *host = 0;
  gethostbyname_r( hostname, &_host, tmpbuf, 1024, &host, &_errno );
#  elif defined(__hpux)
  _errno = gethostbyname_r( hostname, &_host, &tmpbuf );
  hostent *host = &_host;
#  elif defined(__sun)
  hostent *host = gethostbyname_r( hostname, &_host, tmpbuf, 1024, &_errno );
#  else // !__linux !__hpux !__sun
#    error "Check port of gethostbyname_r"
#  endif // __linux __hpux __sun
  if ( host != 0 ) {
    memcpy( (char *)&inet, (char *)host->h_addr, host->h_length );
  }
#else // __FIT_GETHOSTBYADDR
  hostent *host = gethostbyname( hostname );
  if ( host != 0 ) {
    memcpy( (char *)&inet, (char *)host->h_addr, host->h_length );
  }
#  ifdef WIN32
    else {
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
  if ( host == 0 ) {
    throw std::domain_error( "host not found" );
  }

#ifdef __LITTLE_ENDIAN
  return ((inet.s_addr >> 24) & 0xff) | ((inet.s_addr & 0xff) << 24) | ((inet.s_addr & 0xff00) << 8) | ((inet.s_addr >> 8) & 0xff00);
#elif defined(__BIG_ENDIAN)
  return inet.s_addr;
#else
#  error Undefined byte order
#endif
}

std::string hostname( in_addr_t inet_addr )
{
  std::string _hostname;

#ifdef __FIT_GETHOSTBYADDR
  hostent *he;
#else
  hostent he;
#ifndef __hpux
  char tmp_buff[1024];
#else
  hostent_data tmp_buff;
#endif
#  ifdef __linux
  hostent *phe = 0;
#  endif
#endif
  int err = 0;
  in_addr in;
#ifdef __LITTLE_ENDIAN
  in.s_addr =  ((inet_addr >> 24) & 0xff) | ((inet_addr & 0xff) << 24) | ((inet_addr & 0xff00) << 8) | ((inet_addr >> 8) & 0xff00);
#elif defined(__BIG_ENDIAN)
  in.s_addr = inet_addr;
#else
#  error Undefined byte order
#endif
#ifdef __FIT_GETHOSTBYADDR
  // For Win 'he' is thread private data, so that's safe
  // It's MT-safe also for HP-UX 11.00
  he = gethostbyaddr( (char *)&in.s_addr, sizeof(in_addr), AF_INET );
  if ( he != 0 ) {
    _hostname = he->h_name;
  } else {
    _hostname = "unknown";
  }
#else // __FIT_GETHOSTBYADDR
  if (
#  ifdef __sun
       gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
                        &he, tmp_buff, 1024, &err ) != 0
#  elif defined(__linux)
       gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
                        &he, tmp_buff, 1024, &phe, &err ) == 0
#  elif defined(__hpux) // reentrant variant for HP-UX before 11.00
       gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
                        &he, &tmp_buff ) == 0
#  else
#    error "Check port of gethostbyaddr_r"
#  endif
     )
  {
    _hostname = he.h_name;
  } else {
    _hostname = "unknown";
  }
#endif // __FIT_GETHOSTBYADDR

  _hostname += " [";
  _hostname += inet_ntoa( in );
  _hostname += "]";

  return _hostname;
}

std::string hostname()
{
  std::string _hostname;
  char tmp_buff[1024];

  if ( gethostname( tmp_buff, 1024 ) == 0 ) {
    _hostname = tmp_buff;
  } else {
    _hostname = "unknown";
  }
  // getdomainname may be called here, but POSIX not specify such call

  return _hostname;
}

int service( const char *name, const char *proto )
{
#ifdef _WIN32
  typedef u_short uint16_t;
#endif
#ifndef __FIT_GETHOSTBYADDR
  char tmp_buf[1024];
  struct servent se;
#  ifdef __linux
  struct servent *sep = 0;
  if ( getservbyname_r( name, proto, &se, tmp_buf, 1024, &sep ) != 0 ) {
    throw std::domain_error( "service not found" );
  }
  return ntohs( uint16_t(se.s_port) );
#  endif
#  ifdef __sun
  if ( getservbyname_r( name, proto, &se, tmp_buf, 1024 ) == 0 ) {
    throw std::domain_error( "service not found" );
  }
  return ntohs( uint16_t(se.s_port) );
#  endif
#else // __FIT_GETHOSTBYADDR
  struct servent *s = ::getservbyname( name, proto );
  if ( s == 0 ) {
    throw std::domain_error( "service not found" );
  }
  return ntohs( uint16_t(s->s_port) );
#endif
}

std::string service( int port, const char *proto )
{
#ifdef _WIN32
  typedef u_short uint16_t;
#endif
  std::string _servname;

  port = htons( uint16_t(port) );

#ifndef __FIT_GETHOSTBYADDR
  char tmp_buf[1024];
  struct servent se;
#  ifdef __linux
  struct servent *sep = 0;
  if ( getservbyport_r( port, proto, &se, tmp_buf, 1024, &sep ) != 0 ) {
    throw std::domain_error( "service not found" );
  }
  _servname.assign( se.s_name );
  return _servname;
#  endif
#  ifdef __sun
  if ( getservbyport_r( port, proto, &se, tmp_buf, 1024 ) == 0 ) {
    throw std::domain_error( "service not found" );
  }
  _servname.assign( se.s_name );
  return _servname;
#  endif
#else // __FIT_GETHOSTBYADDR
  struct servent *s = ::getservbyport( port, proto );
  if ( s == 0 ) {
    throw std::domain_error( "service not found" );
  }
  _servname.assign( s->s_name );
  return _servname;
#endif
}

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif
