// -*- C++ -*- Time-stamp: <99/04/16 19:42:07 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <algorithm>

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

  MT_REENTRANT( _storage_lock, _1 );
  sockmgr_client *&cl = _storage[ create_unique() ];

  __stl_assert( cl == 0 );

  cl = new sockmgr_client();

  __stl_assert( cl != 0 );

  cl->s.open( _sd, addr.any );
  cl->s.rdbuf()->hostname( cl->hostname );

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

    MT_LOCK( _storage_lock );
    container_type::iterator i = _storage.begin();
    sockbuf *b;
    while ( i != _storage.end() ) {
      b = (*i).second->s.rdbuf();
      if ( /* b->stype() == sock_base::sock_dgram && */ // all dgram here
        b->port() == addr.inet.sin_port &&
        b->inet_addr() == addr.inet.sin_addr.s_addr ) {
        break;
      }
      ++i;
    }

    if ( i == _storage.end() || !(*i).second->thrID.good() ) {
      sockmgr_client *&cl = _storage[ create_unique() ];
      __stl_assert( cl == 0 );
      cl = new sockmgr_client();
      __stl_assert( cl != 0 );
      cl->s.open( dup( fd() ), addr.any, sock_base::sock_dgram );
      MT_UNLOCK( _storage_lock );
      cl->s.rdbuf()->hostname( cl->hostname );

      return cl;
    }
    // That's dangerous: garbage collector in working!
//    if ( !(*i).second->thrID.good() ) {
//      sockmgr_client *&cl = _storage[ (*i).first ];
//      __stl_assert( cl != 0 );
//      if ( !cl->s.is_open() ) {
//        cl->s.open( dup( fd() ), addr.any, sock_base::sock_dgram );
//      }      
//      return cl;
//    }
    // otherwise, thread exist and living, and I wait while it read message
    MT_UNLOCK( _storage_lock );
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
const sockmgr_stream<Connect>::key_type sockmgr_stream<Connect>::_low = 0x0000000a;

template <class Connect>
const sockmgr_stream<Connect>::key_type sockmgr_stream<Connect>::_high = 0x7fffffff;

template <class Connect>
sockmgr_stream<Connect>::key_type sockmgr_stream<Connect>::_id = sockmgr_stream<Connect>::_low;

template <class Connect>
sockmgr_stream<Connect>::key_type sockmgr_stream<Connect>::create_unique()
{
  pair<container_type::iterator,bool> ret;

  do {
    if ( ++_id > _high ) {
      _id = (_id - _low) % (_high - _low) + _low;
    }
  } while ( _storage.find( _id ) != _storage.end() );

  return _id;
}

template <class Connect>
int sockmgr_stream<Connect>::loop( void *p )
{
  sockmgr_stream *me = static_cast<sockmgr_stream *>(p);

  me->_gc_id.launch( garbage_collector, me );
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

      s->thrID.launch( connection, &pass, sizeof(pass) );
    }
  }
  catch ( int sig ) {
    me->shutdown( sock_base::stop_in );
    me->close();
    cerr << "\n--- Interrupted ---" << endl;
  }
  catch ( runtime_error& e ) {
    cerr << e.what() << endl;
    ret_code = -1;
  }
  catch ( exception& e ) {
    cerr << e.what() << endl;
  }
  catch ( ... ) {
    cerr << "(1) Oh, oh, say baby Sally. Dick and Jane launch." << endl;
  }

  return ret_code;
}

template <class Connect>
int sockmgr_stream<Connect>::connection( void *p )
{
  params *pass = static_cast<params *>(p);
  sockmgr_stream *me = pass->me;
  sockmgr_client *client = pass->client;

  int ret_code = 0;
#ifdef __unix
  Thread::unblock_signal( SIGPIPE );
#endif


  try {
    Connect _proc;

    // The user connect function: application processing
    _proc.connect( client->s, client->hostname, client->info );

    // Enforce socket close before thread terminated: this urgent for
    // udp sockstreams policy, and not significant for tcp.
    client->s.close();
  }
  catch ( int ) { // signal
    client->s.close();
  }
  catch ( ios_base::failure& ) {
    
  }
  catch ( ... ) {
    ret_code = -1;
  }

  return ret_code;
}

template <class Connect>
int sockmgr_stream<Connect>::garbage_collector( void *p )
{
  sockmgr_stream *me = static_cast<sockmgr_stream *>(p);
  container_type::iterator i;

  typedef pair<const key_type,sockmgr_client*> container_content;
  typedef select2nd<container_content>     value;

#ifdef __unix
  timespec t;

  t.tv_sec = 12; // 900;
  t.tv_nsec = 0;
#endif
#ifdef WIN32
  int t = 12000; // 900000;
#endif

  while ( me->_garbage_end.set() ) {
#ifdef __unix
    nanosleep( &t, 0 );
#endif
#ifdef WIN32
    Sleep( t );
#endif

    MT_LOCK( me->_storage_lock );
    i =  me->_storage.begin();
    while ( (i = find_if( i, me->_storage.end(), compose1( bad_thread(), value() ) ))
            != me->_storage.end() ) {
      delete (*i).second;
      me->_storage.erase( i++ );
    }
    MT_UNLOCK( me->_storage_lock );
  }

  MT_LOCK( me->_storage_lock );
  for_each( me->_storage.begin(), me->_storage.end(), compose1( remove_client(), value() ) );
  me->_storage.erase( me->_storage.begin(), me->_storage.end() );
  MT_UNLOCK( me->_storage_lock );

  me->_garbage_end.signal();
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
  
  sock_base::socket_type _sd = ::accept( fd(), &addr.any, &sz );
  if ( _sd == -1 ) {
    // check and set errno
    __stl_assert( _sd == -1 );
    return 0;
  }

  MT_REENTRANT( _storage_lock, _1 );
  sockmgr_client_MP<Connect> *&s = _storage[ create_unique() ];

  __stl_assert( s == 0 );

  s = new sockmgr_client_MP<Connect>();

  __stl_assert( s != 0 );

  s->s.open( _sd, addr.any );
  s->s.rdbuf()->hostname( s->hostname );

  return s;
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
  fd_set pfd;
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
  pollfd pfd;
  pfd.fd = fd();
  pfd.events = POLLIN;
  pfd.revents = 0;
  if ( poll( &pfd, 1, -1 ) > 0 ) { // wait infinite
    // get address of caller only
    char buff[32];    
    ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr.any, &sz );
  } else {
    return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
  }
#endif

  MT_REENTRANT( _storage_lock, _1 );
  container_type::iterator i = _storage.begin();
  sockbuf *b;
  while ( i != _storage.end() ) {
    b = (*i).second->s.rdbuf();
    if ( /* b->stype() == sock_base::sock_dgram && */ // all dgram here
         b->port() == addr.inet.sin_port &&
         b->inet_addr() == addr.inet.sin_addr.s_addr ) {
      break;
    }
    ++i;
  }
  sockmgr_client_MP<Connect> *&s = _storage[ i != _storage.end() ? (*i).first : create_unique() ];
  if ( s == 0 ) {
    s = new sockmgr_client_MP<Connect>();
  }
  __stl_assert( s != 0 );

  if ( !s->s.is_open() ) {
    s->s.open( dup( fd() ), addr.any, sock_base::sock_dgram );
    s->s.rdbuf()->hostname( s->hostname );
  }

  return s;
}

template <class Connect>
const sockmgr_stream_MP<Connect>::key_type sockmgr_stream_MP<Connect>::_low = 0x0000000a;

template <class Connect>
const sockmgr_stream_MP<Connect>::key_type sockmgr_stream_MP<Connect>::_high = 0x7fffffff;

template <class Connect>
sockmgr_stream_MP<Connect>::key_type sockmgr_stream_MP<Connect>::_id = sockmgr_stream_MP<Connect>::_low;

template <class Connect>
sockmgr_stream_MP<Connect>::key_type sockmgr_stream_MP<Connect>::create_unique()
{
  pair<container_type::iterator,bool> ret;

  do {
    if ( ++_id > _high ) {
      _id = (_id - _low) % (_high - _low) + _low;
    }
  } while ( _storage.find( _id ) != _storage.end() );

  return _id;
}

template <class Connect>
int sockmgr_stream_MP<Connect>::loop( void *p )
{
  sockmgr_stream_MP *me = static_cast<sockmgr_stream_MP *>(p);

  me->_gc_id.launch( garbage_collector, me );
#ifdef __unix
  Thread::unblock_signal( SIGINT );
#endif

  int ret_code = 0;

  set_unexpected( unexpected );
  set_terminate( terminate );

  try {
    sockmgr_client_MP<Connect> *s;

    while ( (s = me->accept()) != 0 ) {    
      // The user connect function: application processing
      s->_proc.connect( s->s, s->hostname, s->info );
    }
  }
  catch ( int sig ) {
    me->shutdown( sock_base::stop_in );
    me->close();
    cerr << "\n--- Interrupted MP ---" << endl;
  }
  catch ( runtime_error& e ) {
    cerr << e.what() << endl;
    ret_code = -1;
  }
  catch ( exception& e ) {
    cerr << e.what() << endl;
  }
  catch ( ... ) {
    cerr << "(1) Oh, oh, say baby Sally. Dick and Jane launch." << endl;
  }

  return ret_code;
}


template <class Connect>
int sockmgr_stream_MP<Connect>::garbage_collector( void *p )
{
  sockmgr_stream_MP *me = static_cast<sockmgr_stream_MP *>(p);
  container_type::iterator i;

  typedef pair<const key_type,sockmgr_client_MP<Connect> *> container_content;
  typedef select2nd<container_content>     value;
  typedef bad_connect<Connect>  bad_connect_type;

#ifdef __unix
  timespec t;

  t.tv_sec = 12; // 900;
  t.tv_nsec = 0;
#endif
#ifdef WIN32
  int t = 12000; // 900000;
#endif

  while ( me->_garbage_end.set() ) {
#ifdef __unix
    nanosleep( &t, 0 );
#endif
#ifdef WIN32
    Sleep( t );
#endif

    MT_LOCK( me->_storage_lock );
    i =  me->_storage.begin();
    while ( (i = find_if( i, me->_storage.end(), compose1( bad_connect_type(), value() ) ))
            != me->_storage.end() ) {
      delete (*i).second;
      me->_storage.erase( i++ );
    }
    MT_UNLOCK( me->_storage_lock );
  }

  MT_LOCK( me->_storage_lock );
  for_each( me->_storage.begin(), me->_storage.end(), compose1( remove_client_MP<Connect>(), value() ) );
  me->_storage.erase( me->_storage.begin(), me->_storage.end() );
  MT_UNLOCK( me->_storage_lock );

  me->_garbage_end.signal();
  return 0;
}

} // namespace std
