// -*- C++ -*- Time-stamp: <00/10/06 22:06:20 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
 * ParallelGraphics Ltd.
 * 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include <cerrno>
#include <sockios/sockmgr.h>

__STL_BEGIN_NAMESPACE

__PG_DECLSPEC
void basic_sockmgr::open( int port, sock_base::stype type, sock_base::protocol prot )
{
  if ( is_open() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _errno = 0;
#ifdef WIN32
  WSASetLastError( 0 );
#endif
  if ( prot == sock_base::inet ) {
    _fd = socket( PF_INET, type, 0 );
    if ( _fd == -1 ) {
      _state |= sock_base::sockfailbit;
#ifdef WIN32
      _errno = WSAGetLastError();
#else
      _errno = errno;
#endif
      return;
    }
    _address.inet.sin_family = AF_INET;
    _address.inet.sin_port = htons( port );
    _address.inet.sin_addr.s_addr = htons( INADDR_ANY );
    int attempt = 0;

    while ( ::bind( _fd, &_address.any, sizeof(_address) ) == -1 ) {
#ifdef WIN32
      _errno = WSAGetLastError();
      if ( attempt++ == 10 || _errno != WSAEADDRINUSE )
#else
      _errno = errno;
      if ( attempt++ == 10 || _errno != EADDRINUSE )
#endif
      {
	_state |= sock_base::bindfailbit;
#ifdef WIN32
	::closesocket( _fd );
#else
	::close( _fd );
#endif
	return;
      }
#ifdef WIN32
      ::Sleep( 20000 );
#else
      ::sleep( 20 );
#endif
    }	    
    if ( attempt > 0 ) {
      _errno = 0;
#ifdef WIN32
      WSASetLastError( 0 );
#else
      errno = 0;
#endif
    }
    if ( type == sock_base::sock_stream ||
	 type == sock_base::sock_seqpacket ) {
      // let's try reuse local address
      setoptions( sock_base::so_reuseaddr, true );
      // I am shure, this is socket of type SOCK_STREAM | SOCK_SEQPACKET,
      // so don't check return code from listen
      ::listen( _fd, SOMAXCONN );
    }
  } else if ( prot == sock_base::local ) {
    return;
  } else {
    return;
  }
  _state = ios_base::goodbit;
  _errno = 0; // if any
  _open = true;

  return;
}

__PG_DECLSPEC
void basic_sockmgr::close()
{
  if ( !is_open() ) {
    return;
  }
  shutdown( sock_base::stop_in | sock_base::stop_out );
#ifdef WIN32
  ::closesocket( _fd );
#else
  ::close( _fd );
#endif
  _fd = -1;
  _open = false;
}

void basic_sockmgr::shutdown( sock_base::shutdownflg dir )
{
  if ( is_open() ) {
    if ( (dir & (sock_base::stop_in | sock_base::stop_out)) ==
         (sock_base::stop_in | sock_base::stop_out) ) {
      ::shutdown( _fd, 2 );
    } else if ( dir & sock_base::stop_in ) {
      ::shutdown( _fd, 0 );
    } else if ( dir & sock_base::stop_out ) {
      ::shutdown( _fd, 1 );
    }
  }
}

void basic_sockmgr::setoptions( sock_base::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  if ( is_open() ) {
    if ( optname != sock_base::so_linger ) {
      if ( setsockopt( _fd, SOL_SOCKET, (int)optname, (const void *)&on_off,
                       (socklen_t)sizeof(bool) ) != 0 ) {
        _state |= sock_base::sockfailbit;
#ifdef WIN32
        _errno = WSAGetLastError();
#else
        _errno = errno;
#endif
      }
    } else {
      linger l;
      l.l_onoff = on_off ? 1 : 0;
      l.l_linger = __v;
      if ( setsockopt( _fd, SOL_SOCKET, (int)optname, (const void *)&l,
                       (socklen_t)sizeof(linger) ) != 0 ) {
        _state |= sock_base::sockfailbit;
#ifdef WIN32
        _errno = WSAGetLastError();
#else
        _errno = errno;
#endif
      }
      
    }
  } else {
    _state |= sock_base::sockfailbit;
  }
#endif
}

__STL_END_NAMESPACE
