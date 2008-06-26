// -*- C++ -*- Time-stamp: <08/06/19 21:28:42 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

namespace std {

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
          errno = 0;
          continue;
        }
        // throw system_error
      }
      // std::cerr << "epoll see " << n << std::endl;

      std::tr2::lock_guard<std::tr2::mutex> lk( dll );

      for ( int i = 0; i < n; ++i ) {
        // std::cerr << "epoll i = " << i << std::endl;
        std::tr2::lock_guard<std::tr2::mutex> lck( cll );
        for ( typename fd_container_type::iterator closed_ifd = closed_queue.begin(); closed_ifd != closed_queue.end(); ++closed_ifd ) {
          if ( epoll_ctl( efd, EPOLL_CTL_DEL, closed_ifd->first, 0 ) < 0 ) {
            // ignore
          }
          if ( closed_ifd->first == ev[i].data.fd ) {
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
          }
          // descr.erase( closed_ifd->first );    
        }
        closed_queue.clear();
        // at this point closed queue empty

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
    case tcp_buffer:
      ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        fd_info new_info = { 0, static_cast<sockbuf_t*>(_ctl.data.ptr), 0 };
        descr[ev_add.data.fd] = new_info;
        if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
          descr.erase( ev_add.data.fd );
          // throw system_error
        }
      }
      break;
    case listener_on_exit:
      listeners_final.insert( _ctl.data.ptr );
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      {
        int lfd = check_closed_listener( reinterpret_cast<socks_processor_t*>(_ctl.data.ptr) );
        if ( lfd != -1 ) {
          descr.erase( lfd );
        }
        dump_descr();
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
  std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
  if ( ev.events & EPOLLRDHUP ) {
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      // throw system_error
    }

    if ( ifd->second.p != 0 ) {
      ifd->second.p->close();
      for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
        if ( (i->second.p == ifd->second.p) && (i->second.b != 0) ) {
          i->second.b->shutdown( sock_base::stop_in | sock_base::stop_out );
        }
      }
    }

    listeners_final.insert(static_cast<void *>(ifd->second.p));

    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

    socks_processor_t* p = ifd->second.p;

    descr.erase( ifd );

    int lfd = check_closed_listener( p );
    
    if ( lfd != -1 ) {
      descr.erase( lfd );
    }

    dump_descr();

    return;
  }

  if ( (ev.events & EPOLLIN) == 0 ) {
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    return; // I don't know what to do this case...
  }

  sockaddr addr;
  socklen_t sz = sizeof( sockaddr_in );

  fd_info info = ifd->second;

  for ( ; ; ) {
    int fd = accept( ev.data.fd, &addr, &sz );
    if ( fd < 0 ) {
      // std::cerr << "Accept, listener # " << ev.data.fd << ", errno " << errno << std::endl;
      std::cerr << __FILE__ << ":" << __LINE__ /* << " " << std::tr2::getpid() */ << std::endl;
      if ( (errno == EINTR) || (errno == ECONNABORTED) /* || (errno == ERESTARTSYS) */ ) {
        std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        errno = 0;
        continue;
      }
      if ( !(errno == EAGAIN /* || errno == EWOULDBLOCK */ ) ) { // EWOULDBLOCK == EAGAIN
        // std::cerr << "Accept, listener " << ev.data.fd << ", errno " << errno << std::endl;
        std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
          std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
          // throw system_error
        }

        if ( ifd->second.p != 0 ) {
          ifd->second.p->close();
          for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
            if ( (i->second.p == ifd->second.p) && (i->second.b != 0) ) {
              i->second.b->shutdown( sock_base::stop_in | sock_base::stop_out );
            }
          }
        }

        std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

        socks_processor_t* p = ifd->second.p;
        listeners_final.insert( static_cast<void *>(p) );

        descr.erase( ifd );

        check_closed_listener( p );

        std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        dump_descr();
      } else { // back to listen
        errno = 0;
        epoll_event xev;
        xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
        xev.data.fd = ev.data.fd;
        if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
          std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        }
      }
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      return;
    }
    if ( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      throw std::runtime_error( "can't establish nonblock mode" );
    }
      
    try {
      epoll_event ev_add;
      ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.fd = fd;

      if ( epoll_ctl( efd, EPOLL_CTL_ADD, fd, &ev_add ) < 0 ) {
        descr.erase( fd );
        // throw system_error
        std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        return; // throw
      }

      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      sockbuf_t* b = (*info.p)( fd, addr );
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

      fd_info new_info = { 0, b, info.p };
      descr[fd] = new_info;
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
  std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

  fd_info& info = ifd->second;

  sockbuf_t* b = info.b;
  if ( b == 0 ) { // marginal case: sockbuf wasn't created by processor...
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      // throw system_error
    }
    if ( info.p != 0 ) { // ... but controlled by processor
      (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );

      socks_processor_t* p = info.p;
      descr.erase( ifd );
      int lfd = check_closed_listener( p );
      if ( lfd != -1 ) {
        descr.erase( lfd );
      }
    } else {
      descr.erase( ifd );
    }
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    dump_descr();
    return;
  }

  errno = 0;

  std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
  if ( ev.events & EPOLLIN ) {
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    for ( ; ; ) {
      if ( b->_ebuf == b->egptr() ) {
        std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        // process extract data from buffer too slow for us!
        if ( (info.flags & fd_info::level_triggered) == 0 ) {
          epoll_event xev;
          xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
          xev.data.fd = ev.data.fd;
          info.flags |= fd_info::level_triggered;
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
            // std::cerr << "X " << ev.data.fd << ", " << errno << std::endl;
          }
        }
        // std::cerr << "Z " << ev.data.fd << ", " << errno << std::endl;
        if ( info.p != 0 ) { // or (info.flags & fd_info::owner) != 0
          (*info.p)( ev.data.fd );
        }
        break;
      }
      // std::cerr << "ptr " <<  (void *)b->egptr() << ", " << errno << std::endl;
      long offset = read( ev.data.fd, b->egptr(), sizeof(charT) * (b->_ebuf - b->egptr()) );
      // std::cerr << "offset " << offset << ", " << errno << std::endl;
      if ( offset < 0 ) {
        std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        switch ( errno ) {
          case EINTR:      // read was interrupted
            errno = 0;
            continue;
            break;
          case EFAULT:     // Bad address
          case ECONNRESET: // Connection reset by peer
            errno = 0;
            ev.events |= EPOLLRDHUP; // will be processed below
            break;
          case EAGAIN:
            // case EWOULDBLOCK:
            errno = 0;
            {
              epoll_event xev;
              xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
              xev.data.fd = ev.data.fd;
              epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev );
            }
            break;
          default:
            // std::cerr << "not listener, other " << ev.data.fd << std::hex << ev.events << std::dec << " : " << errno << std::endl;
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
            break;
        }
        break;
      } else if ( offset > 0 ) {
        std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        offset /= sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!
            
        if ( info.flags & fd_info::level_triggered ) {
          epoll_event xev;
          xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
          xev.data.fd = ev.data.fd;
          info.flags &= ~static_cast<unsigned>(fd_info::level_triggered);
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
            // std::cerr << "Y " << ev.data.fd << ", " << errno << std::endl;
          }
        }
        std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
        b->setg( b->eback(), b->gptr(), b->egptr() + offset );
        b->ucnd.notify_one();
        if ( info.p != 0 ) {
          std::cerr << __FILE__ << ":" << __LINE__ << " " << ev.data.fd << std::endl;
          (*info.p)( ev.data.fd );
        }
      } else {
        std::cerr << __FILE__ << ":" << __LINE__ << " " << ev.data.fd << std::endl;
        // std::cerr << "K " << ev.data.fd << ", " << errno << std::endl;
        // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
        ev.events |= EPOLLRDHUP; // will be processed below
        break;
      }
    }
  }

  if ( (ev.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR) ) != 0 ) {
    // std::cerr << "Poll EPOLLRDHUP " << ev.data.fd << ", " << errno << std::endl;
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      // throw system_error
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    }

    if ( info.p != 0 ) {
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

      socks_processor_t* p = info.p;

      closed_queue.erase( ev.data.fd );
      descr.erase( ifd );

      int lfd = check_closed_listener( p );
      if ( lfd != -1 ) {
        descr.erase( lfd );
      }
    } else {
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      b->close();
      closed_queue.erase( ev.data.fd );
      descr.erase( ifd );
    }
    dump_descr();
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
#if 0
  std::tr2::lock_guard<std::tr2::mutex> lk_descr( dll );

  for ( typename fd_container_type::iterator ifd = descr.begin(); ifd != descr.end(); ) {
    if ( (ifd->second.flags & fd_info::owner) && (ifd->second.p == &p) ) {
      std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)&p << " " << (void*)ifd->second.b << std::endl;
      p( ifd->first, typename socks_processor_t::adopt_close_t() );
      std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)&p << " " << (void*)ifd->second.b << std::endl;
      if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
        // throw system_error
      }
      descr.erase( ifd++ );
    } else {
      ++ifd;
    }
  }

  std::tr2::lock_guard<std::tr2::mutex> lk( cll );

  // I can't use closed_queue.erase( p.fd() ) here: fd is -1 already
  for ( typename fd_container_type::iterator closed_ifd = closed_queue.begin(); closed_ifd != closed_queue.end(); ) {
    if ( closed_ifd->second.p == &p ) {
      closed_queue.erase( closed_ifd++ );
    } else {
      ++closed_ifd;
    }
  }
#endif
}

template<class charT, class traits, class _Alloc>
int sockmgr<charT,traits,_Alloc>::check_closed_listener( socks_processor_t* p )
{
  int myfd = -1;

  if ( !listeners_final.empty() ) {
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    if ( listeners_final.find( static_cast<void*>(p) ) != listeners_final.end() ) {
      for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
        if ( i->second.p == p ) {
          if ( (i->second.flags & fd_info::listener) == 0 ) { // it's not me!
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
            return -1;
          }
          myfd = i->first;
          std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        }
      }
      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

      // no more connection with this listener
      listeners_final.erase( static_cast<void*>(p) );

      std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

      p->stop();
    }
  }

  return myfd;
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::dump_descr()
{
  for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
    std::cerr << i->first
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
