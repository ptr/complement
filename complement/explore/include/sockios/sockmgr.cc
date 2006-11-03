// -*- C++ -*- Time-stamp: <06/10/11 15:30:02 ptr>

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
        int pipefd[2];
        pipe( pipefd ); // check err
        _cfd = pipefd[1];

        _pfd.resize( 2 );
        _pfd[0].fd = fd_unsafe();
        _pfd[0].events = POLLIN;
        _pfd[1].fd = pipefd[0];
        _pfd[1].events = POLLIN;        
      }
    } else if ( t == sock_base::sock_dgram ) {
      _accept = &_Self_type::accept_udp;
      if ( _pfd.size() == 0 ) {
        int pipefd[2];
        pipe( pipefd ); // check err
        _cfd = pipefd[1];

        _pfd.resize( 2 );
        _pfd[0].fd = fd_unsafe();
        _pfd[0].events = POLLIN;
        _pfd[1].fd = pipefd[0];
        _pfd[1].events = POLLIN;        
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
bool sockmgr_stream_MP<Connect>::_shift_fd()
{
  bool ret = false;
  typename iterator_traits<typename _fd_sequence::iterator>::difference_type d;

  for ( _fd_sequence::iterator j = _pfd.begin() + 2; j != _pfd.end(); ++j ) { // _pfd at least 2 in size
    if ( j->revents != 0 ) {
      // We should distinguish closed socket from income message
      typename container_type::iterator i = 
        find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, j->fd ) );
      // Solaris return ERROR on poll, before close socket
      if ( i == _M_c.end() ) {
        // Socket already closed (may be after read/write failure)
        // this way may not notify poll (like in HP-UX 11.00) via POLLERR flag
        // as made in Solaris
        d = j - _pfd.begin();
        _pfd.erase( j );
        j = _pfd.begin() + (d - 1);
        for ( i = _M_c.begin(); i != _M_c.end(); ++i ) {
          if ( (*i)->s->rdbuf()->fd() == -1 ) {
            (*i)->s->close();
            (*i)->_proc->close();
          }
        }
      } else if ( j->revents & POLLERR /* | POLLHUP | POLLNVAL */ ) {
        // poll first see closed socket
        d = j - _pfd.begin();
        _pfd.erase( j );
        j = _pfd.begin() + (d - 1);
        (*i)->s->close();
        (*i)->_proc->close();
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
          d = j - _pfd.begin();
          _pfd.erase( j );
          j = _pfd.begin() + (d - 1);
          (*i)->s->close();
          (*i)->_proc->close();
        } else { // normal data available for reading
          _dlock.lock();
          _conn_pool.push_back( *i );
          // xmt::Thread::gettime( &_tpush );
          _pool_cnd.set( true );
          _observer_cnd.set( true );
          _dlock.unlock();

          /* erase: I don't want to listen this socket
           * (it may be polled elsewhere, during connection
           * processing).
           * This help to avoid unwanted processing of the same socket
           * in different threads: the socket in process can't
           * come here before it will be re-added after processing
           */
          d = j - _pfd.begin();
          _pfd.erase( j );
          j = _pfd.begin() + (d - 1);

          ret = true; // mark that I add somthing in connection queue
        }
      }
    }
  }

  return ret;
}

template <class Connect>
bool sockmgr_stream_MP<Connect>::accept_tcp()
{
  if ( !is_open() ) {
    return false;
  }

  _xsockaddr addr;
  socklen_t sz = sizeof( sockaddr_in );
  bool _in_buf;

  do {
    _in_buf = false;
    _pfd[0].revents = 0;
    _pfd[1].revents = 0;
    while ( poll( &_pfd[0], _pfd.size(), -1 ) < 0 ) { // wait infinite
      if ( errno == EINTR ) { // may be interrupted, check and ignore
        errno = 0;
        continue;
      }
      return false; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }

    // New connction open before data read from opened sockets.
    // This policy may be worth to revise.

    if ( (_pfd[0].revents & POLLERR) != 0 || (_pfd[1].revents & POLLERR) != 0 ) {
      return false; // return 0 only for binded socket, or control socket
    }

    if ( _pfd[0].revents != 0 ) {
      MT_REENTRANT( _fd_lck, _1 );
      if ( !is_open_unsafe() ) { // may be already closed
        return false;
      }
      // poll found event on binded socket
      sock_base::socket_type _sd = ::accept( fd_unsafe(), &addr.any, &sz );
      if ( _sd == -1 ) {
        // check and set errno
        // _STLP_ASSERT( _sd == -1 );
        return false;
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
          if ( cl_new->s->rdbuf()->in_avail() > 0 ) {
            // this is the case when user read from sockstream
            // in ctor above; push processing of this stream
            MT_REENTRANT( _dlock, _1 );
            _conn_pool.push_back( cl_new );
            _pool_cnd.set( true );
            _observer_cnd.set( true );
            _in_buf = true;
          }
        } else { // we can reuse old
          cl_new = *i;
          cl_new->s->open( _sd, addr.any );
          delete cl_new->_proc; // may be new ( cl_new->_proc ) Connect( *cl_new->s );
          cl_new->_proc = new Connect( *cl_new->s );
          if ( cl_new->s->rdbuf()->in_avail() > 0 ) {
            // this is the case when user read from sockstream
            // in ctor above; push processing of this stream
            MT_REENTRANT( _dlock, _1 );
            _conn_pool.push_back( cl_new );
            _pool_cnd.set( true );
            _observer_cnd.set( true );
            _in_buf = true;
          }
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
    if ( _pfd[1].revents != 0 ) { // fd come back for poll
      pollfd rfd;
      ::read( _pfd[1].fd, reinterpret_cast<char *>(&rfd.fd), sizeof(sock_base::socket_type) );
      rfd.events = POLLIN;
      rfd.revents = 0;

      _pfd.push_back( rfd );
    }
  } while ( !_shift_fd() && !_in_buf );
  
  return true; // something was pushed in connection queue (by _shift_fd)
}

template <class Connect>
bool sockmgr_stream_MP<Connect>::accept_udp()
{
  if ( !is_open() ) {
    return false;
  }

  socklen_t sz = sizeof( sockaddr_in );
  _xsockaddr addr;

  if ( poll( &_pfd[0], 1, -1 ) < 0 ) { // wait infinite
    return false; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
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
        return true /* *i */;
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
  return true /* cl */;
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
    me->_loop_cnd.set( true );

    me->_follow = true;
    me->_observer_run = false;

    while ( (me->*me->_accept)() ) {
      if ( me->mgr.size() < 2 ) {
        me->mgr.launch( connect_processor, me );
      }
      me->_orlock.lock();
      if ( !me->_observer_run ) {
        me->_observer_run = true;
        me->_orlock.unlock();
        me->mgr.launch( observer, me, 0, 0, PTHREAD_STACK_MIN * 2 );
      } else {
        me->_orlock.unlock();
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
    ::close( me->_cfd );
    ::close( me->_pfd[1].fd );
    me->close();
    me->_c_lock.unlock();
    rtc.iword = -1;

    me->_dlock.lock();
    me->_follow = false;
    me->_pool_cnd.set( true, true );
    me->_observer_cnd.set( true );
    me->_dlock.unlock();

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
  ::close( me->_cfd );
  ::close( me->_pfd[1].fd );
  me->close();
  me->_c_lock.unlock();

  me->_dlock.lock();
  me->_follow = false;
  me->_pool_cnd.set( true, true );
  me->_observer_cnd.set( true );
  me->_dlock.unlock();

  return rtc;
}

template <class Connect>
xmt::Thread::ret_code sockmgr_stream_MP<Connect>::connect_processor( void *p )
{
  _Self_type *me = static_cast<_Self_type *>(p);
  xmt::Thread::ret_code rtc;
  rtc.iword = 0;

  try {
    sockstream *stream;
    timespec idle( me->_idle );
    int idle_count = 0;

    me->_dlock.lock();
    _Connect *c = 0;
    if ( me->_conn_pool.size() != 0 ) {
      c = me->_conn_pool.front();
      me->_conn_pool.pop_front();
      xmt::Thread::gettime( &me->_tpop );
    }
    me->_dlock.unlock();

    do {
      if ( c != 0 ) {
        stream = c->s;
        if ( stream->is_open() ) {
          c->_proc->connect( *stream );
          if ( !stream->good() ) {
            stream->close();
            c->_proc->close();
          } else if ( stream->is_open() ) {
            if ( stream->rdbuf()->in_avail() > 0 ) {
              // socket has buffered data, push it back to queue
              MT_REENTRANT( me->_dlock, _1 );
              me->_conn_pool.push_back( c );
              me->_observer_cnd.set( true );
              me->_pool_cnd.set( true );
              // xmt::Thread::gettime( &me->_tpush );
            } else { // no buffered data, return socket to poll
              sock_base::socket_type rfd = stream->rdbuf()->fd();
              ::write( me->_cfd, reinterpret_cast<const char *>(&rfd), sizeof(sock_base::socket_type) );
            }
          } else {
            stream->close();
            c->_proc->close();
          }
        }
      }

      for ( idle_count = 0; idle_count < 2; ++idle_count ) {
        { 
          MT_REENTRANT( me->_dlock, _1 );
          if ( !me->_follow ) {
            break;
          }
          if ( me->_conn_pool.size() != 0 ) {
            c = me->_conn_pool.front();
            me->_conn_pool.pop_front();
            xmt::Thread::gettime( &me->_tpop );
            break;
          }
          me->_pool_cnd.set( false );
          me->_observer_cnd.set( false );
        }
        if ( me->_pool_cnd.try_wait_delay( &idle ) != 0 ) {
          idle_count = 2;
        }
      }
    } while ( me->_is_follow() && idle_count < 2 );
  }
  catch ( ... ) {
  }
  return rtc;
}

template <class Connect>
xmt::Thread::ret_code sockmgr_stream_MP<Connect>::observer( void *p )
{
  _Self_type *me = static_cast<_Self_type *>(p);
  xmt::Thread::ret_code rtc;
  rtc.iword = 0;
  int pool_size[3];
  // size_t thr_pool_size[3];
  timespec tpop;
  timespec delta( me->_busylimit );
  timespec alarm( me->_alarm );

  timespec idle( me->_idle );

  timespec now;
  std::fill( pool_size, pool_size + 3, 0 );

  try {
    do {
      // std::swap( pool_size[0], pool_size[1] );
      std::rotate( pool_size, pool_size, pool_size + 3 );
      {
        MT_REENTRANT( me->_dlock, _1 );
        pool_size[2] = static_cast<int>(me->_conn_pool.size());
        tpop = me->_tpop;
      }
      if ( pool_size[2] != 0 ) {
        if ( me->_thr_limit > me->mgr.size() ) {
          if ( (pool_size[0] - 2 * pool_size[1] + pool_size[2]) > 0 ||
               pool_size[2] > 32
               /* pool_size[1] > 3 && pool_size[0] <= pool_size[1] */ ) {
            // queue not empty and not decrease
            me->mgr.launch( connect_processor, me /* , 0, 0, PTHREAD_STACK_MIN * 2 */ );
          } else {
            xmt::Thread::gettime( &now );
            if ( (tpop + delta) < now ) {
              // a long time was since last pop from queue
              me->mgr.launch( connect_processor, me /* , 0, 0, PTHREAD_STACK_MIN * 2 */ );
            }
          }
        }
        xmt::Thread::delay( &alarm );
      } else {
        if ( me->_observer_cnd.try_wait_delay( &idle ) != 0 ) {
          MT_REENTRANT( me->_orlock, _1 );
          me->_observer_run = false;

          return rtc;
        }
      }
    } while ( me->_is_follow() );
  }
  catch ( ... ) {
  }

  MT_REENTRANT( me->_orlock, _1 );
  me->_observer_run = false;

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