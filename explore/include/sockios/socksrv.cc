// -*- C++ -*- Time-stamp: <08/06/17 14:54:16 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

namespace std {

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( const in_addr& addr, int port, sock_base::stype type, sock_base::protocol prot )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
#ifdef WIN32
  ::WSASetLastError( 0 );
#endif
  if ( prot == sock_base::inet ) {
    basic_socket_t::_fd = socket( PF_INET, type, 0 );
    if ( basic_socket_t::_fd == -1 ) {
      _state |= ios_base::failbit | ios_base::badbit;
      return;
    }
    // _open = true;
    basic_socket_t::_address.inet.sin_family = AF_INET;
    basic_socket_t::_address.inet.sin_port = htons( port );
    basic_socket_t::_address.inet.sin_addr.s_addr = addr.s_addr;

    if ( type == sock_base::sock_stream || type == sock_base::sock_seqpacket ) {
      // let's try reuse local address
      setoptions_unsafe( sock_base::so_reuseaddr, true );
    }

    if ( ::bind( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof(basic_socket_t::_address) ) == -1 ) {
      _state |= ios_base::failbit;
#ifdef WIN32
      ::closesocket( basic_socket_t::_fd );
#else
      ::close( basic_socket_t::_fd );
#endif
      basic_socket_t::_fd = -1;
      return;
    }

    if ( type == sock_base::sock_stream || type == sock_base::sock_seqpacket ) {
      // I am shure, this is socket of type SOCK_STREAM | SOCK_SEQPACKET,
      // so don't check return code from listen
      ::listen( basic_socket_t::_fd, SOMAXCONN );
      basic_socket_t::mgr->push( *this );
    }
  } else if ( prot == sock_base::local ) {
    return;
  } else {
    return;
  }
  _state = ios_base::goodbit;

  return;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::close()
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( !basic_socket_t::is_open_unsafe() ) {
    return;
  }
  basic_socket<charT,traits,_Alloc>::mgr->pop( *this, basic_socket_t::_fd );
#ifdef WIN32
  ::closesocket( basic_socket_t::_fd );
#else
  ::shutdown( basic_socket_t::_fd, 2 );
  ::close( basic_socket_t::_fd );
#endif
  basic_socket_t::_fd = -1;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::shutdown( sock_base::shutdownflg dir )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    if ( (dir & (sock_base::stop_in | sock_base::stop_out)) ==
         (sock_base::stop_in | sock_base::stop_out) ) {
      ::shutdown( basic_socket_t::_fd, 2 );
    } else if ( dir & sock_base::stop_in ) {
      ::shutdown( basic_socket_t::_fd, 0 );
    } else if ( dir & sock_base::stop_out ) {
      ::shutdown( basic_socket_t::_fd, 1 );
    }
  }
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::setoptions_unsafe( sock_base::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  if ( basic_socket_t::is_open_unsafe() ) {
    if ( optname != sock_base::so_linger ) {
      int turn = on_off ? 1 : 0;
      if ( setsockopt( basic_socket_t::_fd, SOL_SOCKET, (int)optname, (const void *)&turn,
                       (socklen_t)sizeof(int) ) != 0 ) {
        _state |= ios_base::failbit;
      }
    } else {
      linger l;
      l.l_onoff = on_off ? 1 : 0;
      l.l_linger = __v;
      if ( setsockopt( basic_socket_t::_fd, SOL_SOCKET, (int)optname, (const void *)&l,
                       (socklen_t)sizeof(linger) ) != 0 ) {
        _state |= ios_base::failbit;
      }
      
    }
  } else {
    _state |= ios_base::failbit;
  }
#endif // __unix
}


template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
int connect_processor<Connect, charT, traits, _Alloc, C>::Init::_count = 0;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
bool connect_processor<Connect, charT, traits, _Alloc, C>::Init::_at_fork = false;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
std::tr2::mutex connect_processor<Connect, charT, traits, _Alloc, C>::Init::_init_lock;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::_guard( int direction )
{
  if ( direction ) {
    std::tr2::lock_guard<std::tr2::mutex> lk( _init_lock );
    if ( _count++ == 0 ) {
#ifdef __FIT_PTHREADS
      if ( !_at_fork ) { // call only once
        if ( pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child ) ) {
          // throw system_error
        }
        _at_fork = true;
      }
#endif
//       _sock_processor_base::_idx = std::tr2::this_thread::xalloc();
    }
  } else {
    std::tr2::lock_guard<std::tr2::mutex> lk( _init_lock );
    if ( --_count == 0 ) {

    }
  }
}

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_prepare()
{ _init_lock.lock(); }

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_child()
{
  _init_lock.unlock();

  if ( _count != 0 ) {
    // std::cerr << "SHOULD NEVER HAPPEN!!!!\n";
    throw std::logic_error( "Fork while connect_processor working may has unexpected behaviour in child process" );
  }
  // _sock_processor_base::_idx =  std::tr2::this_thread::xalloc();
}

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_parent()
{ _init_lock.unlock(); }

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
char connect_processor<Connect, charT, traits, _Alloc, C>::Init_buf[128];

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>                                                             
void connect_processor<Connect, charT, traits, _Alloc, C>::close()
{
  base_t::close();

  { 
    std::tr2::lock_guard<std::tr2::mutex> lk(inwlock);
    _in_work = false; // <--- set before cnd.notify_one(); (below in this func)
  }

  {
    std::tr2::lock_guard<std::tr2::mutex> lk2( rdlock );
    ready_pool.push_back( processor() ); // make ready_pool not empty
    // std::cerr << "=== " << ready_pool.size() << std::endl;
    cnd.notify_one();
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::sockbuf_t* connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( sock_base::socket_type fd, const sockaddr& addr )
{
  typename base_t::sockstream_t* s = base_t::create_stream( fd, addr );

  if ( s == 0 ) {
    return 0;
  }

  Connect* c = new Connect( *s ); // bad point! I can't read from s in ctor indeed!

  // if ( s->rdbuf()->in_avail() ) {
  //   std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
  //  ready_pool.push_back( processor( c, s ) );
  //   cnd.notify_one();
  // } else {
  std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
  worker_pool.insert( std::make_pair( fd, processor( c, s ) ) );
  // }

  return s->rdbuf();
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( sock_base::socket_type fd, const typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::adopt_close_t& )
{
  {
    std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
    typename worker_pool_t::iterator i = worker_pool.find( fd );
    if ( i != worker_pool.end() ) {
      delete i->second.c;
      delete i->second.s;
      // std::cerr << "oops\n";
      worker_pool.erase( i );
      return;
    }
  }

  processor p;
  {
    std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
    typename ready_pool_t::iterator j = std::find( ready_pool.begin(), ready_pool.end(), /* std::bind2nd( typename processor::equal_to(), &s ) */ fd );
    if ( j != ready_pool.end() ) {
      // std::cerr << "oops 2\n";
      p = *j;
      ready_pool.erase( j );
    }
  }
  if ( p.c != 0 ) {
    // (*p.c)( *p.s );
    delete p.c;
    delete p.s;
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( sock_base::socket_type fd )
{
  processor p;

  {
    std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
    typename worker_pool_t::const_iterator i = worker_pool.find( fd );
    if ( i == worker_pool.end() ) {
      return;
    }
    p = i->second;
    worker_pool.erase( i );
  }

  std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
  ready_pool.push_back( p );
  cnd.notify_one();
  // std::cerr << "notify data " << (void *)c << " " << ready_pool.size() << std::endl;
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
bool connect_processor<Connect, charT, traits, _Alloc, C>::pop_ready( processor& p )
{
  {
    std::tr2::unique_lock<std::tr2::mutex> lk( rdlock );

    cnd.wait( lk, not_empty );
    p = ready_pool.front(); // it may contain p.c == 0,  p.s == 0, if !in_work()
    ready_pool.pop_front();
    // std::cerr << "pop 1\n";
    if ( p.c == 0 ) { // wake up, but _in_work may be still true here (in processor pipe?),
      return false;   // even I know that _in_work <- false before notification...
    }                 // so, check twice
  }

  // std::cerr << "pop 2\n";

  std::tr2::lock_guard<std::tr2::mutex> lk(inwlock);
  return _in_work ? true : false;
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::worker()
{
  _in_work = true;

  processor p;

  while ( pop_ready( p ) ) {
    // std::cerr << "worker 1\n";
    (p.c->*C)( *p.s );
    // std::cerr << "worker 2\n";
    if ( p.s->rdbuf()->in_avail() ) {
      std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
      ready_pool.push_back( p );
    } else {
      std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
      worker_pool[p.s->rdbuf()->fd()] = p;
    }
    // std::cerr << "worker 3\n";
  }
}

} // namespace std
