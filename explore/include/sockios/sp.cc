// -*- C++ -*- Time-stamp: <08/06/05 12:55:59 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

namespace std {

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( const in_addr& addr, int port, sock_base2::stype type, sock_base2::protocol prot )
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
  if ( prot == sock_base2::inet ) {
    basic_socket_t::_fd = socket( PF_INET, type, 0 );
    if ( basic_socket_t::_fd == -1 ) {
      _state |= ios_base::failbit | ios_base::badbit;
      return;
    }
    // _open = true;
    basic_socket_t::_address.inet.sin_family = AF_INET;
    basic_socket_t::_address.inet.sin_port = htons( port );
    basic_socket_t::_address.inet.sin_addr.s_addr = addr.s_addr;

    if ( type == sock_base2::sock_stream || type == sock_base2::sock_seqpacket ) {
      // let's try reuse local address
      setoptions_unsafe( sock_base2::so_reuseaddr, true );
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

    if ( type == sock_base2::sock_stream || type == sock_base2::sock_seqpacket ) {
      // I am shure, this is socket of type SOCK_STREAM | SOCK_SEQPACKET,
      // so don't check return code from listen
      ::listen( basic_socket_t::_fd, SOMAXCONN );
      basic_socket_t::mgr->push( *this );
    }
  } else if ( prot == sock_base2::local ) {
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
void sock_processor_base<charT,traits,_Alloc>::shutdown( sock_base2::shutdownflg dir )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    if ( (dir & (sock_base2::stop_in | sock_base2::stop_out)) ==
         (sock_base2::stop_in | sock_base2::stop_out) ) {
      ::shutdown( basic_socket_t::_fd, 2 );
    } else if ( dir & sock_base2::stop_in ) {
      ::shutdown( basic_socket_t::_fd, 0 );
    } else if ( dir & sock_base2::stop_out ) {
      ::shutdown( basic_socket_t::_fd, 1 );
    }
  }
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::setoptions_unsafe( sock_base2::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  if ( basic_socket_t::is_open_unsafe() ) {
    if ( optname != sock_base2::so_linger ) {
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


template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
int connect_processor<Connect, charT, traits, _Alloc, C>::Init::_count = 0;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
bool connect_processor<Connect, charT, traits, _Alloc, C>::Init::_at_fork = false;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
std::tr2::mutex connect_processor<Connect, charT, traits, _Alloc, C>::Init::_init_lock;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
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

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_prepare()
{ _init_lock.lock(); }

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_child()
{
  _init_lock.unlock();

  if ( _count != 0 ) {
    // std::cerr << "SHOULD NEVER HAPPEN!!!!\n";
    throw std::logic_error( "Fork while connect_processor working may has unexpected behaviour in child process" );
  }
  // _sock_processor_base::_idx =  std::tr2::this_thread::xalloc();
}

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_parent()
{ _init_lock.unlock(); }

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
char connect_processor<Connect, charT, traits, _Alloc, C>::Init_buf[128];

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>                                                             
void connect_processor<Connect, charT, traits, _Alloc, C>::close()
{
  base_t::close();

  { 
    std::tr2::lock_guard<std::tr2::mutex> lk(inwlock);
    _in_work = false; // <--- set before cnd.notify_one(); (below in this func)
  }

  std::tr2::lock_guard<std::tr2::mutex> lk2( rdlock );
  ready_pool.push_back( processor() ); // make ready_pool not empty
  // std::cerr << "=== " << ready_pool.size() << std::endl;
  cnd.notify_one();
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( int fd, const sockaddr& addr )
{
  typename base_t::sockstream_t* s = base_t::create_stream( fd, addr );

  Connect* c = new Connect( *s ); // bad point! I can't read from s in ctor indeed!

  if ( s->rdbuf()->in_avail() ) {
    std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
    ready_pool.push_back( processor( c, s ) );
    cnd.notify_one();
  } else {
    std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
    worker_pool.insert( std::make_pair( fd, processor( c, s ) ) );
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( int fd, const typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::adopt_close_t& )
{
  {
    std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
    typename worker_pool_t::iterator i = worker_pool.find( fd );
    if ( i != worker_pool.end() ) {
      delete i->second.s;
      delete i->second.c;
      // std::cerr << "oops\n";
      worker_pool.erase( i );
      return;
    }
  }

  Connect* c = 0;
  {
    std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
    typename ready_pool_t::iterator j = std::find( ready_pool.begin(), ready_pool.end(), /* std::bind2nd( typename processor::equal_to(), &s ) */ fd );
    if ( j != ready_pool.end() ) {
      // std::cerr << "oops 2\n";
      c = j->c;
      ready_pool.erase( j );
    }
  }
  if ( c != 0 ) {
//using unknown variable s  
//    (c->*C)( s );
    delete c;
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( int fd, const typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::adopt_data_t& )
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

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
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


template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
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

namespace detail {
template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::io_worker()
{
  epoll_event ev[/*n_ret*/ 512 ];

/*
  ctl _xctl;
  int r = read( pipefd[0], &_xctl, sizeof(ctl) );

  if ( _xctl.cmd == rqstart ) {
    std::cerr << "io_worker fine" << std::endl;
  } else {
    std::cerr << "io_worker not fine, " << r << ", " << errno << std::endl;
  }
*/

  try {
    for ( ; ; ) {
      int n = epoll_wait( efd, &ev[0], /* n_ret */ 512, -1 );

      if ( n < 0 ) {
        if ( errno == EINTR ) {
          continue;
        }
        // throw system_error
      }
      // std::cerr << "epoll see " << n << std::endl;

      std::tr2::lock_guard<std::tr2::mutex> lk( dll );

      for ( int i = 0; i < n; ++i ) {
        // std::cerr << "epoll i = " << i << std::endl;
        if ( ev[i].data.fd == pipefd[0] ) {
          // std::cerr << "on pipe\n";
          cmd_from_pipe();
        } else {
          // std::cerr << "#\n";

          typename fd_container_type::iterator ifd = descr.find( ev[i].data.fd );
          if ( ifd == descr.end() ) {
            if ( epoll_ctl( efd, EPOLL_CTL_DEL, ev[i].data.fd, 0 ) < 0 ) {
              // throw system_error
            }
            continue;
            // throw std::logic_error( "file descriptor in epoll, but not in descr[]" );
          }

          fd_info& info = ifd->second;
          if ( info.flags & fd_info::listener ) {
            // std::cerr << "%\n";
            process_listener( ev[i], ifd );
          } else {
            // std::cerr << "not listener\n";
            process_regular( ev[i], ifd );
          }
        }
      }
    }
  }
  catch ( std::detail::stop_request& ) {
    // this is possible, normal flow of operation
  }
  catch ( std::exception& e ) {
    std::cerr << e.what() << std::endl;
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::cmd_from_pipe()
{
  epoll_event ev_add;
  ctl _ctl;

  int r = read( pipefd[0], &_ctl, sizeof(ctl) );
  if ( r < 0 ) {
    // throw system_error
    // std::cerr << "Read pipe\n";
  } else if ( r == 0 ) {
    // std::cerr << "Read pipe 0\n";
    throw runtime_error( "Read pipe return 0" );
  }

  switch ( _ctl.cmd ) {
    case listener:
      ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.fd = static_cast<socks_processor_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        if ( fcntl( ev_add.data.fd, F_SETFL, fcntl( ev_add.data.fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
          // std::cerr << "xxx " << errno << " " << std::tr2::getpid() << std::endl;
          throw std::runtime_error( "can't establish nonblock mode on listener" );
        }
        fd_info new_info = { fd_info::listener, 0, static_cast<socks_processor_t*>(_ctl.data.ptr) };
        descr[ev_add.data.fd] = new_info;
        if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
          descr.erase( ev_add.data.fd );
          // throw system_error
        }
      }
      break;
#if 0
    case tcp_stream:
      ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.fd = static_cast<sockstream_t*>(_ctl.data.ptr)->rdbuf()->fd();
      if ( ev_add.data.fd >= 0 ) {
        fd_info new_info = { 0, static_cast<sockstream_t*>(_ctl.data.ptr), 0 };
        descr[ev_add.data.fd] = new_info;
        if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
          descr.erase( ev_add.data.fd );
          // throw system_error
        }
      }
      break;
#endif
    case tcp_buffer:
      ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        fd_info new_info = { fd_info::buffer, static_cast<sockstream_t* /* sockbuf_t* */ >(_ctl.data.ptr), 0 };
        descr[ev_add.data.fd] = new_info;
        if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
          descr.erase( ev_add.data.fd );
          // throw system_error
        }
      }
      break;
    case rqstop:
      // std::cerr << "Stop request\n";
      throw std::detail::stop_request(); // runtime_error( "Stop request (normal flow)" );
      // break;
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_listener( epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  if ( ev.events & EPOLLRDHUP ) {
    epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 );
    // walk through descr and detach every .p ?
    descr.erase( ifd );
    std::cerr << "Remove listener EPOLLRDHUP\n";
  } else if ( ev.events & EPOLLIN ) {
    sockaddr addr;
    socklen_t sz = sizeof( sockaddr_in );

    fd_info info = ifd->second;

    for ( ; ; ) {
      int fd = accept( ev.data.fd, &addr, &sz );
      if ( fd < 0 ) {
        std::cerr << "Accept, listener # " << ev.data.fd << ", errno " << errno << std::endl;
        std::cerr << __FILE__ << ":" << __LINE__ << " " << std::tr2::getpid() << std::endl;
        if ( (errno == EINTR) || (errno == ECONNABORTED) /* || (errno == ERESTARTSYS) */ ) {
          continue;
        }
        if ( !(errno == EAGAIN || errno == EWOULDBLOCK) ) {
          // std::cerr << "Accept, listener " << ev[i].data.fd << ", errno " << errno << std::endl;
          // throw system_error ?
        }
#if 0
        {
          std::tr2::lock_guard<std::tr2::mutex> lck( cll );
          typename fd_container_type::iterator closed_ifd = closed_queue.find( ev.data.fd );
          if ( closed_ifd != closed_queue.end() ) {
            typename fd_container_type::iterator ifd = descr.begin();
            for ( ; ifd != descr.end(); ) {
              if ( ifd->second.p == closed_ifd->second.p ) {
                descr.erase( ifd++ );
              } else {
                ++ifd;
              }
            }
            closed_queue.erase( closed_ifd );
          }
        }
#endif
        break;
      }
      // std::cerr << "listener accept " << fd << std::endl;
      if ( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
        throw std::runtime_error( "can't establish nonblock mode" );
      }
      
      try {
        std::cerr << __FILE__ << ":" << __LINE__ << " new sockstream_t" << std::endl;
        (*info.p)( fd, addr );

        epoll_event ev_add;
        ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
        ev_add.data.fd = fd;
//undeclared s
//        fd_info new_info = { fd_info::owner, s, info.p };
        fd_info new_info = { fd_info::owner, 0, info.p };
        descr[fd] = new_info;

        if ( epoll_ctl( efd, EPOLL_CTL_ADD, fd, &ev_add ) < 0 ) {
          std::cerr << "Accept, add " << fd << ", errno " << errno << std::endl;
          descr.erase( fd );
          // throw system_error
        }

        std::cerr << __FILE__ << ":" << __LINE__ << " adopt_new_t()\n";
        bool in_closed = false;
        {
          std::tr2::lock_guard<std::tr2::mutex> lk( cll );
          typename fd_container_type::iterator closed_ifd = closed_queue.begin();
          for ( ; closed_ifd != closed_queue.end(); ++closed_ifd ) {
            if ( closed_ifd->second.p == info.p ) {
              in_closed = true;
              std::cerr << "@@@ 1\n" << std::endl;
              break;
            }
          }
        }
      }
      catch ( const std::bad_alloc& ) {
        // nothing
      }
      catch ( ... ) {
        descr.erase( fd );
      }
    }
  } else {
    // std::cerr << "listener: " << std::hex << ev.events << std::dec << std::endl;
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_regular( epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  fd_info& info = ifd->second;

  if ( ev.events & EPOLLIN ) {
    if ( (info.flags & fd_info::owner) == 0 ) {
      // marginal case: me not owner (registerd via push(),
      // when I owner, I know destroy point),
      // already closed, but I not see closed event yet;
      // object may be deleted already, so I can't
      // call b->egptr() etc. here
      std::tr2::lock_guard<std::tr2::mutex> lck( cll );
      typename fd_container_type::iterator closed_ifd = closed_queue.find( ev.data.fd );
      if ( closed_ifd != closed_queue.end() ) {
        closed_queue.erase( closed_ifd );
        if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
          // throw system_error
        }
        descr.erase( ifd );
        return;
      }
    }
    sockbuf_t* b = (info.flags & fd_info::buffer != 0) ? info.s.b : info.s.s->rdbuf();
    errno = 0;
    for ( ; ; ) {
      if ( b->_ebuf == b->egptr() ) {
        // process extract data from buffer too slow for us!
        if ( (info.flags & fd_info::level_triggered) == 0 ) {
          epoll_event xev;
          xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
          xev.data.fd = ev.data.fd;
          info.flags |= fd_info::level_triggered;
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
            std::cerr << "X " << ev.data.fd << ", " << errno << std::endl;
          }
        }
        std::cerr << "Z " << ev.data.fd << ", " << errno << std::endl;
        if ( info.p != 0 ) {
          bool is_closed = false;
          {
            std::tr2::lock_guard<std::tr2::mutex> lk( cll );
            typename fd_container_type::iterator closed_ifd = closed_queue.begin();
            for ( ; closed_ifd != closed_queue.end(); ++closed_ifd ) {
              if ( closed_ifd->second.p == info.p ) {
                is_closed = true;
                std::cerr << "@@@ 2\n" << std::endl;
                break;
              }
            }
          }
          if ( !is_closed ) {
            (*info.p)( ev.data.fd, typename socks_processor_t::adopt_data_t() );
          }
        }
        break;
      }
      // std::cerr << "ptr " <<  (void *)b->egptr() << ", " << errno << std::endl;
      long offset = read( ev.data.fd, b->egptr(), sizeof(charT) * (b->_ebuf - b->egptr()) );
      // std::cerr << "offset " << offset << ", " << errno << std::endl;
      if ( offset < 0 ) {
        if ( (errno == EAGAIN) || (errno == EINTR) ) {
          errno = 0;
          epoll_event xev;
          xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
          xev.data.fd = ev.data.fd;
          epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev );
          break;
        } else {
          switch ( errno ) {
            // case EINTR:      // read was interrupted
            // continue;
            //  break;
            case EFAULT:     // Bad address
            case ECONNRESET: // Connection reset by peer
              ev.events |= EPOLLRDHUP; // will be processed below
              break;
            default:
              // std::cerr << "not listener, other " << ev.data.fd << std::hex << ev.events << std::dec << " : " << errno << std::endl;
              break;
          }
          break;
        }
      } else if ( offset > 0 ) {
        offset /= sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!
            
        if ( info.flags & fd_info::level_triggered ) {
          epoll_event xev;
          xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
          xev.data.fd = ev.data.fd;
          info.flags &= ~static_cast<unsigned>(fd_info::level_triggered);
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
            std::cerr << "Y " << ev.data.fd << ", " << errno << std::endl;
          }
        }
        std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
        b->setg( b->eback(), b->gptr(), b->egptr() + offset );
        b->ucnd.notify_one();
        if ( info.p != 0 ) {
          // std::cerr << "data here" << std::endl;
          bool is_closed = false;
          {
            std::tr2::lock_guard<std::tr2::mutex> lk( cll );
            typename fd_container_type::iterator closed_ifd = closed_queue.begin();
            for ( ; closed_ifd != closed_queue.end(); ++closed_ifd ) {
              if ( closed_ifd->second.p == info.p ) {
                is_closed = true;
                std::cerr << "@@@ 3\n" << std::endl;
                break;
              }
            }
          }
          if ( !is_closed ) {
            (*info.p)( ev.data.fd, typename socks_processor_t::adopt_data_t() );
          }
        }
      } else {
        std::cerr << "K " << ev.data.fd << ", " << errno << std::endl;
        // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
        ev.events |= EPOLLRDHUP; // will be processed below
        break;
      }
    }
  } else {
    std::cerr << "Q\n";
  }

  if ( (ev.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR) ) != 0 ) {
    // std::cerr << "Poll EPOLLRDHUP " << ev.data.fd << ", " << errno << std::endl;
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      // throw system_error
    }
    bool need_delete = true;
    if ( info.p != 0 ) {
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( cll );
        typename fd_container_type::iterator closed_ifd = closed_queue.begin();
        for ( ; closed_ifd != closed_queue.end(); ++closed_ifd ) {
          if ( closed_ifd->second.p == info.p ) {
            need_delete = false; // will be deleted in 'final' method
            std::cerr << "@@@ 4\n" << std::endl;
            break;
          }
        }
      }
      if ( need_delete ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)info.s.s << std::endl;
        (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
        std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)info.s.s << std::endl;
      }
    }
    if ( (info.flags & fd_info::owner) != 0 && need_delete ) {
      std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)info.s.s << std::endl;
      delete info.s.s; // Ahtung!
      info.s.s = 0;
    } else {
      std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)info.s.s << std::endl;
      if ( (info.flags & fd_info::buffer) != 0 ) {
        info.s.b->close();
      } else {
        info.s.s->close();
      }
      std::tr2::lock_guard<std::tr2::mutex> lck( cll );
      closed_queue.erase( ev.data.fd );
    }
    descr.erase( ifd );
  }
  // if ( ev.events & EPOLLHUP ) {
  //   std::cerr << "Poll HUP" << std::endl;
  // }
  // if ( ev.events & EPOLLERR ) {
  //   std::cerr << "Poll ERR" << std::endl;
  // }
  if ( ev.events & EPOLLPRI ) {
    std::cerr << "Poll PRI" << std::endl;
  }
  if ( ev.events & EPOLLRDNORM ) {
    std::cerr << "Poll RDNORM" << std::endl;
  }
  if ( ev.events & EPOLLRDBAND ) {
    std::cerr << "Poll RDBAND" << std::endl;
  }
  if ( ev.events & EPOLLMSG ) {
    std::cerr << "Poll MSG" << std::endl;
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::final( sockmgr<charT,traits,_Alloc>::socks_processor_t& p )
{
  std::tr2::lock_guard<std::tr2::mutex> lk_descr( dll );

  for ( typename fd_container_type::iterator ifd = descr.begin(); ifd != descr.end(); ) {
    if ( (ifd->second.flags & fd_info::owner) && (ifd->second.p == &p) ) {
      std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)&p << " " << (void*)ifd->second.s.s << std::endl;
      p( ifd->first, typename socks_processor_t::adopt_close_t() );
      std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)&p << " " << (void*)ifd->second.s.s << std::endl;
      delete ifd->second.s.s;
      if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
        // throw system_error
      }
      descr.erase( ifd++ );
    } else {
      ++ifd;
    }
  }

  std::tr2::lock_guard<std::tr2::mutex> lk( cll );

  // I can't use  closed_queue.erase( p.fd() ) here: fd is -1 already
  for ( typename fd_container_type::iterator closed_ifd = closed_queue.begin(); closed_ifd != closed_queue.end(); ++closed_ifd ) {
    if ( closed_ifd->second.p == &p ) {
      closed_queue.erase( closed_ifd );
      break;
    }
  }
}

} // namespace detail

} // namespace std
