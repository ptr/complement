// -*- C++ -*- Time-stamp: <06/06/28 10:33:02 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2005
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2000
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License Version 2.1
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

#include <string>
#include <sockios/sockstream>

#ifdef WIN32

// The Microsoft's cool programmers made two wanderful things:
// 1. All sockets must be initialized via WSAStartup
// 2. Do it procedure once per every new thread.
//    Not yet all: for one Windows this should be done
//    once per every new thread, while for others only
//    once per process (this depends upon pair Windows/sockets on it).
// 3. If we do more intialization or less than this Windows expected,
//    we fail with fatal results.
// 
// So I do this via Tls* functions (TlsAlloc() in plock.cc)

#include <ostream>
#include <sstream>
#include <mt/xmt.h>

#  if 0
extern "C" int APIENTRY
DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
{
  return TRUE;   // ok
}
#  endif

namespace xmt {

static char __xbuff[16];
static const char *WINSOCK_ERR_MSG  = "WinSock DLL not 2.0";
static const char *WINSOCK_ERR_MSG1 = "WinSock DLL not 2.0 (or not Win95)";

// extern __declspec( dllexport ) int __thr_key; // xmt.cc
int __thr_key = TlsAlloc();

// static int _sb_idx = Thread::xalloc();

} // namespace xmt

namespace std {

static int __glob_init_cnt = 0;
static int __glob_init_wsock2 = 0;

static xmt::Mutex _SI_lock;

enum {
  WINDOWS_NT_4,
  WINDOWS_NT_3,
  WINDOWS_98,
  WINDOWS_95,
  WINDOWS_95_WSOCK2,
  WINDOWS_3_1
};

int WinVer()
{
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);
  GetVersionEx(&info);

  if ( info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) { // Win 9x
    if ( info.dwMinorVersion == 0 ) {
      if ( __glob_init_wsock2 != 0 ) {
        return WINDOWS_95_WSOCK2;
      }
      return WINDOWS_95;
    }
    return WINDOWS_98;
  } else if ( info.dwPlatformId == VER_PLATFORM_WIN32_NT ) { // Win NT
    if ( info.dwMajorVersion == 3 ) {
      return WINDOWS_NT_3;
    }
    return WINDOWS_NT_4;
  }
  return WINDOWS_3_1;
}

__FIT_DECLSPEC
sock_base::Init::Init()
{
  MT_REENTRANT( _SI_lock, _1 );
  int __err = 0;
  try {
    int __tls_init_cnt = (int)TlsGetValue( xmt::__thr_key );
//  long& __tls_init_cnt = xmt::Thread::iword( _sb_idx );
    if ( __tls_init_cnt++ == 0 ) {
      int win_ver = WinVer();

      WORD    __vers;
      WSADATA __wsadata;

      if ( win_ver != WINDOWS_3_1 && !(win_ver == WINDOWS_95 && __glob_init_cnt++ > 0) ) {
        __vers = MAKEWORD( 2, 0 );
        __err = WSAStartup( __vers, &__wsadata );

        if ( __err != 0 && __err != WSAVERNOTSUPPORTED ) {
          TlsSetValue( xmt::__thr_key, 0 );
          throw domain_error( xmt::WINSOCK_ERR_MSG );
        }
        if ( LOBYTE(__wsadata.wVersion) != 2 || HIBYTE(__wsadata.wVersion) != 0 ) {
          if ( win_ver != WINDOWS_95 ) {
            WSACleanup();
            TlsSetValue( xmt::__thr_key, 0 );
            throw domain_error( xmt::WINSOCK_ERR_MSG1 );
          }
        } else if ( win_ver == WINDOWS_95 ) {
          __glob_init_wsock2 = 1;
        }

      }
    }
    TlsSetValue( xmt::__thr_key, (void *)__tls_init_cnt );
    int a = 1;
    int b = 2;
    int c = 3;
    for ( int j = 0; j < 100000; ++j ) {
      swap( a, b );
      swap( b, c );
      a = j;
    }
  }
  catch ( domain_error& err ) {
    ostringstream ss;
    ss << err.what() << ", Error code " << __err << "\n"
       << __FILE__ << ":" << __LINE__ << endl;
    MessageBox( 0, ss.str().c_str(), "Planet Problem", MB_OK );
    throw;
  }
  catch ( ... ) {
    ostringstream ss;
    ss << "Unspesified exception, Error code " << __err << "\n"
       << __FILE__ << ":" << __LINE__ << endl;
    MessageBox( 0, ss.str().c_str(), "Planet Problem", MB_OK );
    throw;
  }
}

__FIT_DECLSPEC
sock_base::Init::~Init()
{
  MT_REENTRANT( _SI_lock, _1 );
  int __tls_init_cnt = (int)TlsGetValue( xmt::__thr_key );
  int win_ver = WinVer();
  // --__glob_init_cnt; // only for Win 95
  if ( --__tls_init_cnt == 0 ) {
    if ( win_ver != WINDOWS_95 ) {
      WSACleanup();
      int a = 1;
      int b = 2;
      int c = 3;
      for ( int j = 0; j < 100000; ++j ) {
        swap( a, b );
        swap( b, c );
        a = j;
      }
    } else if ( __glob_init_cnt == 0 ) {
      // WSACleanup();
    }
  }
  TlsSetValue( xmt::__thr_key, (void *)__tls_init_cnt );
}

__FIT_DECLSPEC
sock_base::sock_base()
{
  new( xmt::__xbuff ) Init();
}

__FIT_DECLSPEC
sock_base::~sock_base()
{
  reinterpret_cast<Init *>(xmt::__xbuff)->~Init();
}

} // namespace std

#endif // WIN32

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

::in_addr findhost( const char *hostname ) throw( std::domain_error )
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

  return inet;
}

std::string hostname( unsigned long inet_addr )
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
  in.s_addr = inet_addr;
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

int service( const char *name, const char *proto ) throw( std::domain_error )
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

std::string service( int port, const char *proto ) throw( std::domain_error )
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
