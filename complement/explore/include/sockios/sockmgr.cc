// -*- C++ -*- Time-stamp: <08/07/01 10:16:39 ptr>

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

      std::tr2::lock_guard<std::tr2::mutex> lk( dll );

      for ( int i = 0; i < n; ++i ) {
        if ( ev[i].data.fd == pipefd[0] ) {
          cmd_from_pipe();
        } else {
          typename fd_container_type::iterator ifd = descr.find( ev[i].data.fd );
          if ( ifd == descr.end() ) {
            // std::cerr << __FILE__ << ":" << __LINE__ << " " << ev[i].data.fd << std::endl;
            if ( epoll_ctl( efd, EPOLL_CTL_DEL, ev[i].data.fd, 0 ) < 0 ) {
              // throw system_error
            }
            continue;
            // throw std::logic_error( "file descriptor in epoll, but not in descr[]" );
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
  ctl _ctl;

  int r = read( pipefd[0], &_ctl, sizeof(ctl) );
  if ( r < 0 ) {
    // throw system_error
    throw std::detail::stop_request(); // runtime_error( "Stop request (normal flow)" );
  } else if ( r == 0 ) {
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
        if ( descr.find( ev_add.data.fd ) != descr.end() ) { // reuse?
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev_add.data.fd, &ev_add ) < 0 ) {
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
            // descr.erase( ev_add.data.fd );
            // throw system_error
            return;
          }
        } else {
          // std::cerr << __FILE__ << ":" << __LINE__ << " " << ev_add.data.fd << std::endl;
          if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
            // throw system_error
            return;
          }
        }
        descr[ev_add.data.fd] = fd_info( static_cast<socks_processor_t*>(_ctl.data.ptr) );
      }
      break;
    case tcp_buffer:
      ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        if ( descr.find( ev_add.data.fd ) != descr.end() ) { // reuse?
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev_add.data.fd, &ev_add ) < 0 ) {
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
            // descr.erase( ev_add.data.fd );
            // throw system_error
            return;
          }         
        } else {
          // std::cerr << __FILE__ << ":" << __LINE__ << " " << ev_add.data.fd << std::endl;
          if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
            // throw system_error
            return;
          }
        }
        descr[ev_add.data.fd] = fd_info( static_cast<sockbuf_t*>(_ctl.data.ptr) );
      }
      break;
    case listener_on_exit:
      listeners_final.insert( _ctl.data.ptr );
      // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      {
        int lfd = check_closed_listener( reinterpret_cast<socks_processor_t*>(_ctl.data.ptr) );
        if ( lfd != -1 ) {
          descr.erase( lfd );
        }
        // dump_descr();
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
  // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
  if ( ev.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR) ) {
    // std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << std::endl;
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      // throw system_error
      std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << " " << errno << std::endl;
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

    // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

    socks_processor_t* p = ifd->second.p;

    descr.erase( ifd );

    int lfd = check_closed_listener( p );
    
    if ( lfd != -1 ) {
      descr.erase( lfd );
    }

    // dump_descr();

    return;
  }

  if ( (ev.events & EPOLLIN) == 0 ) {
    // std::cerr << __FILE__ << ":" << __LINE__ << " " << std::hex << ev.events << std::dec << std::endl;
    return; // I don't know what to do this case...
  }

  sockaddr addr;
  socklen_t sz = sizeof( sockaddr_in );

  fd_info info = ifd->second;

  for ( ; ; ) {
    int fd = accept( ev.data.fd, &addr, &sz );
    if ( fd < 0 ) {
      // std::cerr << __FILE__ << ":" << __LINE__ /* << " " << std::tr2::getpid() */ << std::endl;
      if ( (errno == EINTR) || (errno == ECONNABORTED) /* || (errno == ERESTARTSYS) */ ) {
        // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        errno = 0;
        continue;
      }
      if ( !(errno == EAGAIN /* || errno == EWOULDBLOCK */ ) ) { // EWOULDBLOCK == EAGAIN
        // std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << std::endl;
        if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
          std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << " "
                    << errno << std::endl;
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

        // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

        socks_processor_t* p = ifd->second.p;
        listeners_final.insert( static_cast<void *>(p) );

        descr.erase( ifd );

        check_closed_listener( p );

        // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        // dump_descr();
      } else { // back to listen
        errno = 0;
        epoll_event xev;
        xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
        xev.data.fd = ev.data.fd;
        if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
          std::cerr << __FILE__ << ":" << __LINE__ << " " << ev.data.fd << " "
                    << errno << std::endl;
        }
      }
      // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
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

      if ( descr.find( fd ) != descr.end() ) { // reuse?
        // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        if ( epoll_ctl( efd, EPOLL_CTL_MOD, fd, &ev_add ) < 0 ) {
          std::cerr << __FILE__ << ":" << __LINE__ << " " << fd << " " << errno << std::endl;
          descr.erase( fd );
          // throw system_error
          return; // throw
        }
      } else {
        // std::cerr << __FILE__ << ":" << __LINE__ << " " << fd << std::endl;
        if ( epoll_ctl( efd, EPOLL_CTL_ADD, fd, &ev_add ) < 0 ) {
          // throw system_error
          std::cerr << __FILE__ << ":" << __LINE__ << " " << fd << " " << errno << std::endl;
          return; // throw
        }
      }

      // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      sockbuf_t* b = (*info.p)( fd, addr );
      // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

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
    // std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << std::endl;
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << " " << errno << std::endl;
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
    return;
  }

  errno = 0;

  // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
  if ( ev.events & EPOLLIN ) {
    // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    for ( ; ; ) {
      if ( b->_ebuf == b->egptr() ) {
        // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        // process extract data from buffer too slow for us!
        if ( (info.flags & fd_info::level_triggered) == 0 ) {
          epoll_event xev;
          xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
          xev.data.fd = ev.data.fd;
          info.flags |= fd_info::level_triggered;
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
            std::cerr << __FILE__ << ":" << __LINE__ << " " << ev.data.fd << " " << errno << std::endl;
          }
        }
        if ( info.p != 0 ) {
          (*info.p)( ev.data.fd );
        }
        break;
      }

      long offset = read( ev.data.fd, b->egptr(), sizeof(charT) * (b->_ebuf - b->egptr()) );

      if ( offset < 0 ) {
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
              if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
                std::cerr << __FILE__ << ":" << __LINE__ << " " << ev.data.fd << " " << errno << std::endl;
              }
            }
            break;
          default:
            std::cerr << __FILE__ << ":" << __LINE__ << " " << errno << std::endl;
            break;
        }
        break;
      } else if ( offset > 0 ) {
        offset /= sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!
            
        if ( info.flags & fd_info::level_triggered ) {
          epoll_event xev;
          xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
          xev.data.fd = ev.data.fd;
          info.flags &= ~static_cast<unsigned>(fd_info::level_triggered);
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
            std::cerr << __FILE__ << ":" << __LINE__ << " " << ev.data.fd << " " << errno << std::endl;
          }
        }
        std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
        b->setg( b->eback(), b->gptr(), b->egptr() + offset );
        b->ucnd.notify_one();
        if ( info.p != 0 ) {
          // std::cerr << __FILE__ << ":" << __LINE__ << " " << ev.data.fd << std::endl;
          (*info.p)( ev.data.fd );
        }
      } else {
        // std::cerr << __FILE__ << ":" << __LINE__ << " " << ev.data.fd << std::endl;
        // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
        ev.events |= EPOLLRDHUP; // will be processed below
        break;
      }
    }
  }

  if ( (ev.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR) ) != 0 ) {
    // std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << std::endl;
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      // throw system_error
      std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << " " << errno << std::endl;
    }

    if ( info.p != 0 ) {
      // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
      // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

      socks_processor_t* p = info.p;

      descr.erase( ifd );

      int lfd = check_closed_listener( p );
      if ( lfd != -1 ) {
        descr.erase( lfd );
      }
    } else {
      b->_notify_close = false; // avoid deadlock
      // std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << std::endl;
      if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
        // throw system_error
        std::cerr << __FILE__ << ":" << __LINE__ << " " << ifd->first << " " << errno << std::endl;
      }
      descr.erase( ifd );
      b->close();
    }
    // dump_descr();
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
int sockmgr<charT,traits,_Alloc>::check_closed_listener( socks_processor_t* p )
{
  int myfd = -1;

  if ( !listeners_final.empty() ) {
    // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    if ( listeners_final.find( static_cast<void*>(p) ) != listeners_final.end() ) {
      for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
        if ( i->second.p == p ) {
          if ( (i->second.flags & fd_info::listener) == 0 ) { // it's not me!
            // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
            return -1;
          }
          myfd = i->first;
          // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
        }
      }
      // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

      // no more connection with this listener
      listeners_final.erase( static_cast<void*>(p) );

      // std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

      // if ( myfd != -1 ) {
      //   std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
      p->stop();
        // }
    }
  }

  return myfd;
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
