// -*- C++ -*- Time-stamp: <03/07/04 22:29:04 ptr>

/*
 * Copyright (c) 2003
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.2
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

#include <mt/xmt.h>
#include <iostream>

#  include <sys/socket.h>
#  include <stropts.h>
#  ifdef __sun
#    include <sys/conf.h>
#  endif
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#include <errno.h>
#include <unistd.h>

using namespace __impl;
using namespace std;

union _xsockaddr {
    sockaddr_in inet;
    sockaddr    any;
};

Condition cnd;
int _fd;

int thread_entry_call( void * )
{
  _fd = socket( PF_INET, SOCK_STREAM, 0 );
  int _errno;
  _xsockaddr _address;
  int port = 10000;

  _address.inet.sin_family = AF_INET;
  _address.inet.sin_port = htons( port );
  _address.inet.sin_addr.s_addr = htons( INADDR_ANY );


  if ( ::bind( _fd, &_address.any, sizeof(_address) ) == -1 ) {
#ifdef WIN32
    _errno = WSAGetLastError();
#else
    _errno = errno;
#endif
#ifdef WIN32
    ::closesocket( _fd );
#else
    ::close( _fd );
#endif
    _fd = -1;
    return -1;
  }

  ::listen( _fd, SOMAXCONN );

  fd_set _pfdr;
  fd_set _pfde;
  fd_set _pfdw;
  FD_ZERO( &_pfdr );
  FD_ZERO( &_pfde );
  FD_ZERO( &_pfdw );
  FD_SET( _fd, &_pfdr );
  FD_SET( _fd, &_pfde );
  FD_SET( _fd, &_pfdw );
  cerr << "signal" << endl;
  cnd.set( true );
  int ret = select( _fd + 1, &_pfdr, &_pfdw, &_pfde, 0 );

  _xsockaddr addr;
  size_t sz = sizeof( sockaddr_in );

  int _sd = ::accept( _fd, &addr.any, &sz );

  cerr << "pass select, " << (int)FD_ISSET( _fd, &_pfdr )
       << ", " << (int)FD_ISSET( _fd, &_pfdw )
       << ", " << (int)FD_ISSET( _fd, &_pfde ) << ", " << ret
       << ", accept say " << _sd << endl;

  return 0;
}

int thread_other_call( void * )
{
  cnd.try_wait();
  cerr << "wait pass" << endl;
  timespec t;
  t.tv_sec = 2;
  t.tv_nsec = 0;
  Thread::delay( &t );
  // ::close( _fd );
  shutdown( _fd, 2 );
  cerr << "close" << endl;
}

int main( int, char * const * )
{
  cnd.set( false );

  Thread t( thread_entry_call );
  Thread t2( thread_other_call );

  t.join();
  t2.join();

  return 0;
}
