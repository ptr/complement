// -*- C++ -*- Time-stamp: <00/09/08 13:55:57 ptr>

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


__STL_END_NAMESPACE
