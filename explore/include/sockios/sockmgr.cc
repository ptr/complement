// -*- C++ -*- Time-stamp: <99/02/25 18:46:24 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifdef __unix
// Current <csignal> not include all needed prototipes
#include <signal.h>
#endif

#include <algorithm>

#ifdef WIN32
namespace std {
// select1st and select2nd are extensions: they are not part of the standard.
template <class _Pair>
struct _Select1st : public unary_function<_Pair, typename _Pair::first_type> {
  const result_type& operator()(const _Pair& __x) const {
    return __x.first;
  }
};

template <class _Pair>
struct _Select2nd : public unary_function<_Pair, typename _Pair::second_type>
{
  const result_type& operator()(const _Pair& __x) const {
    return __x.second;
  }
};

template <class _Pair> struct select1st : public _Select1st<_Pair> {};
template <class _Pair> struct select2nd : public _Select2nd<_Pair> {};

// unary_compose and binary_compose (extensions, not part of the standard).

template <class _Operation1, class _Operation2>
class unary_compose
  : public unary_function<typename _Operation2::argument_type,
                          typename _Operation1::result_type> 
{
protected:
  _Operation1 __op1;
  _Operation2 __op2;
public:
  unary_compose(const _Operation1& __x, const _Operation2& __y) 
    : __op1(__x), __op2(__y) {}
  result_type operator()(const argument_type& __x) const {
    return __op1(__op2(__x));
  }
};

template <class _Operation1, class _Operation2>
inline unary_compose<_Operation1,_Operation2> 
compose1(const _Operation1& __op1, const _Operation2& __op2)
{
  return unary_compose<_Operation1,_Operation2>(__op1, __op2);
}

} // namespace std

#endif // WIN32

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

#ifdef WIN32
  fd_set pfd;
  FD_ZERO( &pfd );
  FD_SET( fd(), &pfd );

  if ( select( fd() + 1, &pfd, 0, 0, 0 ) > 0 ) {
    // get address of caller only
    char buff[32];    
    ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr._address.any, &sz );
  }
#else
  pollfd pfd;
  pfd.fd = fd();
  pfd.events = POLLIN;
  pfd.revents = 0;
  if ( poll( &pfd, 1, -1 ) > 0 ) { // wait infinite
    // get address of caller only
    char buff[32];    
    ::recvfrom( fd(), buff, 32, MSG_PEEK, &addr._address.any, &sz );
  }
#endif

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


#ifdef __unix

template <class Connect>
sockmgr_stream<Connect> *sockmgr_stream<Connect>::__self = 0;

template <class Connect>
void sockmgr_stream<Connect>::broken_pipe( int )
{
  cerr << "\nbroken pipe detected" << endl;
}

template <class Connect>
void sockmgr_stream<Connect>::interrupt( int )
{
  cerr << "\nInterrupted" << endl;
  __stl_assert( __self != 0 );
  __self->shutdown( sock_base::stop_in );
  __self->close();
}

#endif

template <class Connect>
int sockmgr_stream<Connect>::loop( void *p )
{
  sockmgr_stream *me = static_cast<sockmgr_stream *>(p);
#ifdef __unix
  __self = me;
#endif
//  me->_loop_end.set( false );

  me->_garbage_end.set( false );
  me->_gc_id.launch( garbage_collector, me );

  sockmgr_client *s;
  string who;
#ifdef WIN32
  hostent *he;
#else
  hostent he;
#endif
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
  sigaddset( &sigset, SIGINT );

#ifdef _SOLARIS_THREADS
  thr_sigsetmask( SIG_BLOCK, &sigset, 0 );
#endif
#ifdef _PTHREADS
  pthread_sigsetmask( SIG_BLOCK, &sigset, 0 );
#endif
 
  struct sigaction act;

  act.sa_handler = SIG_IGN;
  sigaction( SIGPIPE, &act, 0 );
  act.sa_handler = SIG_PF(interrupt);
  sigaction( SIGINT, &act, 0 );
#endif

  set_unexpected( unexpected );
  set_terminate( terminate );

  try {
    while ( (s = me->accept()) != 0 ) {
      in.s_addr = s->s.rdbuf()->inet_addr();
#ifdef WIN32
      // For Win he is thread private data, so that's safe
      he = gethostbyaddr( (char *)&in.s_addr, sizeof(in_addr), AF_INET );
      if ( he != 0 ) {
	s->hostname = he->h_name;
      } else {
	s->hostname = "unknown";
      }
#else
      if ( gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
			    &he, tmp_buff, 1024, &err ) != 0 ) {
	s->hostname = he.h_name;
      } else {
	s->hostname = "unknown";
      }
#endif

      s->hostname += " [";
      s->hostname += inet_ntoa( in );
      s->hostname += "]";
    
      pass.client = s;

      s->thrID.launch( connection, &pass, sizeof(pass) );
    }
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

//  __stl_assert( false );

  me->_garbage_end.set( true );

//  me->_loop_end.signal();

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

    // The user connect function: application processing
    _proc.connect( client->s, client->hostname, client->info );
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

  typedef pair<const _xsockaddr,sockmgr_client*> container_content;
  typedef select2nd<container_content>     value;

#ifdef __unix
  timespec t;

  t.tv_sec = 900; // 900;
  t.tv_nsec = 0;
#endif
#ifdef WIN32
  int t = 900000; // 900000;
#endif

  while ( !me->_garbage_end.set() ) {
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

  return 0;
}

} // namespace std
