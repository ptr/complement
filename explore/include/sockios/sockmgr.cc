// -*- C++ -*- Time-stamp: <09/02/04 11:15:27 ptr>

/*
 * Copyright (c) 2008, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

namespace std {

namespace detail {

template<class charT, class traits, class _Alloc>
sockmgr<charT,traits,_Alloc>::sockmgr( int hint, int ret ) :
    efd( -1 ),
    n_ret( ret ),
    _worker( 0 )
{
  pipefd[0] = -1;
  pipefd[1] = -1;

  efd = epoll_create( hint );
  if ( efd < 0 ) {
    // throw system_error( errno )
    throw std::runtime_error( "epoll_create" );
  }
  if ( pipe( pipefd ) < 0 ) { // check err
    ::close( efd );
    efd = -1;
    // throw system_error;
    throw std::runtime_error( "pipe" );
  }
  // cfd = pipefd[1];

  epoll_event ev_add;
  ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP;
#if 1
  ev_add.data.u64 = 0ULL;
#endif
  ev_add.data.fd = pipefd[0];
  epoll_ctl( efd, EPOLL_CTL_ADD, pipefd[0], &ev_add );

  _worker = new std::tr2::thread( _loop, this );

  // ctl _ctl;
  // _ctl.cmd = rqstart;

  // write( pipefd[1], &_ctl, sizeof(ctl) );
}

template<class charT, class traits, class _Alloc>
sockmgr<charT,traits,_Alloc>::~sockmgr()
{
  if ( _worker->joinable() ) {
    ctl _ctl;
    _ctl.cmd = rqstop;
    _ctl.data.ptr = 0;

    ::write( pipefd[1], &_ctl, sizeof(ctl) );

    // _worker->join();
  }

  delete _worker;
  _worker = 0;

  ::close( pipefd[1] );
  pipefd[1] = -1;
  ::close( pipefd[0] );
  pipefd[0] = -1;
  ::close( efd );
  efd = -1;
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::push( socks_processor_t& p )
{
  ctl _ctl;
  _ctl.cmd = listener;
  _ctl.data.ptr = static_cast<void *>(&p);

  p.addref();
  int r = 0;
  int ret = 0;
  do {
    ret = ::write( pipefd[1], &_ctl, sizeof(ctl) );
    if ( ret < 0 ) {
      switch ( errno ) {
        case EINTR:
        case EAGAIN:
          continue;
        default:
          p.release();
          throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>::push( socks_processor_t& p )" ) );
          break;
      }
    }
    r += ret;
  } while ( (r != sizeof(ctl)) /* || (ret != 0) */ );
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::push( sockbuf_t& s )
{
  ctl _ctl;
  _ctl.cmd = tcp_buffer;
  _ctl.data.ptr = static_cast<void *>(&s);

  errno = 0;

  int r = 0;
  int ret = 0;
  do {
    ret = ::write( pipefd[1], &_ctl, sizeof(ctl) );
    if ( ret < 0 ) {
      switch ( errno ) {
        case EINTR:
        case EAGAIN:
          continue;
        default:
          throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>::push( sockbuf_t& s )" ) );
          break;
      }
    }
    r += ret;
  } while ( (r != sizeof(ctl)) /* || (ret != 0) */ );
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::pop( socks_processor_t& p, sock_base::socket_type _fd )
{
  ctl _ctl;
  _ctl.cmd = listener_on_exit;
  _ctl.data.ptr = reinterpret_cast<void *>(&p);

  p.addref();
  int r = 0;
  int ret = 0;
  do {
    ret = ::write( pipefd[1], &_ctl, sizeof(ctl) );
    if ( ret < 0 ) {
      switch ( errno ) {
        case EINTR:
        case EAGAIN:
          continue;
        default:
          p.release();
          throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>::pop( socks_processor_t& p, sock_base::socket_type _fd )" ) );
          break;
      }
    }
    r += ret;
  } while ( (r != sizeof(ctl)) /* || (ret != 0) */ );
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::exit_notify( sockbuf_t* b, sock_base::socket_type fd )
{
  try {
    std::tr2::unique_lock<std::tr2::mutex> lk( dll, std::tr2::defer_lock );

    if ( lk.try_lock() ) {
      if ( b->_notify_close ) {
        typename fd_container_type::iterator i = descr.find( fd );
        if ( (i != descr.end()) && (i->second.b == b) && (i->second.p == 0) ) {
          if ( epoll_ctl( efd, EPOLL_CTL_DEL, fd, 0 ) < 0 ) {
            // std::cerr << __FILE__ << ":" << __LINE__ << " " << fd << std::endl;
            // throw system_error
          }
          descr.erase( i );
        }
        b->_notify_close = false;
      }
    }
  }
  catch ( const std::tr2::lock_error& ) {
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::io_worker()
{
  epoll_event ev[ n_ret ];

  memset( ev, 0, n_ret * sizeof(epoll_event) );
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
      int n = epoll_wait( efd, &ev[0], n_ret, -1 );

      if ( n < 0 ) {
        if ( errno == EINTR ) {
          errno = 0;
          continue;
        }
        // throw system_error
        std::cerr << __FILE__ << ":" << __LINE__ << " " << efd << " " << std::posix_error::make_error_code( static_cast<std::posix_error::posix_errno>(errno) ).message() << std::endl;
      }

      std::tr2::lock_guard<std::tr2::mutex> lk( dll );

      for ( int i = 0; i < n; ++i ) {
        if ( ev[i].data.fd == pipefd[0] ) {
          cmd_from_pipe();
        } else {
          typename fd_container_type::iterator ifd = descr.find( ev[i].data.fd );
          if ( ifd == descr.end() ) { // already closed
            // and should be already removed from efd's vector,
            // so no epoll_ctl( efd, EPOLL_CTL_DEL, ev[i].data.fd, 0 ) here
            continue;
          }

          fd_info& info = ifd->second;
          if ( info.flags & fd_info::listener ) {
            process_listener( ev[i], ifd );
          } else {
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
#if 0
  memset( &ev_add, 0, sizeof(epoll_event) );
#endif

  ctl _ctl;

  int r = 0;
  int ret = 0;
  do {
    ret = read( pipefd[0], &_ctl, sizeof(ctl) );
    if ( ret < 0 ) {
      switch ( errno ) {
        case EINTR:
        case EAGAIN:
          continue;
        default:
          throw std::detail::stop_request(); // runtime_error( "Stop request (normal flow)" );
      }
    } else if ( ret == 0 ) {
      throw runtime_error( "Read pipe return 0" );
    }
    r += ret;
  } while ( r != sizeof(ctl) );

  switch ( _ctl.cmd ) {
    case listener:
      ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
#if 1
      ev_add.data.u64 = 0ULL;
#endif
      ev_add.data.fd = static_cast<socks_processor_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        if ( fcntl( ev_add.data.fd, F_SETFL, fcntl( ev_add.data.fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
          static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
          throw std::runtime_error( "can't establish nonblock mode on listener" );
        }
        if ( descr.find( ev_add.data.fd ) != descr.end() ) { // reuse?
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev_add.data.fd, &ev_add ) < 0 ) {
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
            // descr.erase( ev_add.data.fd );
            static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
            return;
          }
        } else {
          if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
            static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
            return;
          }
        }
        descr[ev_add.data.fd] = fd_info( static_cast<socks_processor_t*>(_ctl.data.ptr) );
      }
      static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
      break;
    case tcp_buffer:
      ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
#if 1
      ev_add.data.u64 = 0ULL;
#endif
      ev_add.data.fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
          descr.erase( ev_add.data.fd );
          // std::cerr << __FILE__ << ":" << __LINE__ << " " << std::error_code( errno, std::posix_category ).message() << " " << ev_add.data.fd << " " << std::tr2::getpid() << std::endl;
          return; // already closed?
        }      
        descr[ev_add.data.fd] = fd_info( static_cast<sockbuf_t*>(_ctl.data.ptr) );
      }
      break;
    case listener_on_exit:
      {
        socks_processor_t* p = reinterpret_cast<socks_processor_t*>(_ctl.data.ptr);
        for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ) {
          if ( i->second.p == p ) {
            if ( (i->second.flags & fd_info::listener) == 0 ) { // it's not me!
              i->second.b->close();
              (*p)( i->first, typename socks_processor_t::adopt_close_t() );
              }
            /* i = */ descr.erase( i++ );
          } else {
            ++i;
          }
        }

        // no more connection with this listener
        p->stop();
        p->release(); // socks_processor_t* p not under sockmgr control more
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
  if ( ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR) ) {
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      // std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << " " << system_error( posix_error::make_error_code( static_cast<posix_error::posix_errno>(errno) ) ).what() << std::endl;
      std::cerr << __FILE__ << ":" << __LINE__ << " " << efd << " " << std::posix_error::make_error_code( static_cast<std::posix_error::posix_errno>(errno) ).message() << std::endl;

      // already closed?
    }

    descr.erase( ifd );

    return;
  }

  if ( (ev.events & EPOLLIN) == 0 ) {
    return; // I don't know what to do this case...
  }

  sockaddr addr;
  socklen_t sz = sizeof( sockaddr_in );

  fd_info info = ifd->second;

  for ( ; ; ) {
    int fd = accept( ev.data.fd, &addr, &sz );
    if ( fd < 0 ) {
      if ( (errno == EINTR) || (errno == ECONNABORTED) /* || (errno == ERESTARTSYS) */ ) {
        errno = 0;
        continue;
      }
      if ( errno == EAGAIN ) { // EWOULDBLOCK == EAGAIN
        // back to listen
        errno = 0;
        epoll_event xev;
        xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
#if 1
        xev.data.u64 = 0ULL;
#endif
        xev.data.fd = ev.data.fd;
        if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) == 0 ) {
          return; // normal flow, back to epoll
        }
        // closed?
        std::cerr << __FILE__ << ":" << __LINE__ << " " << ev.data.fd << " "
                  << errno << std::endl;
      }

      if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
        // ignore error, may be closed
        // std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << " "
        //           << errno << std::endl;
        // throw system_error
      }

      // ifd removed from descr in cmd_from_pipe, see listener_on_exit label

      return;
    }

    if ( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      throw std::runtime_error( "can't establish nonblock mode" );
    }
      
    try {
      epoll_event ev_add;
      ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
#if 1
      ev_add.data.u64 = 0ULL;
#endif
      ev_add.data.fd = fd;

      if ( epoll_ctl( efd, EPOLL_CTL_ADD, fd, &ev_add ) < 0 ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " " << std::error_code( errno, std::posix_category ).message() << " " << fd << " " << std::tr2::getpid() << std::endl;
        // throw system_error
        return;
      }      

      sockbuf_t* b = (*info.p)( fd, addr );

      descr[fd] = fd_info( b, info.p );
    }
    catch ( const std::bad_alloc& ) {
      // nothing
      descr.erase( fd );
    }
    catch ( ... ) {
      descr.erase( fd );
    }
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_regular( epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  fd_info& info = ifd->second;

  sockbuf_t* b = info.b;
  if ( b == 0 ) { // marginal case: sockbuf wasn't created by processor...
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << " " << errno << std::endl;
      // throw system_error
    }
    if ( info.p != 0 ) { // ... but controlled by processor
      (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
    }
    descr.erase( ifd );
    return;
  }

  errno = 0;

  try {
    if ( ev.events & EPOLLIN ) {
      long offset;
      for ( ; ; ) {
        if ( b->_ebuf == b->egptr() ) {
          // process extract data from buffer too slow for us!
          if ( (info.flags & fd_info::level_triggered) == 0 ) {
            epoll_event xev;
#if 1
            xev.data.u64 = 0ULL;
#endif
            xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP;
            xev.data.fd = ev.data.fd;
            info.flags |= fd_info::level_triggered;
            if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
              throw fdclose(); // already closed?
            }
          }
          if ( info.p != 0 ) {
            (*info.p)( ev.data.fd );
          }
          break;
        }

        offset = read( ev.data.fd, b->egptr(), sizeof(charT) * (b->_ebuf - b->egptr()) );

        if ( offset == 0 ) {
          // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
          epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ); // ignore possible error
          throw fdclose();
        }

        if ( offset < 0 ) {
          if ( errno == EINTR ) { // read was interrupted
            errno = 0;
            continue;
          }
          if ( errno == EAGAIN ) { // EWOULDBLOCK
            errno = 0;
            epoll_event xev;
            xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
#if 1
            xev.data.u64 = 0ULL;
#endif
            xev.data.fd = ev.data.fd;
            if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) == 0 ) {
              break; // normal flow, back to epoll
            }
            // already closed?
          } else {
            epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ); // ignore possible error
          }
          // EBADF (already closed?), EFAULT (Bad address),
          // ECONNRESET (Connection reset by peer), ...
          // errno = 0;
          throw fdclose();
        }

        offset /= sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!
            
        if ( info.flags & fd_info::level_triggered ) {
          epoll_event xev;
          xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
#if 1
          xev.data.u64 = 0ULL;
#endif
          xev.data.fd = ev.data.fd;
          info.flags &= ~static_cast<unsigned>(fd_info::level_triggered);
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
            // std::cerr << __FILE__ << ":" << __LINE__ << " " << std::error_code( errno, std::posix_category ).message() << " " << ev.data.fd << " " << std::tr2::getpid() << std::endl;
            throw fdclose(); // already closed?
          }
        }
        {
          std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
          b->setg( b->eback(), b->gptr(), b->egptr() + offset );
          b->ucnd.notify_one();
          if ( info.p != 0 ) {
            (*info.p)( ev.data.fd );
          }
        }
      }
    }

    if ( (ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR) ) != 0 ) {
      epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ); // ignore possible error
      throw fdclose();
    }
    // if ( ev.events & EPOLLHUP ) {
    //   std::cerr << "Poll HUP" << std::endl;
    // }
    // if ( ev.events & EPOLLERR ) {
    //   std::cerr << "Poll ERR" << std::endl;
    // }
#if 0
    // Never see here: raw socket?
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
#endif
  }
  catch ( const fdclose& ) {
    errno = 0;

    if ( info.p != 0 ) {
      b->close();
      (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );

      descr.erase( ifd );
    } else {
      b->_notify_close = false; // avoid deadlock
      descr.erase( ifd );
      b->close();
    }
    // dump_descr();
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::dump_descr()
{
  for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
    std::cerr << i->first << " "
              << std::hex
              << i->second.flags
              << " "
              << (void*)i->second.b
              << " "
              << (void*)i->second.p
              << std::dec
              << endl;
  }
}


} // namespace detail

} // namespace std
