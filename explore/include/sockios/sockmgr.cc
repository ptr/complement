// -*- C++ -*- Time-stamp: <01/03/01 10:49:28 ptr>

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

#include <algorithm>

#ifdef __unix
extern "C" int nanosleep(const struct timespec *, struct timespec *);
#endif

using __impl::Thread;

__STL_BEGIN_NAMESPACE

template <class Connect>
void sockmgr_stream<Connect>::open( int port, sock_base::stype t )
{
  basic_sockmgr::open( port, t, sock_base::inet );
  if ( is_open() ) {
    if ( t == sock_base::sock_stream ) {
      _accept = &_Self_type::accept_tcp;
    } else if ( t == sock_base::sock_dgram ) {
      _accept = &_Self_type::accept_udp;
    } else {
      throw invalid_argument( "sockmgr_stream" );
    }
    loop_id.launch( loop, this );
  }
}

template <class Connect>
sockmgr_client *sockmgr_stream<Connect>::accept_tcp()
{
  if ( !is_open() ) {
    return 0;
  }

  _xsockaddr addr;
#if defined(_WIN32) || (defined(__hpux) && !defined(_INCLUDE_POSIX1C_SOURCE))
  int sz = sizeof( sockaddr_in );
#else
  size_t sz = sizeof( sockaddr_in );
#endif

  sock_base::socket_type _sd = ::accept( fd(), &addr.any, &sz );
  if ( _sd == -1 ) {
    // check and set errno
    __STL_ASSERT( _sd == -1 );
    return 0;
  }

  sockmgr_client *cl;
  try {
    _c_lock._M_acquire_lock();

    container_type::iterator i = 
      find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, -1 ) );

    if ( i == _M_c.end() ) {
      cl = new sockmgr_client();
      _M_c.push_back( cl );
    } else {
      cl = *i;
    }
  
    cl->s.open( _sd, addr.any );
    // cl->s.rdbuf()->hostname( _M_c.front()->hostname );

    _c_lock._M_release_lock();
  }
  catch ( ... ) {
    _c_lock._M_release_lock();
  }
  return cl;
}

template <class Connect>
sockmgr_client *sockmgr_stream<Connect>::accept_udp()
{
  if ( !is_open() ) {
    return 0;
  }

#if defined(_WIN32) || (defined(__hpux) && !defined(_INCLUDE_POSIX1C_SOURCE))
  int sz = sizeof( sockaddr_in );
#else
  size_t sz = sizeof( sockaddr_in );
#endif
  _xsockaddr addr;
#ifdef __unix
  timespec t;

  t.tv_sec = 0;
  t.tv_nsec = 10000;

#if !defined( _RWSTD_VER ) // bogus RogueWave
  struct pollfd pfd;
#else
  struct ::pollfd pfd;
#endif
  pfd.fd = fd();
  pfd.events = POLLIN;
#endif
#ifdef WIN32
  int t = 10;
  fd_set pfd;
#endif

  do {
#ifdef WIN32
    FD_ZERO( &pfd );
    FD_SET( fd(), &pfd );

    if ( select( fd() + 1, &pfd, 0, 0, 0 ) > 0 ) {
      // get address of caller only
      char buff[32];    
      ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr.any, &sz );
    } else {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#else
    pfd.revents = 0;
    if ( poll( &pfd, 1, -1 ) > 0 ) { // wait infinite
    // get address of caller only
      char buff[32];    
      ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr.any, &sz );
    } else {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#endif

    _c_lock._M_acquire_lock();
    container_type::iterator i = _M_c.begin();
    sockbuf *b;
    while ( i != _M_c.end() ) {
      b = (*i)->s.rdbuf();
      if ( (*i)->s.is_open() && b->stype() == sock_base::sock_dgram &&
        b->port() == addr.inet.sin_port &&
        b->inet_addr() == addr.inet.sin_addr.s_addr ) {
        break;
      }
      ++i;
    }

    sockmgr_client *cl;
    if ( i == _M_c.end() ) {
      i = find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, -1 ) );

      if ( i == _M_c.end() ) {
        cl = new sockmgr_client();
        _M_c.push_back( cl );
      } else {
        cl = *i;
      }
#ifdef __unix
      cl->s.open( dup( fd() ), addr.any, sock_base::sock_dgram );
#endif
#ifdef WIN32
      SOCKET dup_fd;
      HANDLE proc = GetCurrentProcess();
      DuplicateHandle( proc, (HANDLE)fd(), proc, (HANDLE *)&dup_fd, 0, FALSE, DUPLICATE_SAME_ACCESS );
      cl->s.open( dup_fd, addr.any, sock_base::sock_dgram );
#endif
      _c_lock._M_release_lock();
      // cl->s.rdbuf()->hostname( _M_c.front()->hostname );
      return cl;
    }
    // otherwise, thread exist and living, and I wait while it read message
    _c_lock._M_release_lock();
#ifdef __unix
    nanosleep( &t, 0 );
#endif
#ifdef WIN32
    Sleep( t );
#endif
  } while ( true );

  return 0; // never
}

template <class Connect>
int sockmgr_stream<Connect>::loop( void *p )
{
  sockmgr_stream *me = static_cast<sockmgr_stream *>(p);

#ifdef __unix
  Thread::unblock_signal( SIGINT );
#endif

  sockmgr_client *s;
  params pass;
  int ret_code = 0;

  pass.me = me;

  try {
    while ( (s = me->accept()) != 0 ) {
    
      pass.client = s;

      me->thr_mgr.launch( connection, &pass, sizeof(pass) );
    }
  }
  catch ( ... ) {
    me->close();
    throw;
  }

  return 0;
}

template <class Connect>
int sockmgr_stream<Connect>::connection( void *p )
{
  params *pass = static_cast<params *>(p);
  sockmgr_stream *me = pass->me;
  sockmgr_client *client = pass->client;

#ifdef __unix
  Thread::unblock_signal( SIGPIPE );
#endif


  try {
    Connect _proc;

    // The user connect function: application processing
    _proc.connect( client->s );

    // Enforce socket close before thread terminated: this urgent for
    // udp sockstreams policy, and not significant for tcp.
    client->s.close();
  }
  catch ( int ) { // signal
    client->s.close();
    throw;
  }
  catch ( ios_base::failure& ) {
    client->s.close();
  }
  catch ( ... ) {
    client->s.close();
    throw;
  }

  return 0;
}

// multiplexor

template <class Connect>
void sockmgr_stream_MP<Connect>::open( int port, sock_base::stype t )
{
  basic_sockmgr::open( port, t, sock_base::inet );
  if ( is_open() ) {
    if ( t == sock_base::sock_stream ) {
      _accept = &_Self_type::accept_tcp;
    } else if ( t == sock_base::sock_dgram ) {
      _accept = &_Self_type::accept_udp;
    } else {
      throw invalid_argument( "sockmgr_stream_MP" );
    }
    
    loop_id.launch( loop, this );
  }
}

template <class Connect>
sockmgr_client_MP<Connect> *sockmgr_stream_MP<Connect>::_shift_fd()
{
  sockmgr_client_MP<Connect> *msg = 0;
  int j = 1;
  while ( j < _fdcount ) {
    if ( _pfd[j].revents != 0 ) {
      // We should distinguish closed socket from income message
      typename container_type::iterator i = 
        find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, _pfd[j].fd ) );
// #ifdef __sun // Solaris return ERROR on poll, before close socket
//      __STL_ASSERT( i != _M_c.end() );
// #else
      if ( i == _M_c.end() ) { // Socket already closed (may be after read/write failure)
        // this way may not notify poll (like in HP-UX 11.00) via POLLERR flag
        // as made in Solaris
        --_fdcount;
#  if !defined( _RWSTD_VER ) // bogus RogueWave
        memmove( &_pfd[j], &_pfd[j+1], sizeof(struct pollfd) * (_fdcount - j) );
#  else
        memmove( &_pfd[j], &_pfd[j+1], sizeof(_pfd[0]) * (_fdcount - j) );
#  endif
        for ( i = _M_c.begin(); i != _M_c.end(); ++i ) {
          if ( (*i)->s.rdbuf()->fd() == -1 ) {
            (*i)->s.close();
            (*i)->_proc.close();
          }
        }
        continue;
      } else
// #endif // !__sun
      if ( _pfd[j].revents & POLLERR /* | POLLHUP | POLLNVAL */ ) { // poll first see closed socket
        --_fdcount;
#if !defined( _RWSTD_VER ) // bogus RogueWave
        memmove( &_pfd[j], &_pfd[j+1], sizeof(struct pollfd) * (_fdcount - j) );
#else
        memmove( &_pfd[j], &_pfd[j+1], sizeof(_pfd[0]) * (_fdcount - j) );
#endif
        (*i)->s.close();
// #ifndef __sun
        (*i)->_proc.close();
// #endif
        continue;
      }
      if ( msg == 0 ) {
        _pfd[j].revents = 0;
        msg = *i;
      }
    }
    ++j;
  }

  return msg;
}

#ifdef WIN32 // based on select call
template <class Connect>
sockmgr_client_MP<Connect> *sockmgr_stream_MP<Connect>::accept_tcp()
{
  if ( !is_open() ) {
    return 0;
  }

  _xsockaddr addr;
  size_t sz = sizeof( sockaddr_in );

  sockmgr_client_MP<Connect> *cl;

  do {
    if ( _fdcount == 0 ) {
      FD_ZERO( &_pfd );
      ++_fdcount;
    }
    FD_SET( fd(), &_pfd );
    if ( select( FD_SETSIZE, &_pfd, 0, 0, 0 ) < 0 ) {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }

    if ( FD_ISSET( fd(), &_pfd ) ) { // poll found event on binded socket
      sock_base::socket_type _sd = ::accept( fd(), &addr.any, &sz );
      if ( _sd == -1 ) {
        // check and set errno
        __STL_ASSERT( _sd == -1 );
        return 0;
      }

      try {
        sockmgr_client_MP<Connect> *cl_new;
        typename container_type::iterator i = 
          find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, -1 ) );
    
        if ( i == _M_c.end() ) { // we need new message processor
          cl_new = new sockmgr_client_MP<Connect>();
          _M_c.push_back( cl_new );
        } else { // we can reuse old
#ifndef __hpux
          cl_new = *i;
#else
          _M_c.erase( i );
          cl_new = new sockmgr_client_MP<Connect>();
          _M_c.push_back( cl_new );
#endif
        }

        cl_new->s.open( _sd, addr.any );

        FD_SET( _sd, &_pfd );
      }
      catch ( ... ) {
      }
    }

    cl = _shift_fd(); // find polled and return it
    if ( cl != 0 ) {
      return cl; // return message processor
    } else if ( _pfd[0].revents & POLLERR ) {
      return 0; // return 0 only for binded socket
    } else {    // nothing found, may be only closed sockets
      _pfd[0].revents = 1; // we return to poll again
    }
  } while ( FD_ISSET( fd(), &_pfd ) );
  container_type::iterator i = _M_c.begin();
  while ( i != _M_c.end() ) {
    if ( (*i)->s.is_open() && FD_ISSET( (*i)->s.rdbuf()->fd(), &_pfd ) ) {
      return *i;
    }
    ++i;
  }

  return 0;

}
#endif //  WIN32

#ifdef __unix // based on poll call
template <class Connect>
sockmgr_client_MP<Connect> *sockmgr_stream_MP<Connect>::accept_tcp()
{
  if ( !is_open() ) {
    return 0;
  }

  _xsockaddr addr;
  size_t sz = sizeof( sockaddr_in );

  if ( _pfd == 0 ) {
    _pfd = new pollfd[1024];
    _pfd[0].fd = fd();
    _pfd[0].events = POLLIN;
    ++_fdcount;
    __STL_ASSERT( _fdcount == 1 );
  }

  sockmgr_client_MP<Connect> *cl;

  do {

    // find sockets that has buffered data
    typename container_type::iterator ba = 
      find_if( _M_c.begin(), _M_c.end(), _M_av );
    if ( ba != _M_c.end() ) {
      return *ba;
    }

    _pfd[0].revents = 0;
    if ( poll( _pfd, _fdcount, -1 ) < 0 ) { // wait infinite
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }

    if ( _pfd[0].revents != 0 && (_pfd[0].revents & POLLERR) == 0 ) {
      // poll found event on binded socket
      sock_base::socket_type _sd = ::accept( fd(), &addr.any, &sz );
      if ( _sd == -1 ) {
        // check and set errno
        __STL_ASSERT( _sd == -1 );
        return 0;
      }

      try {
        sockmgr_client_MP<Connect> *cl_new;
        typename container_type::iterator i = 
          find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, -1 ) );
    
        if ( i == _M_c.end() ) { // we need new message processor
          cl_new = new sockmgr_client_MP<Connect>();
          _M_c.push_back( cl_new );
        } else { // we can reuse old
// #ifndef __hpux
          cl_new = *i;
// #else
//          _M_c.erase( i );
//          cl_new = new sockmgr_client_MP<Connect>();
//          _M_c.push_back( cl_new );
// #endif
        }

        cl_new->s.open( _sd, addr.any );

        int j = _fdcount++;

        _pfd[j].fd = _sd;
        _pfd[j].events = POLLIN;
        _pfd[j].revents = 0;
      }
      catch ( ... ) {
      }
    }

    cl = _shift_fd(); // find polled and return it
    if ( cl != 0 ) {
      return cl; // return message processor
    } else if ( _pfd[0].revents & POLLERR ) {
      return 0; // return 0 only for binded socket
    } else {    // nothing found, may be only closed sockets
      _pfd[0].revents = 1; // we return to poll again
    }
  } while ( _pfd[0].revents != 0 );

  return 0; // Unexpected; should never occur
}
#endif // __unix

template <class Connect>
sockmgr_client_MP<Connect> *sockmgr_stream_MP<Connect>::accept_udp()
{
  if ( !is_open() ) {
    return 0;
  }

  size_t sz = sizeof( sockaddr_in );
  _xsockaddr addr;

#ifdef WIN32
  if ( _fdcount == 0 ) {
    FD_ZERO( &_pfd );
    FD_SET( fd(), &_pfd );
    ++_fdcount;
  }

  if ( select( fd() + 1, &_pfd, 0, 0, 0 ) < 0 ) {
    return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
  }
#else
  if ( _pfd == 0 ) {
    _pfd = new pollfd[1];
    _pfd[0].fd = fd();
    _pfd[0].events = POLLIN;
    ++_fdcount;
  }

  if ( poll( _pfd, 1, -1 ) < 0 ) { // wait infinite
    return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
  }
#endif
  // get address of caller only
  char buff[32];    
  ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr.any, &sz );
  sockmgr_client_MP<Connect> *cl;
  try {
    _c_lock._M_acquire_lock();
    typename container_type::iterator i = _M_c.begin();
    sockbuf *b;
    while ( i != _M_c.end() ) {
      b = (*i)->s.rdbuf();
      if ( (*i)->s.is_open() && b->stype() == sock_base::sock_dgram &&
           b->port() == addr.inet.sin_port &&
           b->inet_addr() == addr.inet.sin_addr.s_addr ) {
        _c_lock._M_release_lock();
        return *i;
      }
      ++i;
    }

    cl = new sockmgr_client_MP<Connect>();
    _M_c.push_back( cl );
#ifdef __unix
    cl->s.open( dup( fd() ), addr.any, sock_base::sock_dgram );
#endif
#ifdef WIN32
    SOCKET dup_fd;
    HANDLE proc = GetCurrentProcess();
    DuplicateHandle( proc, (HANDLE)fd(), proc, (HANDLE *)&dup_fd, 0, FALSE, DUPLICATE_SAME_ACCESS );
    cl->s.open( dup_fd, addr.any, sock_base::sock_dgram );
#endif
    _c_lock._M_release_lock();
  }
  catch ( ... ) {
    _c_lock._M_release_lock();
    cl = 0;
  }
  return cl;
}

template <class Connect>
int sockmgr_stream_MP<Connect>::loop( void *p )
{
  sockmgr_stream_MP *me = static_cast<sockmgr_stream_MP *>(p);

#ifdef __unix
  Thread::unblock_signal( SIGINT );
#endif

  try {
    sockmgr_client_MP<Connect> *s;
#if 1
    unsigned _sfd;
#endif

    while ( (s = me->accept()) != 0 ) {    
      // The user connect function: application processing
#if 1
      if ( s->s.is_open() ) {
        _sfd = s->s.rdbuf()->fd();
        s->_proc.connect( s->s );
        if ( !s->s.good() ) {
          s->s.close();
        }
        if ( !s->s.is_open() ) { // remove all closed sockets from poll
#ifdef __unix
          for ( int i = 1; i < me->_fdcount; ++i ) {
            if ( me->_pfd[i].fd == _sfd ) {
              --me->_fdcount;
#if !defined( _RWSTD_VER ) // bogus RogueWave
              memmove( &me->_pfd[i], &me->_pfd[i+1], sizeof(struct pollfd) * (me->_fdcount - i) );
#else // _RWSTD_VER
              memmove( &me->_pfd[i], &me->_pfd[i+1], sizeof(&me->_pfd[0]) * (me->_fdcount - i) );
#endif
            }
          }
#endif // __unix
#ifdef WIN32
          FD_CLR( _sfd, &me->_pfd );
#endif
          // me->_c_lock._M_release_lock();
        }
      }
#else // 0
      s->_proc.connect( s->s );
#endif // 0
    }
  }
  catch ( ... ) {
    me->_c_lock._M_acquire_lock();

    typename container_type::iterator i = me->_M_c.begin();
    while ( i != me->_M_c.end() ) {
      (*i++)->s.close();
    }
    me->close();
    me->_c_lock._M_release_lock();
    throw;
  }

  cerr << "Out of loop!" << endl;
  me->_c_lock._M_acquire_lock();

  typename container_type::iterator i = me->_M_c.begin();
  while ( i != me->_M_c.end() ) {
    (*i++)->s.close();
  }
  me->close();
  me->_c_lock._M_release_lock();

  return 0;
}

__STL_END_NAMESPACE
