// -*- C++ -*- Time-stamp: <06/09/20 11:59:41 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <algorithm>
#include <functional>

#ifdef __unix
extern "C" int nanosleep(const struct timespec *, struct timespec *);
#endif

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

#ifndef __FIT_NO_POLL

template <class Connect>
void sockmgr_stream_MP<Connect>::_open( sock_base::stype t )
{
  MT_REENTRANT( _fd_lck, _1 );
  if ( is_open_unsafe() ) {
    if ( t == sock_base::sock_stream ) {
      _accept = &_Self_type::accept_tcp;
      _pfd.reserve( 32 );
      if ( _pfd.size() == 0 ) {
        _pfd.resize( 1 );
        _pfd[0].fd = fd_unsafe();
        _pfd[0].events = POLLIN;
      }
    } else if ( t == sock_base::sock_dgram ) {
      _accept = &_Self_type::accept_udp;
      if ( _pfd.size() == 0 ) {
        _pfd.resize( 1 );
        _pfd[0].fd = fd_unsafe();
        _pfd[0].events = POLLIN;
      }
    } else {
      throw invalid_argument( "sockmgr_stream_MP" );
    }
    
    _loop_cnd.set( false );
    loop_id.launch( loop, this, 0, PTHREAD_STACK_MIN * 2 );
    _loop_cnd.try_wait();
  }
}

template <class Connect>
void sockmgr_stream_MP<Connect>::open( const in_addr& addr, int port, sock_base::stype t )
{
  basic_sockmgr::open( addr, port, t, sock_base::inet );
  sockmgr_stream_MP<Connect>::_open( t );
}

template <class Connect>
void sockmgr_stream_MP<Connect>::open( unsigned long addr, int port, sock_base::stype t )
{
  basic_sockmgr::open( addr, port, t, sock_base::inet );
  sockmgr_stream_MP<Connect>::_open( t );
}

template <class Connect>
void sockmgr_stream_MP<Connect>::open( int port, sock_base::stype t )
{
  basic_sockmgr::open( port, t, sock_base::inet );
  sockmgr_stream_MP<Connect>::_open( t );
}

template <class Connect>
__FIT_TYPENAME sockmgr_stream_MP<Connect>::_Connect *sockmgr_stream_MP<Connect>::_shift_fd()
{
  _Connect *msg = 0;
  _fd_sequence::iterator j = _pfd.begin();
  _fd_sequence::iterator k = j++;

  for ( ; j != _pfd.end(); ++j ) {
    if ( j->revents != 0 ) {
      // We should distinguish closed socket from income message
      typename container_type::iterator i = 
        find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, j->fd ) );
      // Solaris return ERROR on poll, before close socket
      if ( i == _M_c.end() ) {
        // Socket already closed (may be after read/write failure)
        // this way may not notify poll (like in HP-UX 11.00) via POLLERR flag
        // as made in Solaris
        _pfd.erase( j-- );
        for ( i = _M_c.begin(); i != _M_c.end(); ++i ) {
          if ( (*i)->s->rdbuf()->fd() == -1 ) {
            (*i)->s->close();
            (*i)->_proc->close();
          }
        }
        continue;
      } else if ( j->revents & POLLERR /* | POLLHUP | POLLNVAL */ ) {
        // poll first see closed socket
        _pfd.erase( j-- );
        (*i)->s->close();
        (*i)->_proc->close();
        continue;
      } else {
        // Check that other side close socket:
        // on Linux and (?) Solaris I see normal POLLIN event, and see error
        // only after attempt to read something.
        // Due to this fd isn't stream (it's upper than stream),
        // I can't use ioctl with I_PEEK command here.
        char x;
        int nr = recv( j->fd, reinterpret_cast<void *>(&x), 1, MSG_PEEK );
        if ( nr <= 0 ) { // I can't read even one byte: this designate closed
                         // socket operation
          _pfd.erase( j-- );
          (*i)->s->close();
          (*i)->_proc->close();
          continue;
        }
      }
      if ( msg == 0 ) {
        j->revents = 0;
        msg = *i;
        k = j;
      }
    }
  }

  if ( _pfd.size() > 2 ) {
    j = _pfd.end();
    --j;
    if ( k != _pfd.begin() && k != j ) {
      // random_shuffle( ++_pfd.begin(), _pfd.end() );
      std::swap( *k, *j );
    }
  }

  return msg;
}

template <class Connect>
__FIT_TYPENAME sockmgr_stream_MP<Connect>::_Connect *sockmgr_stream_MP<Connect>::accept_tcp()
{
  if ( !is_open() ) {
    return 0;
  }

  _xsockaddr addr;
  socklen_t sz = sizeof( sockaddr_in );
  _Connect *cl;

  do {
    // find sockets that has buffered data
    // Note, that available data will processed first
    // with preference over connection establishes and 
    // new data;
    // This policy may be worth to revise.
    typename container_type::iterator ba = 
      find_if( _M_c.begin(), _M_c.end(), _M_av );
    if ( ba != _M_c.end() ) {
      return *ba;
    }

    _pfd[0].revents = 0;
    while ( poll( &_pfd[0], _pfd.size(), -1 ) < 0 ) { // wait infinite
      if ( errno == EINTR ) { // may be interrupted, check and ignore
        errno = 0;
        continue;
      }
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }

    // New connction open before data read from opened sockets.
    // This policy may be worth to revise.

    if ( (_pfd[0].revents & POLLERR) != 0 ) {
      return 0; // return 0 only for binded socket
    }

    if ( _pfd[0].revents != 0 ) {
      MT_REENTRANT( _fd_lck, _1 );
      if ( !is_open_unsafe() ) { // may be already closed
        return 0;
      }
      // poll found event on binded socket
      sock_base::socket_type _sd = ::accept( fd_unsafe(), &addr.any, &sz );
      if ( _sd == -1 ) {
        // check and set errno
        // _STLP_ASSERT( _sd == -1 );
        return 0;
      }

      try {
        _Connect *cl_new;
        typename container_type::iterator i = 
          find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, -1 ) );
    
        if ( i == _M_c.end() ) { // we need new message processor
          cl_new = new _Connect();
          cl_new->s = new sockstream();
          cl_new->s->open( _sd, addr.any );
          cl_new->_proc = new Connect( *cl_new->s );
          _M_c.push_back( cl_new );
        } else { // we can reuse old
          cl_new = *i;
          cl_new->s->open( _sd, addr.any );
          delete cl_new->_proc; // may be new ( cl_new->_proc ) Connect( *cl_new->s );
          cl_new->_proc = new Connect( *cl_new->s );
        }

        pollfd newfd;

        newfd.fd = _sd;
        newfd.events = POLLIN;
        newfd.revents = 0;

        _pfd.push_back( newfd );
      }
      catch ( ... ) {
      }
    }

    cl = _shift_fd(); // find polled and return it
    if ( cl != 0 ) {
      return cl; // return message processor
    } else { // nothing found, may be only closed sockets
      _pfd[0].revents = 1; // we return to poll again
    }
  } while ( _pfd[0].revents != 0 );
  
  return 0; // Unexpected; should never occur
}

template <class Connect>
__FIT_TYPENAME sockmgr_stream_MP<Connect>::_Connect *sockmgr_stream_MP<Connect>::accept_udp()
{
  if ( !is_open() ) {
    return 0;
  }

  socklen_t sz = sizeof( sockaddr_in );
  _xsockaddr addr;

  if ( poll( &_pfd[0], 1, -1 ) < 0 ) { // wait infinite
    return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
  }
  // get address of caller only
  char buff[32];    
  ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr.any, &sz );
  _Connect *cl;
  try {
    _c_lock.lock();
    typename container_type::iterator i = _M_c.begin();
    sockbuf *b;
    while ( i != _M_c.end() ) {
      b = (*i)->s->rdbuf();
      if ( (*i)->s->is_open() && b->stype() == sock_base::sock_dgram &&
           b->port() == addr.inet.sin_port &&
           b->inet_addr() == addr.inet.sin_addr.s_addr ) {
        _c_lock.unlock();
        return *i;
      }
      ++i;
    }

    cl = new _Connect();
    cl->s = new sockstream();
    _M_c.push_back( cl );
    cl->s->open( dup( fd() ), addr.any, sock_base::sock_dgram );
    cl->_proc = new Connect( *cl->s );
    _c_lock.unlock();
  }
  catch ( ... ) {
    _c_lock.unlock();
    cl = 0;
  }
  return cl;
}

template <class Connect>
void sockmgr_stream_MP<Connect>::_close_by_signal( int )
{
#ifdef _PTHREADS
  void *_uw_save = *((void **)pthread_getspecific( xmt::Thread::mtkey() ) + _idx );
  _Self_type *me = static_cast<_Self_type *>( _uw_save );

  me->close();
#else
#error "Fix me!"
#endif
}

template <class Connect>
xmt::Thread::ret_code sockmgr_stream_MP<Connect>::loop( void *p )
{
  _Self_type *me = static_cast<_Self_type *>(p);
  me->loop_id.pword( _idx ) = me; // push pointer to self for signal processing
  xmt::Thread::ret_code rtc;
  rtc.iword = 0;

  try {
    _Connect *s;
    unsigned _sfd;

    std::deque<_Connect *> conn_pool;
    xmt::Mutex dlock;

    _ProcState _state( conn_pool, dlock );
    xmt::Thread *thr = 0;

    me->_loop_cnd.set( true );

    while ( (s = me->accept()) != 0 ) {
      _state.s = s;
      _state.follow = true;
#if 0
      dlock.lock();
      conn_pool.push_back( s );
      // remove 
      _sfd = s->s->rdbuf()->fd();
      { // erase
        _fd_sequence::iterator i = me->_pfd.begin();
        ++i;
        for ( ; i != me->_pfd.end(); ++i ) {
          if ( i->fd == _sfd ) {
            me->_pfd.erase( i );
            break;
          }
        }
      }
      dlock.unlock();

      if ( thr == 0 ) {
        thr = new Thread();
      }
      
#else
      // The user connect function: application processing
      if ( s->s->is_open() ) {
        _sfd = s->s->rdbuf()->fd();
        s->_proc->connect( *s->s );
        if ( !s->s->good() ) {
          s->s->close();
          s->_proc->close();
        }
        if ( !s->s->is_open() ) { // remove all closed sockets from poll
          _fd_sequence::iterator i = me->_pfd.begin();
          ++i;
          for ( ; i != me->_pfd.end(); ++i ) {
            if ( i->fd == _sfd ) {
              me->_pfd.erase( i );
              break;
            }
          }
        }
      }
#endif
    }
  }
  catch ( ... ) {
    me->_c_lock.lock();
    
    for ( typename container_type::iterator i = me->_M_c.begin(); i != me->_M_c.end(); ++i ) {
      if ( (*i)->s->is_open() ) { // close all not closed yet
        (*i)->s->close();
        (*i)->_proc->close();
      }
    }
    me->close();
    me->_c_lock.unlock();
    rtc.iword = -1;
    return rtc;
    // throw;
  }

  me->_c_lock.lock();
  
  for ( typename container_type::iterator i = me->_M_c.begin(); i != me->_M_c.end(); ++i ) {
    if ( (*i)->s->is_open() ) { // close all not closed yet
      (*i)->s->close();
      (*i)->_proc->close();
    }
  }
  me->close();
  me->_c_lock.unlock();

  return rtc;
}

template <class Connect>
xmt::Thread::ret_code sockmgr_stream_MP<Connect>::connect_processor( void *p )
{
  _ProcState *s = static_cast<_ProcState *>(p);
  xmt::Thread::ret_code rtc;
  rtc.iword = 0;

  try {
    sockstream *stream;
    _Connect *c;

    while ( s->follow ) {
      s->dlock.lock();
      c = s->conn_pool.front();
      s->conn_pool.pop_front();
      s->dlock.unlock();

      s->cnd.set( false );

      stream = c->s;
      if ( stream->is_open() ) {
        c->_proc->connect( *stream );
        if ( !stream->good() ) {
          stream->close();
          c->_proc->close();
        } else if ( stream->is_open() ) {
          s->dlock.lock();
          // _pfd.push_back( stream->rdbuf()->fd() );
          s->dlock.unlock();
        }
      }

      s->cnd.try_wait();
    }
  }
  catch ( ... ) {
  }
  return rtc;
}

#endif // !__FIT_NO_POLL

#ifndef __FIT_NO_SELECT

template <class Connect>
void sockmgr_stream_MP_SELECT<Connect>::_open( sock_base::stype t )
{
  MT_REENTRANT( _fd_lck, _1 );
  if ( is_open_unsafe() ) {
    if ( t == sock_base::sock_stream ) {
      _accept = &_Self_type::accept_tcp;
    } else if ( t == sock_base::sock_dgram ) {
      _accept = &_Self_type::accept_udp;
    } else {
      throw invalid_argument( "sockmgr_stream_MP" );
    }
    
    FD_ZERO( &_pfdr );
    FD_ZERO( &_pfde );
    FD_SET( fd_unsafe(), &_pfdr );
    FD_SET( fd_unsafe(), &_pfde );
    _fdmax = fd_unsafe();
    
    loop_id.launch( loop, this, 0, PTHREAD_STACK_MIN * 2 );
  }
}

template <class Connect>
void sockmgr_stream_MP_SELECT<Connect>::open( const in_addr& addr, int port, sock_base::stype t )
{
  basic_sockmgr::open( addr, port, t, sock_base::inet );
  sockmgr_stream_MP_SELECT<Connect>::_open( t );
}

template <class Connect>
void sockmgr_stream_MP_SELECT<Connect>::open( unsigned long addr, int port, sock_base::stype t )
{
  basic_sockmgr::open( addr, port, t, sock_base::inet );
  sockmgr_stream_MP_SELECT<Connect>::_open( t );
}

template <class Connect>
void sockmgr_stream_MP_SELECT<Connect>::open( int port, sock_base::stype t )
{
  basic_sockmgr::open( port, t, sock_base::inet );
  sockmgr_stream_MP_SELECT<Connect>::_open( t );
}

template <class Connect>
__FIT_TYPENAME sockmgr_stream_MP_SELECT<Connect>::_Connect *sockmgr_stream_MP_SELECT<Connect>::_shift_fd()
{
  _Connect *msg = 0;
  for ( unsigned j = 0; j <= _fdmax; ++j ) {
    if ( FD_ISSET( j, &_pfde ) || FD_ISSET( j, &_pfdr ) ) {
      // We should distinguish closed socket from income message
      typename container_type::iterator i = 
        find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, j ) );
      // Solaris return ERROR on poll, before close socket
      if ( i == _M_c.end() ) {
        // Socket already closed (may be after read/write failure)
        // this way may not notify poll (like in HP-UX 11.00) via POLLERR flag
        // as made in Solaris
        // decrement of _fdmax may be here // --_fdcount;
        for ( i = _M_c.begin(); i != _M_c.end(); ++i ) {
          if ( (*i)->s->rdbuf()->fd() == -1 ) {
            (*i)->s->close();
            (*i)->_proc->close();
          }
        }
        continue;
      } else if ( FD_ISSET( j, &_pfde ) ) {
        // poll first see closed socket
        // decrement of _fdmax may be here // --_fdcount;
        (*i)->s->close();
        (*i)->_proc->close();
        continue;
      } else {
        // Check that other side close socket:
        // on Linux and (?) Solaris I see normal POLLIN event, and see error
        // only after attempt to read something.
        // Due to this fd isn't stream (it's upper than stream),
        // I can't use ioctl with I_PEEK command here.
        char x;
        int nr = recv( j, reinterpret_cast<void *>(&x), 1, MSG_PEEK );
        if ( nr <= 0 ) { // I can't read even one byte: this designate closed
                         // socket operation
          // decrement of _fdmax may be here // --_fdcount;
          (*i)->s->close();
          (*i)->_proc->close();
          continue;
        }
      }
      if ( msg == 0 ) {
        FD_CLR( j, &_pfdr );
        FD_CLR( j, &_pfde );
        msg = *i;
      }
    }
  }

  return msg;
}

template <class Connect>
__FIT_TYPENAME sockmgr_stream_MP_SELECT<Connect>::_Connect *sockmgr_stream_MP_SELECT<Connect>::accept_tcp()
{
  if ( !is_open() ) {
    return 0;
  }

  _xsockaddr addr;
  size_t sz = sizeof( sockaddr_in );

  _Connect *cl;
  bool more = true;

  do {
    FD_ZERO( &_pfdr );
    FD_ZERO( &_pfde );

    // *** Set all listen sockets here...
    MT_LOCK( _fd_lck );
    FD_SET( fd_unsafe(), &_pfdr );
    FD_SET( fd_unsafe(), &_pfde );
    _fdmax = fd_unsafe();
    MT_UNLOCK( _fd_lck );
    for ( typename container_type::iterator i = _M_c.begin(); i != _M_c.end(); ++i ) {
      if ( (*i)->s->is_open() ) {
        FD_SET( (*i)->s->rdbuf()->fd(), &_pfdr );
        FD_SET( (*i)->s->rdbuf()->fd(), &_pfde );
        _fdmax = max( (*i)->s->rdbuf()->fd(), _fdmax );
      }
    }

    // select wait infinite here, so it can't return 0 (timeout), so it return -1.
    while ( select( _fdmax + 1, &_pfdr, 0, &_pfde, 0 ) < 0 ) { // wait infinite
      if ( errno == EINTR ) { // may be interrupted, check and ignore
        errno = 0;
        continue;
      }
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }

    MT_REENTRANT( _fd_lck, _1 );
    if ( !is_open_unsafe() || FD_ISSET( fd_unsafe(), &_pfde ) ) { // may be already closed
      return 0;
    }

    if ( FD_ISSET( fd_unsafe(), &_pfdr ) ) { // select found event on binded socket
      sock_base::socket_type _sd = ::accept( fd_unsafe(), &addr.any, &sz );
      if ( _sd == -1 ) {
        // check and set errno
        // _STLP_ASSERT( _sd == -1 );
        return 0;
      }

      try {
        // sockmgr_client_MP<Connect> *cl_new;
        _Connect *cl_new;
        typename container_type::iterator i = 
          find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, -1 ) );

        if ( i == _M_c.end() ) { // we need new message processor
          cl_new = new _Connect();
          cl_new->s = new sockstream();
          cl_new->s->open( _sd, addr.any );
          cl_new->_proc = new Connect( *cl_new->s );
          _M_c.push_back( cl_new );
        } else { // we can reuse old
          cl_new = *i;
          cl_new->s->open( _sd, addr.any );
          delete cl_new->_proc; // may be new ( cl_new->_proc ) Connect( *cl_new->s );
          cl_new->_proc = new Connect( *cl_new->s );
        }

        FD_SET( _sd, &_pfdr );
        FD_SET( _sd, &_pfde );
      }
      catch ( ... ) {
      }
    }

    cl = _shift_fd(); // find polled and return it
    if ( cl != 0 ) {
      return cl; // return message processor
    } else {    // nothing found, may be only closed sockets
      FD_SET( fd_unsafe(), &_pfdr ); // we return to poll again
    }
    more = is_open_unsafe() && FD_ISSET( fd_unsafe(), &_pfdr );
  } while ( more );

  return 0; // Unexpected; should never occur

}

template <class Connect>
__FIT_TYPENAME sockmgr_stream_MP_SELECT<Connect>::_Connect *sockmgr_stream_MP_SELECT<Connect>::accept_udp()
{
  if ( !is_open() ) {
    return 0;
  }

  size_t sz = sizeof( sockaddr_in );
  _xsockaddr addr;

  if ( select( fd() + 1, &_pfdr, 0, &_pfde, 0 ) < 0 ) {
    return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
  }
  // get address of caller only
  char buff[32];    
  ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr.any, &sz );
  _Connect *cl;
  try {
    _c_lock.lock();
    typename container_type::iterator i = _M_c.begin();
    sockbuf *b;
    while ( i != _M_c.end() ) {
      b = (*i)->s->rdbuf();
      if ( (*i)->s->is_open() && b->stype() == sock_base::sock_dgram &&
           b->port() == addr.inet.sin_port &&
           b->inet_addr() == addr.inet.sin_addr.s_addr ) {
        _c_lock.unlock();
        return *i;
      }
      ++i;
    }

    cl = new _Connect();
    cl->s = new sockstream();
    _M_c.push_back( cl );
#ifdef _WIN32
    SOCKET dup_fd;
    HANDLE proc = GetCurrentProcess();
    DuplicateHandle( proc, (HANDLE)fd(), proc, (HANDLE *)&dup_fd, 0, FALSE, DUPLICATE_SAME_ACCESS );
    cl->s->open( dup_fd, addr.any, sock_base::sock_dgram );
#else
    cl->s->open( dup( fd() ), addr.any, sock_base::sock_dgram ); 
#endif
    cl->_proc = new Connect( *cl->s );
    _c_lock.unlock();
  }
  catch ( ... ) {
    _c_lock.unlock();
    cl = 0;
  }
  return cl;
}

template <class Connect>
void sockmgr_stream_MP_SELECT<Connect>::_close_by_signal( int )
{
#ifdef _PTHREADS
  void *_uw_save = *((void **)pthread_getspecific( xmt::Thread::mtkey() ) + _idx );
  _Self_type *me = static_cast<_Self_type *>( _uw_save );

  me->close();
#else
// #error "Fix me!"
#ifdef __FIT_WIN32THREADS
  void *_uw_save = *((void **)TlsGetValue( xmt::Thread::mtkey() ) + _idx );
  _Self_type *me = static_cast<_Self_type *>( _uw_save );

  me->close();
#endif
#endif
}

template <class Connect>
xmt::Thread::ret_code sockmgr_stream_MP_SELECT<Connect>::loop( void *p )
{
  _Self_type *me = static_cast<_Self_type *>(p);
  me->loop_id.pword( _idx ) = me; // push pointer to self for signal processing

  xmt::Thread::ret_code rtc;
  rtc.iword = 0;

  try {
    _Connect *s;
    unsigned _sfd;

    while ( (s = me->accept()) != 0 ) {    
      // The user connect function: application processing
      if ( s->s->is_open() ) {
        _sfd = s->s->rdbuf()->fd();
        s->_proc->connect( *s->s );
        if ( !s->s->good() ) {
          s->s->close();
          s->_proc->close();
        }
      }
    }
  }
  catch ( ... ) {
    me->_c_lock.lock();
    
    for ( typename container_type::iterator i = me->_M_c.begin(); i != me->_M_c.end(); ++i ) {
      if ( (*i)->s->is_open() ) { // close all not closed yet
        (*i)->s->close();
        (*i)->_proc->close();
      }
    }
    me->close();
    me->_c_lock.unlock();
    rtc.iword = -1;
    return rtc;
    // throw;
  }

  me->_c_lock.lock();
  
  for ( typename container_type::iterator i = me->_M_c.begin(); i != me->_M_c.end(); ++i ) {
    if ( (*i)->s->is_open() ) { // close all not closed yet
      (*i)->s->close();
      (*i)->_proc->close();
    }
  }
  me->close();
  me->_c_lock.unlock();

  return rtc;
}

#endif // !__FIT_NO_SELECT

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif
