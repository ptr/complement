// -*- C++ -*- Time-stamp: <99/01/29 19:05:44 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

// #include <csignal>
#include <signal.h>

using __impl::Thread;

namespace std {

template <class Connect>
sockmgr_client *sockmgr_stream<Connect>::accept()
{
  if ( !is_open() ) {
    return 0;
  }

  _xsockaddr addr;
  int sz = sizeof( sockaddr_in );
  
  SOCKET _sd = ::accept( fd(), &addr._address.any, &sz );
  if ( _sd == -1 ) {
    // check and set errno
    __stl_assert( _sd == -1 );
    return 0;
  }

  MT_REENTRANT( _storage_lock, _1 );
  sockmgr_client *&cl = _storage[addr];
  if ( cl != 0 ) {

    __stl_assert( !cl->s.is_open() );

    if ( cl->s.is_open() ) {
      // ???
      return 0;
    }
  } else {
    cl = new sockmgr_client();
    if ( cl == 0 ) {
      __stl_assert( cl == 0 );
      return 0;
    }    
  }
  cl->s.open( _sd, addr._address.any );

  return cl;
}

template <class Connect>
sockstream *sockmgr_stream<Connect>::accept_dgram()
{
  if ( !is_open() ) {
    return 0;
  }

  int sz = sizeof( sockaddr_in );
  _xsockaddr addr;
	
  pollfd pfd;
  pfd.fd = fd();
  pfd.events = POLLIN;
  pfd.revents = 0;
  if ( poll( &pfd, 1, -1 ) > 0 ) { // wait infinite
    // get address of caller only
    char buff[32];    
    ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr._address.any, &sz );
  }

  MT_REENTRANT( _storage_lock, _1 );
  sockmgr_client *&cl = _storage[addr];
  if ( cl != 0 ) {
//    if ( cl->s.is_open() ) {
      // ???
//      return 0;
//    }
  } else {
    cl = new sockmgr_client();
    if ( cl == 0 ) {
      return 0;
    }    
  }
  if ( !cl->s.is_open() ) {
    cl->s.open( fd(), addr._address.any, sock_base::sock_dgram );
  }

  return &cl->s;
}

template <class Connect>
void sockmgr_stream<Connect>::erase( sockstream *s )
{
  if ( s == 0 ) {
    return;
  }
  if ( s->is_open() ) {
    s->close();
  }
  _xsockaddr addr;

  addr._address.any.sa_family = s->rdbuf()->family();
  if ( addr._address.any.sa_family == AF_INET ) {
    addr._address.inet.sin_port = s->rdbuf()->port();
    addr._address.inet.sin_addr.s_addr = s->rdbuf()->inet_addr();
  }

  MT_REENTRANT( _storage_lock, _1 );
  container_type::iterator i = _storage.find( addr );
  if ( i != _storage.end() ) {
    (*i).second->s.close();
    delete (*i).second;
    _storage.erase( i );
  }
}

template <class Connect>
void sockmgr_stream<Connect>::broken_pipe( int )
{
  cerr << "broken pipe detected" << endl;
}

template <class Connect>
int sockmgr_stream<Connect>::loop( void *p )
{
  sockmgr_stream *me = static_cast<sockmgr_stream *>(p);
  sockmgr_client *s;
  string who;
  hostent he;
  char tmp_buff[1024];
  int err = 0;
  params pass;
  in_addr in;
  int ret_code = 0;

  pass.me = me;

#ifdef __unix  // catch SIGPIPE here
  sigset_t sigset;

  sigemptyset( &sigset );
  sigaddset( &sigset, SIGPIPE );

#ifdef _SOLARIS_THREADS
  thr_sigsetmask( SIG_BLOCK, &sigset, 0 );
#endif
#ifdef _PTHREADS
  pthread_sigsetmask( SIG_BLOCK, &sigset, 0 );
#endif
 
  struct sigaction act;

  act.sa_handler = SIG_IGN;
  sigaction( SIGPIPE, &act, 0 );
#endif

  set_unexpected( unexpected );
  set_terminate( terminate );

  try {
    while ( (s = me->accept()) != 0 ) {
      in.s_addr = s->s.rdbuf()->inet_addr();
      if ( gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
			    &he, tmp_buff, 1024, &err ) != 0 ) {
	s->hostname = he.h_name;
      } else {
	s->hostname = "unknown";
      }

      s->hostname += " [";
      s->hostname += inet_ntoa( in );
      s->hostname += "]";
    
      cerr << s->hostname << endl;
      pass.client = s;

      s->thrID.launch( connection, &pass, sizeof(pass) );
    }
  }
  catch ( runtime_error& e ) {
    cerr << e.what() << endl;
    ret_code = -1;
  }
  catch ( ... ) {
    cerr << "Oh, oh, say baby Sally. Dick and Jane launch." << endl;
  }

//  __stl_assert( false );

  return ret_code;
}

template <class Connect>
int sockmgr_stream<Connect>::connection( void *p )
{
  params *pass = static_cast<params *>(p);
  sockmgr_stream *me = pass->me;
  sockmgr_client *client = pass->client;

  int ret_code = 0;

#ifdef __unix  // catch SIGPIPE here
  sigset_t sigset;

  sigemptyset( &sigset );
  sigaddset( &sigset, SIGPIPE );
#ifdef _SOLARIS_THREADS
  thr_sigsetmask( SIG_BLOCK, &sigset, 0 );
#endif
#ifdef _PTHREADS
  pthread_sigsetmask( SIG_BLOCK, &sigset, 0 );
#endif
  struct sigaction act;

  act.sa_handler = SIG_PF(broken_pipe);
  sigaction( SIGPIPE, &act, 0 );
#endif

  try {
    Connect _proc;

    _proc.connect( client->s ); // The user connect function: application processing
  }
  catch ( ios_base::failure& ) {
    
  }
  catch ( ... ) {
    ret_code = -1;
  }

  me->erase( &client->s );

  return ret_code;
}

} // namespace std
