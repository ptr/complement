// -*- C++ -*- Time-stamp: <99/06/02 20:25:56 ptr>

#ident "$SunId$ %Q%"

#include <algorithm>

#ifdef __unix
extern "C" int nanosleep(const struct timespec *, struct timespec *);
#endif

#ifdef WIN32
#include <_algorithm>
#endif // WIN32

using __impl::Thread;

namespace std {

template <class Connect>
void sockmgr_stream<Connect>::open( int port, sock_base::stype t )
{
  basic_sockmgr::open( port, t, sock_base::inet );
  if ( is_open() ) {
    if ( t == sock_base::sock_stream ) {
      _accept = accept_tcp;
    } else if ( t == sock_base::sock_dgram ) {
      _accept = accept_udp;
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
  int sz = sizeof( sockaddr_in );
  
  sock_base::socket_type _sd = ::accept( fd(), &addr.any, &sz );
  if ( _sd == -1 ) {
    // check and set errno
    __stl_assert( _sd == -1 );
    return 0;
  }

  MT_REENTRANT( _c_lock, _1 );
  sockmgr_client *cl;

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

  return cl;
}

template <class Connect>
sockmgr_client *sockmgr_stream<Connect>::accept_udp()
{
  if ( !is_open() ) {
    return 0;
  }

  int sz = sizeof( sockaddr_in );
  _xsockaddr addr;
#ifdef __unix
  timespec t;

  t.tv_sec = 0;
  t.tv_nsec = 10000;

  pollfd pfd;
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

    MT_LOCK( _c_lock );
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
      MT_UNLOCK( _c_lock );
      // cl->s.rdbuf()->hostname( _M_c.front()->hostname );
      return cl;
    }
    // otherwise, thread exist and living, and I wait while it read message
    MT_UNLOCK( _c_lock );
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

  set_unexpected( unexpected );
  set_terminate( terminate );

  try {
    while ( (s = me->accept()) != 0 ) {
    
      pass.client = s;

      me->thr_mgr.launch( connection, &pass, sizeof(pass) );
    }
  }
  catch ( ... ) {
    me->shutdown( sock_base::stop_in );
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
      _accept = accept_tcp;
    } else if ( t == sock_base::sock_dgram ) {
      _accept = accept_udp;
    } else {
      throw invalid_argument( "sockmgr_stream_MP" );
    }
    
    loop_id.launch( loop, this );
  }
}

template <class Connect>
sockmgr_client_MP<Connect> *sockmgr_stream_MP<Connect>::accept_tcp()
{
  if ( !is_open() ) {
    return 0;
  }

  _xsockaddr addr;
  int sz = sizeof( sockaddr_in );

#ifdef __unix
  if ( _pfd == 0 ) {
    _pfd = new pollfd[256];
    _pfd[0].fd = fd();
    _pfd[0].events = POLLIN;
    ++_fdcount;
  }
#endif

  do {
#ifdef WIN32
    if ( _fdcount == 0 ) {
      FD_ZERO( &_pfd );
      ++_fdcount;
    }
    FD_SET( fd(), &_pfd );
    if ( select( FD_SETSIZE, &_pfd, 0, 0, 0 ) < 0 ) {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#else
    _pfd[0].revents = 0;
    if ( poll( _pfd, _fdcount, -1 ) < 0 ) { // wait infinite
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#endif

    if (
#ifdef __unix
      _pfd[0].revents != 0
#endif
#ifdef WIN32
      FD_ISSET( fd(), &_pfd );
#endif
      ) {
      sock_base::socket_type _sd = ::accept( fd(), &addr.any, &sz );
      if ( _sd == -1 ) {
        // check and set errno
        __stl_assert( _sd == -1 );
        return 0;
      }

      MT_REENTRANT( _c_lock, _1 );
      sockmgr_client_MP<Connect> *cl;

      container_type::iterator i = 
        find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, -1 ) );
    
      if ( i == _M_c.end() ) {
        cl = new sockmgr_client_MP<Connect>();
        _M_c.push_back( cl );      
      } else {
        cl = *i;
      }

      cl->s.open( _sd, addr.any );

#ifdef __unix
      int j = _fdcount++;

      _pfd[j].fd = _sd;
      _pfd[j].events = POLLIN;
      _pfd[j].revents = 0;
#endif
#ifdef WIN32
      FD_SET( _sd, &_pfd );
#endif
    }
#ifdef __unix
  } while ( _pfd[0].revents != 0 );
#endif
#ifdef WIN32
  } while ( FD_ISSET( fd(), &_pfd ) );
#endif
  // find polled and return it
#ifdef __unix
  for ( int j = 1; j < _fdcount; ++j ) {
    if ( _pfd[j].revents != 0 ) {
      container_type::iterator i = 
        find_if( _M_c.begin(), _M_c.end(), bind2nd( _M_comp, _pfd[j].fd ) );
      __stl_assert( i != _M_c.end() );
      if ( _pfd[j].revents & POLLERR ) {
        memmove( &_pfd[j], &_pfd[j+1], sizeof(pollfd) * (_fdcount - j - 1) );
        --_fdcount;
        (*i)->s.close();
      }
      _pfd[j].revents = 0;
      return *i;
    }
  }
#endif
#ifdef WIN32
  container_type::iterator i = _M_c.begin();
  while ( i != _M_c.end() ) {
    if ( (*i)->s.is_open() && FD_ISSET( (*i)->s.rdbuf()->fd(), &_pfd ) ) {
      return *i;
    }
    ++i;
  }
#endif

  return 0;
}

template <class Connect>
sockmgr_client_MP<Connect> *sockmgr_stream_MP<Connect>::accept_udp()
{
  if ( !is_open() ) {
    return 0;
  }

  int sz = sizeof( sockaddr_in );
  _xsockaddr addr;

#ifdef WIN32
  if ( _fdcount == 0 ) {
    FD_ZERO( &_pfd );
    FD_SET( fd(), &_pfd );
    ++_fdcount;
  }

  if ( select( fd() + 1, &pfd, 0, 0, 0 ) > 0 ) {
    // get address of caller only
    char buff[32];    
    ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr.any, &sz );
  } else {
    return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
  }
#else
  if ( _pfd == 0 ) {
    _pfd = new pollfd[1];
    _pfd[0].fd = fd();
    _pfd[0].events = POLLIN;
    ++_fdcount;
  }

  if ( poll( _pfd, 1, -1 ) > 0 ) { // wait infinite
    // get address of caller only
    char buff[32];    
    ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr.any, &sz );
  } else {
    return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
  }
#endif
  MT_REENTRANT( _c_lock, _1 );
  container_type::iterator i = _M_c.begin();
  sockbuf *b;
  while ( i != _M_c.end() ) {
    b = (*i)->s.rdbuf();
    if ( (*i)->s.is_open() && b->stype() == sock_base::sock_dgram &&
         b->port() == addr.inet.sin_port &&
         b->inet_addr() == addr.inet.sin_addr.s_addr ) {
      return *i;
    }
    ++i;
  }

  sockmgr_client_MP<Connect> *cl;
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

  return cl;
}

template <class Connect>
int sockmgr_stream_MP<Connect>::loop( void *p )
{
  sockmgr_stream_MP *me = static_cast<sockmgr_stream_MP *>(p);

#ifdef __unix
  Thread::unblock_signal( SIGINT );
#endif

  set_unexpected( unexpected );
  set_terminate( terminate );

  try {
    sockmgr_client_MP<Connect> *s;
    unsigned _sfd;

    while ( (s = me->accept()) != 0 ) {    
      // The user connect function: application processing
      if ( s->s.is_open() ) {
        _sfd = s->s.rdbuf()->fd();
        s->_proc.connect( s->s );
        if ( !s->s.good() ) {
          s->s.close();
        }
        if ( !s->s.is_open() ) {
#ifdef __unix
          for ( int i = 1; i < me->_fdcount; ++i ) {
            if ( me->_pfd[i].fd == _sfd ) {
              memmove( &me->_pfd[i], &me->_pfd[i+1], sizeof(pollfd) * (me->_fdcount - i - 1) );
              --me->_fdcount;
            }
          }
#endif
#ifdef WIN32
          FD_CLR( _sfd, &me->_pfd );
#endif
        }
      }
    }
  }
  catch ( ... ) {
    me->shutdown( sock_base::stop_in );
    MT_REENTRANT( me->_c_lock, _1 );
    container_type::iterator i = me->_M_c.begin();
    while ( i != me->_M_c.end() ) {
      (*i++)->s.close();
    }
    me->close();
    throw;
  }

  return 0;
}

} // namespace std
