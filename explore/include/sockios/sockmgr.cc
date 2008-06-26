// -*- C++ -*- Time-stamp: <07/09/19 11:43:21 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005-2007
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

// #ifdef __unix
// extern "C" int nanosleep(const struct timespec *, struct timespec *);
// #endif

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

#ifndef __FIT_NO_POLL

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
void sockmgr_stream_MP<Connect,C,T>::_open( sock_base::stype t )
{
  xmt::scoped_lock lk(_fd_lck);
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
    loop_thr.launch( loop, this /* , 0, PTHREAD_STACK_MIN * 2 */ );
    _loop_cnd.try_wait();
  }
}

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
void sockmgr_stream_MP<Connect,C,T>::open( const in_addr& addr, int port, sock_base::stype t )
{
  basic_sockmgr::open( addr, port, t, sock_base::inet );
  sockmgr_stream_MP<Connect,C,T>::_open( t );
}

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
void sockmgr_stream_MP<Connect,C,T>::open( unsigned long addr, int port, sock_base::stype t )
{
  basic_sockmgr::open( addr, port, t, sock_base::inet );
  sockmgr_stream_MP<Connect,C,T>::_open( t );
}

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
void sockmgr_stream_MP<Connect,C,T>::open( int port, sock_base::stype t )
{
  basic_sockmgr::open( port, t, sock_base::inet );
  sockmgr_stream_MP<Connect,C,T>::_open( t );
}

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
bool sockmgr_stream_MP<Connect,C,T>::_shift_fd()
{
  bool ret = false;
  typename iterator_traits<typename _fd_sequence::iterator>::difference_type d;

  for ( _fd_sequence::iterator j = _pfd.begin() + 2; j != _pfd.end(); ++j ) { // _pfd at least 2 in size
    // if ( j->fd == -1 ) {
    //   cerr << __FILE__ << ":" << __LINE__ << endl;
    // }
    if ( j->revents != 0 ) {
      xmt::scoped_lock _l( _c_lock );
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
        i = _M_c.begin();
        while ( (i = find_if( i, _M_c.end(), bind2nd( _M_comp, -1 ) )) != _M_c.end() ) {
          _dlock.lock();
          _conn_pool.erase( std::remove( _conn_pool.begin(), _conn_pool.end(), i ), _conn_pool.end() );
          _dlock.unlock();
          _M_c.erase( i++ );
        }
      } else if ( j->revents & POLLERR /* | POLLHUP | POLLNVAL */ ) {
        // poll first see closed socket
        d = j - _pfd.begin();
        _pfd.erase( j );
        j = _pfd.begin() + (d - 1);
        _dlock.lock();
        _conn_pool.erase( std::remove( _conn_pool.begin(), _conn_pool.end(), i ), _conn_pool.end() );
        _dlock.unlock();
        _M_c.erase( i );
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
          _dlock.lock();
          _conn_pool.erase( std::remove( _conn_pool.begin(), _conn_pool.end(), i ), _conn_pool.end() );
          _dlock.unlock();
          _M_c.erase( i );
        } else { // normal data available for reading
          _dlock.lock();
          _conn_pool.push_back( i );
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

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
bool sockmgr_stream_MP<Connect,C,T>::accept_tcp()
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
      xmt::scoped_lock lk(_fd_lck);
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
        xmt::scoped_lock _l( _c_lock );
        _M_c.push_back( _Connect() );
        _M_c.back().open( _sd, addr.any );
        _Connect *cl_new = &_M_c.back();
        if ( cl_new->s.rdbuf()->in_avail() > 0 ) {
          // this is the case when user read from sockstream
          // in ctor above; push processing of this stream
          xmt::scoped_lock lk(_dlock);
          _conn_pool.push_back( --_M_c.end() );
          _pool_cnd.set( true );
          _observer_cnd.set( true );
          _in_buf = true;
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

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
bool sockmgr_stream_MP<Connect,C,T>::accept_udp()
{
  if ( !is_open() ) {
    return false;
  }

  _xsockaddr addr;
  socklen_t sz = sizeof( sockaddr_in );
  bool _in_buf;

  // Problem here:
  // if I see event on pfd[1], I should set fd_in_work = 1 and process it below in loop;
  // but if no event on pfd[1], I don't really know wether pfd[1] polling in
  // connect_processor or not; size of _conn_pool don't help here too ...

  // Hmm, but not all so bad: if I will see event on pfd[0] here, I just
  // add SAME iterator to _conn_pool, and hope that observer process it accurate...

  int pret = poll( &_pfd[1], 1, 1 ); // timeout as short as possible
  int fd_in_work = pret == 0 ? 0 : 1;
  // int fd_in_work = 0;

  do {
    _in_buf = false;
    _pfd[0].revents = 0;
    _pfd[1].revents = 0;
    while ( poll( &_pfd[0 + fd_in_work], /* _pfd.size() */ 2 - fd_in_work, -1 ) < 0 ) { // wait infinite
      if ( errno == EINTR ) { // may be interrupted, check and ignore
        errno = 0;
        continue;
      }
      return false; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }

    // New connction open before data read from opened sockets.
    // This policy may be worth to revise.

    if ( (_pfd[0].revents & POLLERR) != 0 /* || (_pfd[1].revents & POLLERR) != 0 */ ) {
      return false; // return 0 only for binded socket, or control socket
    }

    if ( _pfd[0].revents != 0 ) {
      xmt::scoped_lock lk(_fd_lck);
      if ( !is_open_unsafe() ) { // may be already closed
        return false;
      }
      try {
        xmt::scoped_lock _l( _c_lock );
        // 
        if ( _M_c.empty() ) {
          _M_c.push_back( _Connect() );
          // poll found event on binded socket
          // to fill addr.any only, for _M_c.back().open() call
          char buff[1];
          ::recvfrom( fd_unsafe(), buff, 1, MSG_PEEK, &addr.any, &sz );
          _M_c.back().open( fd_unsafe(), addr.any, sock_base::sock_dgram );
          _Connect *cl_new = &_M_c.back();
          if ( cl_new->s.rdbuf()->in_avail() > 0 ) {
            // this is the case when user read from sockstream
            // in ctor above; push processing of this stream
            xmt::scoped_lock lk(_dlock);
            _conn_pool.push_back( _M_c.begin() );
            _pool_cnd.set( true );
            _observer_cnd.set( true );
            _in_buf = true;
            fd_in_work = 1;
          }
        } else { // normal data available for reading
          xmt::scoped_lock lk(_dlock);
          _conn_pool.push_back( _M_c.begin() );
          // xmt::Thread::gettime( &_tpush );
          _pool_cnd.set( true );
          _observer_cnd.set( true );
          _in_buf = true;
          fd_in_work = 1;
        }
        // if addr.any pesent in _M_c 
        // typename container_type::iterator i =
        //  find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp_inet, addr.any ) );
        // if ( i == _M_c.end() ) {
        //   _M_c.push_back( _Connect() );
        //   _M_c.back().open( fd(), addr.any, sock_base::sock_dgram );
        //   _Connect *cl_new = &_M_c.back();
        // }
        // 
        // ...
        // 
      }
      catch ( ... ) {
      }
    }
    if ( _pfd[1].revents != 0 ) { // fd come back for poll
      // really not used (i.e. this is fd()), but we need to read it from pipe
      sock_base::socket_type _xfd;
      ::read( _pfd[1].fd, reinterpret_cast<char *>(&_xfd), sizeof(sock_base::socket_type) );
      fd_in_work = 0;
    }
  } while ( /* !_shift_fd() && */ !_in_buf );

  return true;
}

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
void sockmgr_stream_MP<Connect,C,T>::_close_by_signal( int )
{
#ifdef _PTHREADS
  void *_uw_save = *((void **)pthread_getspecific( xmt::Thread::mtkey() ) + _idx );
  _Self_type *me = static_cast<_Self_type *>( _uw_save );

  me->close();
#else
#error "Fix me!"
#endif
}

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
xmt::Thread::ret_t sockmgr_stream_MP<Connect,C,T>::loop( void *p )
{
  _Self_type *me = static_cast<_Self_type *>(p);
  me->loop_thr.pword( _idx ) = me; // push pointer to self for signal processing
  xmt::Thread::ret_t rtc = 0;
  xmt::Thread::ret_t rtc_observer = 0;

  xmt::Thread thr_observer;

  try {
    me->_loop_cnd.set( true );

    me->_follow = true;

    while ( (me->*me->_accept)() /* && me->_is_follow() */ ) {
      if ( thr_observer.bad() ) {
        if ( thr_observer.is_join_req() ) {
          rtc_observer = thr_observer.join();
          if ( rtc_observer != 0 ) {
            rtc = reinterpret_cast<xmt::Thread::ret_t>(-2); // there was connect_processor that was killed
          }
        }
        thr_observer.launch( observer, me, 0, PTHREAD_STACK_MIN * 2 );
      }
    }
  }
  catch ( ... ) {
    rtc = reinterpret_cast<xmt::Thread::ret_t>(-1);
  }

  xmt::block_signal( SIGINT );
  xmt::block_signal( SIGPIPE );
  xmt::block_signal( SIGCHLD );
  xmt::block_signal( SIGPOLL );

  me->_dlock.lock();
  me->_follow = false;
  me->_pool_cnd.set( true, true );
  me->_observer_cnd.set( true );
  me->_dlock.unlock();

  // me->_c_lock.lock();
  ::close( me->_cfd );
  ::close( me->_pfd[1].fd );
  me->close();
  // me->_c_lock.unlock();
  rtc_observer = thr_observer.join();

  xmt::scoped_lock _l( me->_c_lock );
  me->_M_c.clear(); // FIN still may not come yet; force close
  if ( rtc_observer != 0 && rtc == 0 ) {
    rtc = reinterpret_cast<xmt::Thread::ret_t>(-2); // there was connect_processor that was killed
  }

  return rtc;
}

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
xmt::Thread::ret_t sockmgr_stream_MP<Connect,C,T>::connect_processor( void *p )
{
  _Self_type *me = static_cast<_Self_type *>(p);

  try {
    timespec idle( me->_idle );
    typename _Sequence::iterator c;

    {
      xmt::scoped_lock lk(me->_dlock);
      if ( me->_conn_pool.empty() ) {
        me->_pool_cnd.set( false );
        me->_observer_cnd.set( false );
        return 0;
      }
      c = me->_conn_pool.front();
      me->_conn_pool.pop_front();
      xmt::gettime( &me->_tpop );
    }

    do {
      sockstream& stream = c->s;
      if ( stream.is_open() ) {
        (c->_proc->*C)( stream );
        if ( stream.is_open() && stream.good() ) {
          if ( stream.rdbuf()->in_avail() > 0 ) {
            // socket has buffered data, push it back to queue
            xmt::scoped_lock lk(me->_dlock);
            me->_conn_pool.push_back( c );
            me->_observer_cnd.set( true );
            me->_pool_cnd.set( true );
            if ( !me->_follow ) {
              break;
            }
            c = me->_conn_pool.front();
            me->_conn_pool.pop_front();
            xmt::gettime( &me->_tpop );
            // xmt::Thread::gettime( &me->_tpush );
            continue;
          } else { // no buffered data, return socket to poll
            sock_base::socket_type rfd = stream.rdbuf()->fd();
            ::write( me->_cfd, reinterpret_cast<const char *>(&rfd), sizeof(sock_base::socket_type) );
          }
        } else {
          me->_dlock.lock();
          me->_conn_pool.erase( std::remove( me->_conn_pool.begin(), me->_conn_pool.end(), c ), me->_conn_pool.end() );
          me->_dlock.unlock();

          xmt::scoped_lock _l( me->_c_lock );
          me->_M_c.erase( c );
        }
      }

      {
        xmt::scoped_lock lk(me->_dlock);
        if ( me->_conn_pool.empty() ) {
          lk.unlock();
          if ( me->_pool_cnd.try_wait_delay( &idle ) != 0 ) {
            lk.lock();
            me->_pool_cnd.set( false );
            me->_observer_cnd.set( false );
            return 0;
          }
          if ( !me->_is_follow() ) { // before _conn_pool.front()
            return 0;
          }
          lk.lock();
          if ( me->_conn_pool.empty() ) {
            return 0;
          }
        }
        c = me->_conn_pool.front();
        me->_conn_pool.pop_front();
        xmt::gettime( &me->_tpop );
      }
    } while ( me->_is_follow() );
  }
  catch ( ... ) {
    return reinterpret_cast<xmt::Thread::ret_t>(-1);
  }

  return 0;
}

template <class Connect, void (Connect::*C)( std::sockstream& ), void (Connect::*T)() >
xmt::Thread::ret_t sockmgr_stream_MP<Connect,C,T>::observer( void *p )
{
  _Self_type *me = static_cast<_Self_type *>(p);
  xmt::Thread::ret_t rtc = 0;
  int pool_size[3];
  // size_t thr_pool_size[3];
  timespec tpop;
  timespec delta( me->_busylimit );
  timespec alarm( me->_alarm );

  timespec idle( me->_idle );

  timespec now;
  std::fill( pool_size, pool_size + 3, 0 );

  try {
    xmt::ThreadMgr mgr;

    mgr.launch( connect_processor, me /* , 0, 0, PTHREAD_STACK_MIN * 2 */ );

    do {
      // std::swap( pool_size[0], pool_size[1] );
      std::rotate( pool_size, pool_size, pool_size + 3 );
      {
        xmt::scoped_lock lk(me->_dlock);
        pool_size[2] = static_cast<int>(me->_conn_pool.size());
        tpop = me->_tpop;
      }
      if ( pool_size[2] != 0 ) {
        if ( me->_thr_limit > mgr.size() ) {
          if ( (pool_size[0] - 2 * pool_size[1] + pool_size[2]) > 0 ||
               pool_size[2] > me->_thr_limit
               /* pool_size[1] > 3 && pool_size[0] <= pool_size[1] */ ) {
            // queue not empty and not decrease
            mgr.launch( connect_processor, me /* , 0, 0, PTHREAD_STACK_MIN * 2 */ );
          } else {
            xmt::gettime( &now );
            if ( (tpop + delta) < now ) {
              // a long time was since last pop from queue
              mgr.launch( connect_processor, me /* , 0, 0, PTHREAD_STACK_MIN * 2 */ );
            }
          }
        }
        mgr.garbage_collector();
        xmt::delay( &alarm );
      } else {
        if (  /* me->_is_follow() && */ me->_observer_cnd.try_wait_delay( &idle ) != 0 && mgr.size() == 0 ) {
          return rtc;
        }
      }
    } while ( me->_is_follow() );

    int count = 24;
    while ( mgr.size() > 0 && count > 0 ) {
      me->_pool_cnd.set( true, true );
      xmt::delay( &alarm );
      alarm *= 1.2;
      --count;
    }
    if ( mgr.size() > 0 ) {
      mgr.signal( SIGTERM );
      rtc = reinterpret_cast<xmt::Thread::ret_t>(-1);
    }
  }
  catch ( ... ) {
    rtc = reinterpret_cast<xmt::Thread::ret_t>(-1);
  }

  return rtc;
}

#endif // !__FIT_NO_POLL

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif
