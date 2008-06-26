// -*- C++ -*- Time-stamp: <07/02/12 14:49:26 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
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

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include <cerrno>
#include <sockios/sockmgr.h>

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

int basic_sockmgr::_idx = -1;

void basic_sockmgr::Init::__at_fork_prepare()
{
}

void basic_sockmgr::Init::__at_fork_child()
{
  basic_sockmgr::_idx = xmt::Thread::xalloc();
}

void basic_sockmgr::Init::__at_fork_parent()
{
}

void basic_sockmgr::Init::_guard( int direction )
{
  static xmt::mutex _init_lock;
  static int _count = 0;

  if ( direction ) {
    if ( _count++ == 0 ) {
#ifdef _PTHREADS
      pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child );
#endif
      basic_sockmgr::_idx = xmt::Thread::xalloc();
    }
  }
}

basic_sockmgr::Init::Init()
{ _guard( 1 ); }

basic_sockmgr::Init::~Init()
{ _guard( 0 ); }

char Init_buf[128];

basic_sockmgr::basic_sockmgr() :
    _errno( 0 ),
    _mode( ios_base::in | ios_base::out ),
    _state( ios_base::goodbit ),
    _fd( -1 )
{
  new( Init_buf ) Init();
}

basic_sockmgr::~basic_sockmgr()
{
  basic_sockmgr::close();
  ((Init *)Init_buf)->~Init();
}

void basic_sockmgr::open( const in_addr& addr, int port, sock_base::stype type, sock_base::protocol prot )
{
  MT_REENTRANT( _fd_lck, _1 );
  if ( is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _errno = 0;
#ifdef WIN32
  ::WSASetLastError( 0 );
#endif
  if ( prot == sock_base::inet ) {
    _fd = socket( PF_INET, type, 0 );
    if ( _fd == -1 ) {
      _state |= ios_base::failbit | ios_base::badbit;
#ifdef WIN32
      _errno = ::WSAGetLastError();
#else
      _errno = errno;
#endif
      return;
    }
    // _open = true;
    _address.inet.sin_family = AF_INET;
    _address.inet.sin_port = htons( port );
    _address.inet.sin_addr.s_addr = addr.s_addr;

    if ( type == sock_base::sock_stream ||
	 type == sock_base::sock_seqpacket ) {
      // let's try reuse local address
      setoptions_unsafe( sock_base::so_reuseaddr, true );
    }

    if ( ::bind( _fd, &_address.any, sizeof(_address) ) == -1 ) {
#ifdef WIN32
      _errno = ::WSAGetLastError();
#else
      _errno = errno;
#endif
      _state |= ios_base::failbit;
#ifdef WIN32
      ::closesocket( _fd );
#else
      ::close( _fd );
#endif
      _fd = -1;
      return;
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

  return;
}

__FIT_DECLSPEC
void basic_sockmgr::open( unsigned long addr, int port, sock_base::stype type, sock_base::protocol prot )
{
  in_addr _addr;
  _addr.s_addr = htonl( addr );
  basic_sockmgr::open( _addr, port, type, prot );
}

__FIT_DECLSPEC
void basic_sockmgr::open( int port, sock_base::stype type, sock_base::protocol prot )
{
  basic_sockmgr::open(INADDR_ANY, port, type, prot);
}

__FIT_DECLSPEC
void basic_sockmgr::close()
{
  MT_REENTRANT( _fd_lck, _1 );
  if ( !is_open_unsafe() ) {
    return;
  }
#ifdef WIN32
  ::closesocket( _fd );
#else
  ::shutdown( _fd, 2 );
  ::close( _fd );
#endif
  _fd = -1;
}

__FIT_DECLSPEC
void basic_sockmgr::shutdown( sock_base::shutdownflg dir )
{
  MT_REENTRANT( _fd_lck, _1 );
  if ( is_open_unsafe() ) {
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

__FIT_DECLSPEC
void basic_sockmgr::setoptions_unsafe( sock_base::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  if ( is_open_unsafe() ) {
    if ( optname != sock_base::so_linger ) {
      int turn = on_off ? 1 : 0;
      if ( setsockopt( _fd, SOL_SOCKET, (int)optname, (const void *)&turn,
                       (socklen_t)sizeof(int) ) != 0 ) {
        _state |= ios_base::failbit;
#  ifdef WIN32
        _errno = WSAGetLastError();
#  else
        _errno = errno;
#  endif
      }
    } else {
      linger l;
      l.l_onoff = on_off ? 1 : 0;
      l.l_linger = __v;
      if ( setsockopt( _fd, SOL_SOCKET, (int)optname, (const void *)&l,
                       (socklen_t)sizeof(linger) ) != 0 ) {
        _state |= ios_base::failbit;
#  ifdef WIN32
        _errno = WSAGetLastError();
#  else
        _errno = errno;
#  endif
      }
      
    }
  } else {
    _state |= ios_base::failbit;
  }
#endif // __unix
}

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif
