// -*- C++ -*- Time-stamp: <03/07/05 09:50:46 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
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
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <algorithm>

#ifdef __unix
extern "C" int nanosleep(const struct timespec *, struct timespec *);
#endif

using __impl::Thread;

_STLP_BEGIN_NAMESPACE

#ifndef __FIT_NO_POLL
template <class Connect>
void sockmgr_stream_MP<Connect>::open( int port, sock_base::stype t )
{
  basic_sockmgr::open( port, t, sock_base::inet );
  MT_REENTRANT( _fd_lck, _1 );
  if ( is_open_unsafe() ) {
    if ( t == sock_base::sock_stream ) {
      _accept = &_Self_type::accept_tcp;
      if ( _pfd == 0 ) { // ?? seems _pfd here always should be 0
        _pfd = new pollfd[1024];
        _pfd[0].fd = fd_unsafe();
        _pfd[0].events = POLLIN;
        ++_fdcount;
        _STLP_ASSERT( _fdcount == 1 );
      }
    } else if ( t == sock_base::sock_dgram ) {
      _accept = &_Self_type::accept_udp;
      if ( _pfd == 0 ) {
        _pfd = new pollfd[1];
        _pfd[0].fd = fd_unsafe();
        _pfd[0].events = POLLIN;
        ++_fdcount;
      }
    } else {
      throw invalid_argument( "sockmgr_stream_MP" );
    }
    
    loop_id.launch( loop, this );
  }
}

template <class Connect>
__FIT_TYPENAME sockmgr_stream_MP<Connect>::_Connect *sockmgr_stream_MP<Connect>::_shift_fd()
{
  _Connect *msg = 0;
  int j = 1;
  while ( j < _fdcount ) {
    if ( _pfd[j].revents != 0 ) {
      // We should distinguish closed socket from income message
      typename container_type::iterator i = 
        find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, _pfd[j].fd ) );
      // Solaris return ERROR on poll, before close socket
      if ( i == _M_c.end() ) {
        // Socket already closed (may be after read/write failure)
        // this way may not notify poll (like in HP-UX 11.00) via POLLERR flag
        // as made in Solaris
        --_fdcount;
        memmove( &_pfd[j], &_pfd[j+1], sizeof(struct pollfd) * (_fdcount - j) );
        for ( i = _M_c.begin(); i != _M_c.end(); ++i ) {
          if ( (*i)->s->rdbuf()->fd() == -1 ) {
            (*i)->s->close();
            (*i)->_proc->close();
          }
        }
        continue;
      } else if ( _pfd[j].revents & POLLERR /* | POLLHUP | POLLNVAL */ ) {
        // poll first see closed socket
        --_fdcount;
        memmove( &_pfd[j], &_pfd[j+1], sizeof(struct pollfd) * (_fdcount - j) );
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
        int nr = recv( _pfd[j].fd, reinterpret_cast<void *>(&x), 1, MSG_PEEK );
        if ( nr <= 0 ) { // I can't read even one byte: this designate closed
                         // socket operation
          --_fdcount;
          memmove( &_pfd[j], &_pfd[j+1], sizeof(struct pollfd) * (_fdcount - j) );
          (*i)->s->close();
          (*i)->_proc->close();
          continue;
        }
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

template <class Connect>
__FIT_TYPENAME sockmgr_stream_MP<Connect>::_Connect *sockmgr_stream_MP<Connect>::accept_tcp()
{
  if ( !is_open() ) {
    return 0;
  }

  _xsockaddr addr;
  size_t sz = sizeof( sockaddr_in );
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
    if ( poll( _pfd, _fdcount, -1 ) < 0 ) { // wait infinite
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
        _STLP_ASSERT( _sd == -1 );
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

  size_t sz = sizeof( sockaddr_in );
  _xsockaddr addr;

  if ( poll( _pfd, 1, -1 ) < 0 ) { // wait infinite
    return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
  }
  // get address of caller only
  char buff[32];    
  ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr.any, &sz );
  _Connect *cl;
  try {
    _c_lock._M_acquire_lock();
    typename container_type::iterator i = _M_c.begin();
    sockbuf *b;
    while ( i != _M_c.end() ) {
      b = (*i)->s->rdbuf();
      if ( (*i)->s->is_open() && b->stype() == sock_base::sock_dgram &&
           b->port() == addr.inet.sin_port &&
           b->inet_addr() == addr.inet.sin_addr.s_addr ) {
        _c_lock._M_release_lock();
        return *i;
      }
      ++i;
    }

    cl = new _Connect();
    cl->s = new sockstream();
    _M_c.push_back( cl );
    cl->s->open( dup( fd() ), addr.any, sock_base::sock_dgram );
    cl->_proc = new Connect( *cl->s );
    _c_lock._M_release_lock();
  }
  catch ( ... ) {
    _c_lock._M_release_lock();
    cl = 0;
  }
  return cl;
}

template <class Connect>
void sockmgr_stream_MP<Connect>::_close_by_signal( int )
{
#ifdef _PTHREADS
  void *_uw_save = *((void **)pthread_getspecific( Thread::mtkey() ) + _idx );
  _Self_type *me = static_cast<_Self_type *>( _uw_save );

  me->close();
#else
#error "Fix me!"
#endif
}

template <class Connect>
int sockmgr_stream_MP<Connect>::loop( void *p )
{
  _Self_type *me = static_cast<_Self_type *>(p);
  me->loop_id.pword( _idx ) = me; // push pointer to self for signal processing

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
        if ( !s->s->is_open() ) { // remove all closed sockets from poll
          for ( int i = 1; i < me->_fdcount; ++i ) {
            if ( me->_pfd[i].fd == _sfd ) {
              --me->_fdcount;
              memmove( &me->_pfd[i], &me->_pfd[i+1], sizeof(struct pollfd) * (me->_fdcount - i) );
              break;
            }
          }
        }
      }
    }
  }
  catch ( ... ) {
    me->_c_lock._M_acquire_lock();
    
    for ( typename container_type::iterator i = me->_M_c.begin(); i != me->_M_c.end(); ++i ) {
      if ( (*i)->s->is_open() ) { // close all not closed yet
        (*i)->s->close();
        (*i)->_proc->close();
      }
    }
    me->close();
    me->_c_lock._M_release_lock();
    throw;
  }

  me->_c_lock._M_acquire_lock();
  
  for ( typename container_type::iterator i = me->_M_c.begin(); i != me->_M_c.end(); ++i ) {
    if ( (*i)->s->is_open() ) { // close all not closed yet
      (*i)->s->close();
      (*i)->_proc->close();
    }
  }
  me->close();
  me->_c_lock._M_release_lock();

  return 0;
}

#endif // !__FIT_NO_POLL

#ifndef __FIT_NO_SELECT
template <class Connect>
void sockmgr_stream_MP_SELECT<Connect>::open( int port, sock_base::stype t )
{
  basic_sockmgr::open( port, t, sock_base::inet );
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
    
    loop_id.launch( loop, this );
  }
}

template <class Connect>
__FIT_TYPENAME sockmgr_stream_MP_SELECT<Connect>::_Connect *sockmgr_stream_MP_SELECT<Connect>::_shift_fd()
{
  _Connect *msg = 0;
  for ( int j = 0; j <= _fdmax; ++j ) {
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
    if ( select( _fdmax + 1, &_pfdr, 0, &_pfde, 0 ) > 0 ) {
      MT_REENTRANT( _fd_lck, _1 );
      if ( !is_open_unsafe() || FD_ISSET( fd_unsafe(), &_pfde ) ) { // may be already closed
        return 0;
      }

      if ( FD_ISSET( fd_unsafe(), &_pfdr ) ) { // select found event on binded socket
        sock_base::socket_type _sd = ::accept( fd_unsafe(), &addr.any, &sz );
        if ( _sd == -1 ) {
          // check and set errno
          _STLP_ASSERT( _sd == -1 );
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
    } else { // select return < 0
      return 0;
    }
    MT_LOCK( _fd_lck );
    more = is_open_unsafe() && FD_ISSET( fd_unsafe(), &_pfdr );
    MT_UNLOCK( _fd_lck );
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
    _c_lock._M_acquire_lock();
    typename container_type::iterator i = _M_c.begin();
    sockbuf *b;
    while ( i != _M_c.end() ) {
      b = (*i)->s->rdbuf();
      if ( (*i)->s->is_open() && b->stype() == sock_base::sock_dgram &&
           b->port() == addr.inet.sin_port &&
           b->inet_addr() == addr.inet.sin_addr.s_addr ) {
        _c_lock._M_release_lock();
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
    _c_lock._M_release_lock();
  }
  catch ( ... ) {
    _c_lock._M_release_lock();
    cl = 0;
  }
  return cl;
}

template <class Connect>
void sockmgr_stream_MP_SELECT<Connect>::_close_by_signal( int )
{
#ifdef _PTHREADS
  void *_uw_save = *((void **)pthread_getspecific( Thread::mtkey() ) + _idx );
  _Self_type *me = static_cast<_Self_type *>( _uw_save );

  me->close();
#else
#error "Fix me!"
#endif
}

template <class Connect>
int sockmgr_stream_MP_SELECT<Connect>::loop( void *p )
{
  _Self_type *me = static_cast<_Self_type *>(p);
  me->loop_id.pword( _idx ) = me; // push pointer to self for signal processing

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
    me->_c_lock._M_acquire_lock();
    
    for ( typename container_type::iterator i = me->_M_c.begin(); i != me->_M_c.end(); ++i ) {
      if ( (*i)->s->is_open() ) { // close all not closed yet
        (*i)->s->close();
        (*i)->_proc->close();
      }
    }
    me->close();
    me->_c_lock._M_release_lock();
    throw;
  }

  me->_c_lock._M_acquire_lock();
  
  for ( typename container_type::iterator i = me->_M_c.begin(); i != me->_M_c.end(); ++i ) {
    if ( (*i)->s->is_open() ) { // close all not closed yet
      (*i)->s->close();
      (*i)->_proc->close();
    }
  }
  me->close();
  me->_c_lock._M_release_lock();

  return 0;
}

#endif // !__FIT_NO_SELECT

_STLP_END_NAMESPACE
