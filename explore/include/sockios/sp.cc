// -*- C++ -*- Time-stamp: <08/06/16 11:05:56 ptr>

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
    case tcp_buffer:
      ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        fd_info new_info = { fd_info::buffer, static_cast<sockbuf_t*>(_ctl.data.ptr), 0 };
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
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      // throw system_error
    }

    if ( ifd->second.p != 0 ) {
      ifd->second.p->close();
    }

    descr.erase( ifd );

    std::tr2::lock_guard<std::tr2::mutex> lck( cll );
    typename fd_container_type::iterator closed_ifd = closed_queue.find( ev.data.fd );
    if ( closed_ifd != closed_queue.end() && closed_ifd->second.p == ifd->second.p ) {
      // listener in process of close
      closed_queue.erase( closed_ifd );
    }

    return;
  }

  if ( (ev.events & EPOLLIN) == 0 ) {
    return; // I don't know what to do this case...
  }

  {
    std::tr2::lock_guard<std::tr2::mutex> lck( cll );
    typename fd_container_type::iterator closed_ifd = closed_queue.find( ev.data.fd );
    if ( closed_ifd != closed_queue.end() && closed_ifd->second.p == ifd->second.p ) {
      // listener in process of closing, ignore all incoming connects
      closed_queue.erase( closed_ifd );
      if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
        // throw system_error
      }
      descr.erase( ifd );
      return;
    }      
  }

  sockaddr addr;
  socklen_t sz = sizeof( sockaddr_in );

  fd_info info = ifd->second;

  for ( ; ; ) {
    int fd = accept( ev.data.fd, &addr, &sz );
    if ( fd < 0 ) {
      // std::cerr << "Accept, listener # " << ev.data.fd << ", errno " << errno << std::endl;
      // std::cerr << __FILE__ << ":" << __LINE__ << " " << std::tr2::getpid() << std::endl;
      if ( (errno == EINTR) || (errno == ECONNABORTED) /* || (errno == ERESTARTSYS) */ ) {
        errno = 0;
        continue;
      }
      if ( !(errno == EAGAIN /* || errno == EWOULDBLOCK */ ) ) { // EWOULDBLOCK == EAGAIN
        // std::cerr << "Accept, listener " << ev.data.fd << ", errno " << errno << std::endl;
        if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
          // throw system_error
        }

        if ( ifd->second.p != 0 ) {
          ifd->second.p->close();
        }

        descr.erase( ifd );

        // check closed_queue, due to ifd->second.p->close(); add record in it
        std::tr2::lock_guard<std::tr2::mutex> lck( cll );
        typename fd_container_type::iterator closed_ifd = closed_queue.find( ev.data.fd );
        if ( closed_ifd != closed_queue.end() && closed_ifd->second.p == ifd->second.p ) {
          // listener in process of close
          closed_queue.erase( closed_ifd );
        }
        // throw system_error ?
      } else { // back to listen
        errno = 0;
        epoll_event xev;
        xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
        xev.data.fd = ev.data.fd;
        epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev );
      }
      return;
    }
    if ( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      throw std::runtime_error( "can't establish nonblock mode" );
    }
      
    try {
      sockbuf_t* b = (*info.p)( fd, addr );

      epoll_event ev_add;
      ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.fd = fd;
      fd_info new_info = { fd_info::owner, b, info.p };
      descr[fd] = new_info;

      if ( epoll_ctl( efd, EPOLL_CTL_ADD, fd, &ev_add ) < 0 ) {
        descr.erase( fd );
        // throw system_error
      }
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

  if ( ev.events & EPOLLIN ) {
    if ( (info.flags & fd_info::owner) == 0 ) {
      /*
      marginal case: sockmgr isn't owner (registerd via push(),
      when I owner, I know destroy point),
      already closed, but I don't see closed event yet;
      object may be deleted already, so I can't
      call b->egptr() etc. here
      */
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
    sockbuf_t* b = /* (info.flags & fd_info::buffer != 0) ? info.s.b : info.s.s->rdbuf() */ info.b;
    errno = 0;
    if ( b == 0 ) { // marginal case: sockbuf wasn't created by processor
      if ( info.p != 0 ) {
        (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
      }
      descr.erase( ifd );
      return;
    }
    for ( ; ; ) {
      if ( b->_ebuf == b->egptr() ) {
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
        switch ( errno ) {
          case EINTR:      // read was interrupted
            errno = 0;
            continue;
            break;
          case EFAULT:     // Bad address
          case ECONNRESET: // Connection reset by peer
            ev.events |= EPOLLRDHUP; // will be processed below
            break;
          case EAGAIN:
            // case EWOULDBLOCK:
            {
              epoll_event xev;
              xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
              xev.data.fd = ev.data.fd;
              epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev );
            }
            break;
          default:
            // std::cerr << "not listener, other " << ev.data.fd << std::hex << ev.events << std::dec << " : " << errno << std::endl;
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
            // std::cerr << "Y " << ev.data.fd << ", " << errno << std::endl;
          }
        }
        std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
        b->setg( b->eback(), b->gptr(), b->egptr() + offset );
        b->ucnd.notify_one();
        if ( info.p != 0 ) {
          (*info.p)( ev.data.fd );
        }
      } else {
        // std::cerr << "K " << ev.data.fd << ", " << errno << std::endl;
        // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
        ev.events |= EPOLLRDHUP; // will be processed below
        break;
      }
    }
  }

  if ( (ev.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR) ) != 0 ) {
    // std::cerr << "Poll EPOLLRDHUP " << ev.data.fd << ", " << errno << std::endl;
    if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
      // throw system_error
    }

    if ( info.p != 0 ) {
      (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
    }
    if ( (info.flags & fd_info::buffer) != 0 ) {
      info.b->close();
    }
    std::tr2::lock_guard<std::tr2::mutex> lck( cll );
    closed_queue.erase( ev.data.fd );
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
}

} // namespace detail

} // namespace std
