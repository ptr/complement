// -*- C++ -*- Time-stamp: <09/02/19 16:42:27 ptr>

/*
 * Copyright (c) 2008, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <mt/system_error>

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
    throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>" ) );
  }
  if ( pipe( pipefd ) < 0 ) { // check err
    ::close( efd );
    efd = -1;
    throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>" ) );
  }
  // cfd = pipefd[1];

  epoll_event ev_add;
  ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP;
#if 1
  ev_add.data.u64 = 0ULL;
#endif
  ev_add.data.fd = pipefd[0];
  if ( epoll_ctl( efd, EPOLL_CTL_ADD, pipefd[0], &ev_add ) < 0 ) {
    ::close( efd );
    efd = -1;
    ::close( pipefd[1] );
    pipefd[1] = -1;
    ::close( pipefd[0] );
    pipefd[0] = -1;
    throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>" ) );
  }

  _worker = new std::tr2::thread( _loop, this );

  // ctl _ctl;
  // _ctl.cmd = rqstart;

  // write( pipefd[1], &_ctl, sizeof(ctl) );
}

template<class charT, class traits, class _Alloc>
sockmgr<charT,traits,_Alloc>::~sockmgr()
{
  if ( _worker->joinable() && (pipefd[1] != -1) ) {
    ctl _ctl;
    _ctl.cmd = rqstop;
    _ctl.data.ptr = 0;

    ::write( pipefd[1], &_ctl, sizeof(ctl) );

    // _worker->join();
  }

  delete _worker;
  _worker = 0;

  if ( pipefd[1] != -1 ) {
    ::close( pipefd[1] );
    pipefd[1] = -1;
  }
  if ( pipefd[0] != -1 ) {
    ::close( pipefd[0] );
    pipefd[0] = -1;
  }
  if ( efd != -1 ) {
    ::close( efd );
    efd = -1;
  }

  for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
    if ( (i->second.flags & fd_info::listener) == 0 ) {
      sockbuf_t* b = i->second.b;
      if ( b != 0 ) {
        std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
        ::close( b->_fd );
        b->_fd = -1;
        b->ucnd.notify_all();
      } else {
        ::close( i->first );
      }
      (*i->second.p)( i->first, typename socks_processor_t::adopt_close_t() );
    }
  }
  for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
    if ( (i->second.flags & fd_info::listener) != 0 ) {
      ::close( i->first );
      i->second.p->stop();
    }
  }
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

#if 0
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
#endif

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::exit_notify( sockbuf_t* b, sock_base::socket_type fd )
{
  try {
    // std::tr2::unique_lock<std::tr2::mutex> lk( dll, std::tr2::defer_lock );

    if ( /* lk.try_lock() */ false ) {
      cerr << __FILE__ << ':' << __LINE__ << endl;
      typename fd_container_type::iterator ifd = descr.find( fd );
      if ( ifd != descr.end() ) {
        descr.erase( ifd );
        // EPOLL_CTL_DEL done by system on close(fd)
        ::close( fd );
        // don't do std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
        // here: it under this lock (sockbuf_t::close())
        b->_fd = -1;
        b->ucnd.notify_all();
        if ( ifd->second.p != 0 ) {
          (*ifd->second.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
        }
      }
    } else {
#if 0
      /*
        here I don't know that about processing in process_regular
        (process_regular may process this sockbuf, or another sockbuf),
        and I can't intersect this processing. For this I push
        command into pipe, and will process closing socket
        within cmd_from_pipe (ensure from intersection with process_regular).
      */
      ctl _ctl;
      _ctl.cmd = tcp_buffer_on_exit;
      _ctl.data.ptr = static_cast<void *>(b);

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
              throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>::exit_notify" ) );
              break;
          }
        }
        r += ret;
      } while ( (r != sizeof(ctl)) /* || (ret != 0) */ );
#else
      /*
        here I don't know that about processing in process_regular
        (process_regular may process this sockbuf, or another sockbuf),
        and I can't intersect this processing. For this I force event
        on epolled descriptor and will process closing socket
        within process_regular (ensure from intersection with process_regular).
        Note: don't call ::close(fd) here: it remove fd from epoll
        vector and I never see event about this socket!
      */
      b->shutdown_unsafe( /* sock_base::stop_in | */ sock_base::stop_out );
#endif
    }
  }
  catch ( const std::tr2::lock_error& ) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << fd << std::endl;
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
        // cerr << __FILE__ << ':' << __LINE__ << endl;
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
          if ( ifd != descr.end() ) {
            fd_info& info = ifd->second;
            if ( info.flags & fd_info::listener ) {
              process_listener( ev[i], ifd );
            } else {
              process_regular( ev[i], ifd );
            }
          } else {
            cerr << __FILE__ << ':' << __LINE__ << endl;
          }
            // otherwise already closed
            // and [should be] already removed from efd's vector,
            // so no epoll_ctl( efd, EPOLL_CTL_DEL, ev[i].data.fd, 0 ) here
        }
      }
    }
  }
  catch ( std::detail::stop_request& ) {
    // this is possible, normal flow of operation
    for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
      if ( (i->second.flags & fd_info::listener) == 0 ) {
        sockbuf_t* b = i->second.b;
        if ( b != 0 ) {
          std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
          ::close( b->_fd );
          b->_fd = -1;
          b->ucnd.notify_all();
        } else {
          ::close( i->first );
        }
        (*i->second.p)( i->first, typename socks_processor_t::adopt_close_t() );
      }
    }
    for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
      if ( (i->second.flags & fd_info::listener) != 0 ) {
        i->second.p->stop();
      }
    }
  }
  catch ( std::exception& e ) {
    std::cerr << e.what() << std::endl;
  }
  catch ( ... ) {

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
          cerr << __FILE__ << ':' << __LINE__ << endl;
          descr.erase( ev_add.data.fd );
          // std::cerr << __FILE__ << ":" << __LINE__ << " " << std::error_code( errno, std::posix_category ).message() << " " << ev_add.data.fd << " " << std::tr2::getpid() << std::endl;
          return; // already closed?
        }      
        descr[ev_add.data.fd] = fd_info( static_cast<sockbuf_t*>(_ctl.data.ptr) );
      }
      // ->release() ?
      break;
#if 0
    case listener_on_exit:
      {
        socks_processor_t* p = reinterpret_cast<socks_processor_t*>(_ctl.data.ptr);
        for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ) {
          if ( i->second.p == p ) {
            if ( (i->second.flags & fd_info::listener) == 0 ) { // it's not me!
              sockbuf_t* b = i->second.b;
              {
                std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
                ::close( b->_fd );
                b->_fd = -1;
                b->ucnd.notify_all();
              }
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
#endif
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
    // close listener:
    fd_info info = ifd->second;

    for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ) {
      if ( i->second.p == info.p ) {
        if ( (i->second.flags & fd_info::listener) == 0 ) { // it's not me!
          sockbuf_t* b = i->second.b;
          {
            std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
            ::close( b->_fd );
            b->_fd = -1;
            b->ucnd.notify_all();
          }
          (*info.p)( i->first, typename socks_processor_t::adopt_close_t() );
        }
        /* i = */ descr.erase( i++ );
      } else {
        ++i;
      }
    }

    // no more connection with this listener
    info.p->stop();
    ::close( ev.data.fd );

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
      // if ( (errno == EINTR) || (errno == ECONNABORTED) /* || (errno == ERESTARTSYS) */ ) {
      //  errno = 0;
      //  continue;
      // }
      if ( (errno == EAGAIN) || (errno == EINTR) || (errno == ECONNABORTED) ) { // EWOULDBLOCK == EAGAIN
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
      }

      // close listener:

      for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ) {
        if ( i->second.p == info.p ) {
          if ( (i->second.flags & fd_info::listener) == 0 ) { // it's not me!
            sockbuf_t* b = i->second.b;
            {
              std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
              ::close( b->_fd );
              b->_fd = -1;
              b->ucnd.notify_all();
            }
            (*info.p)( i->first, typename socks_processor_t::adopt_close_t() );
          }
          /* i = */ descr.erase( i++ );
        } else {
          ++i;
        }
      }

      // no more connection with this listener
      info.p->stop();
      ::close( ev.data.fd );

      return;
    }

    if ( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      ::close( fd );
      throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>" ) );
    }
      
    try {
      epoll_event ev_add;
      ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
#if 1
      ev_add.data.u64 = 0ULL;
#endif
      ev_add.data.fd = fd;

      if ( epoll_ctl( efd, EPOLL_CTL_ADD, fd, &ev_add ) < 0 ) {
        ::close( fd );
        throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>" ) );
        // return;
      }      

      sockbuf_t* b = (*info.p)( fd, addr );

      try {
        /*
          Here b may be 0, if processor don't delegate control
          under sockbuf_t to sockmgr, but want to see notifications;
          see 'if ( b == 0 )' in process_regular below.
        */
        descr[fd] = fd_info( b, info.p );
      }
      catch ( ... ) {
        cerr << __FILE__ << ':' << __LINE__ << endl;
        try {
          descr.erase( fd );
          {
            std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
            ::close( b->_fd );
            b->_fd = -1;
            b->ucnd.notify_all();
          }
          (*info.p)( fd, typename socks_processor_t::adopt_close_t() );
        }
        catch ( ... ) {
        }
      }
    }
    catch ( ... ) {
      cerr << __FILE__ << ':' << __LINE__ << endl;
      ::close( fd );
      (*info.p)( fd, typename socks_processor_t::adopt_close_t() );
    }
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_regular( epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  fd_info& info = ifd->second;

  sockbuf_t* b = info.b;
  if ( b == 0 ) { // marginal case: sockbuf not delegated to sockgr by processor...
    // This is mainly for debug and tests. Candidate for removing:
    // I don't know how use it in usefull way.

    if ( info.p == 0 ) { // this is a error
      std::cerr << __FILE__ << ':' << __LINE__ << ' ' << ifd->first << std::endl;
      ::close( ifd->first );
      descr.erase( ifd );
      throw std::logic_error( "neither buffer, nor processor" );
    }
    // ... it totally controlled by processor

    /* To do here: discover is data available or connection closed, and do
       appropriate actions; now only close connection.
     */
    // int res = 0;
    // int len = 0;
    // int ret = getsockopt( ifd->first, SOL_SOCKET, SO_ERROR, (void *)&res, (socklen_t*)&len );
    // std::cerr << __FILE__ << ":" << __LINE__ << ' ' << res << ' ' << ret << endl;

    // Urgent: socket is NONBLOCK and polling ONESHOT here!
    char c;
    ssize_t len = ::recv( ifd->first, &c, 1, MSG_PEEK );

    if ( len > 0 ) { 
      (*info.p)( ifd->first );
      return;
    }

    // Note: close socket automatically remove it from epoll's vector,
    // so no epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ); here
    ::close( ifd->first );
    (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
    descr.erase( ifd );
    return;
  }

  errno = 0;

  try {
    // if ( ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR) ) {
    //   cerr << __FILE__ << ':' << __LINE__  << ev.data.fd << endl;
    // }
    if ( ev.events & EPOLLIN ) {
      long offset;
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( b->ulck );

        if ( b->_ebuf == b->egptr() ) {
          // process extract data from buffer too slow for us!
          if ( ((info.flags & fd_info::level_triggered) == 0) && ((ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR)) == 0) ) {
            // set to level triggered, if not done before
            epoll_event xev;
#if 1
            xev.data.u64 = 0ULL;
#endif
            xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP;
            xev.data.fd = ev.data.fd;
            info.flags |= fd_info::level_triggered;
            if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
              cerr << __FILE__ << ':' << __LINE__ << ' ' << std::tr2::getpid()
                   << ' ' << ev.data.fd << endl;
              throw fdclose(); // already closed?
            }
          }
          if ( info.p != 0 ) {
            b->ucnd.notify_one();
            lk.unlock();
            (*info.p)( ev.data.fd );
          }
          if ( ((ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR)) != 0) ) {
            throw fdclose(); // oh, it closed!
          }
          return;
        }

        offset = read( ev.data.fd, b->egptr(), sizeof(charT) * (b->_ebuf - b->egptr()) );
        if ( offset > 0 ) {
          offset /= sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!

          // std::string st;

          // _MUST_ be under same b->ulck as read above!
          b->setg( b->eback(), b->gptr(), b->egptr() + offset );
          // st.assign( b->gptr(), b->egptr() );
          b->ucnd.notify_one();

          // cerr << __FILE__ << ':' << __LINE__ << ' ' << st << endl;
          //  cerr << __FILE__ << ':' << __LINE__ << ' ' << offset << endl;
          if ( info.p != 0 ) {
            lk.unlock();
            (*info.p)( ev.data.fd );
          }
        } else if ( offset == 0 ) {
#if 0
          int res = 0;
          int res2 = 0;
          socklen_t l = sizeof(int);
          int ret = getsockopt( ev.data.fd, SOL_TCP, SO_ERROR, &res, &l );
          l = sizeof(int);
          ret += getsockopt( ev.data.fd, SOL_SOCKET, SO_ERROR, &res2, &l );
          cerr << __FILE__ << ':' << __LINE__ << ' ' << std::tr2::getpid()
               << ' ' << ev.data.fd << ' ' << (sizeof(charT) * (b->_ebuf - b->egptr())) << " '" << res << "' " << res2 << ' ' << ret << ' ' << hex << ev.events << dec << endl;
          if ( info.flags & fd_info::level_triggered ) {
            cerr << __FILE__ << ':' << __LINE__ << endl;
          }

          if ( (res2 == 0) && ((ev.events & (EPOLLERR | EPOLLHUP)) == 0 )) {
            epoll_event xev;
            xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP /* | EPOLLET | EPOLLONESHOT */;
#if 1
            xev.data.u64 = 0ULL;
#endif
            xev.data.fd = ev.data.fd;
            if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) == 0 ) {
              info.flags |= fd_info::level_triggered;
              break; // normal flow, back to epoll
            }
          }
          cerr << __FILE__ << ':' << __LINE__ << ' ' << std::tr2::getpid()
               << ' ' << ev.data.fd << ' ' << errno << endl;          
#endif
          // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
          throw fdclose();
        } else { // offset < 0
          if ( (errno == EINTR) || (errno == EAGAIN) ) { // EWOULDBLOCK
            // read was interrupted
            // or no more ready data available; continue.
            errno = 0;
          } else {
            // EBADF (already closed?), EFAULT (Bad address),
            // ECONNRESET (Connection reset by peer), ...
            throw fdclose();
          }
        }
      }

      if ( (ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR)) == 0 )  {
        if ( (info.flags & fd_info::level_triggered) != 0 ) {
          std::tr2::unique_lock<std::tr2::mutex> lk( b->ulck );
          if ( b->_ebuf == b->egptr() ) {
            // no error, but buffer is full; keep level triggered
            return;
          }
          // space in buffer available, switch to edge triggered
          // (EPOLL_CTL_MOD below)
        }

        // Due to EPOLLONESHOT, we need recover this descriptor
        // in the epoll's vector

        // cerr << __FILE__ << ':' << __LINE__ << ' ' << st << endl;
        epoll_event xev;
        xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
#if 1
        xev.data.u64 = 0ULL;
#endif
        xev.data.fd = ev.data.fd;
        info.flags &= ~static_cast<unsigned>(fd_info::level_triggered);
        if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) < 0 ) {
          cerr << __FILE__ << ':' << __LINE__ << endl;
          throw fdclose(); // nonrecoverable? already closed?
        }
      }
    }

    if ( (ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR) ) != 0 ) {
      // EPOLLIN not set?
      throw fdclose(); // closed connection
    }
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
      {
        // std::cerr << __FILE__ << ':' << __LINE__ << ' ' << b->_fd << ' ' << std::tr2::getpid() << std::endl;
        std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
        // Note: close socket automatically remove it from epoll's vector,
        // so no epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ); here
        ::close( b->_fd );

        // Close and set to -1 _before_ (*info.p)( fd, adopt_close_t() );
        // to avoid deadlock! (wait condition: data or close in
        // basic_sockbuf::underflow())
        // std::cerr << __FILE__ << ':' << __LINE__ << ' ' << b->_fd << ' ' << std::tr2::getpid() << std::endl;
        b->_fd = -1;
        b->ucnd.notify_all();
      }
      
      (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
      descr.erase( ifd );
    } else {
      // std::cerr << __FILE__ << ':' << __LINE__ << ' ' << b->_fd << std::endl;
      descr.erase( ifd );
      std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
      // Note: close socket automatically remove it from epoll's vector,
      // so no epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ); here
#if 0
      cerr << __FILE__ << ':' << __LINE__ << ' ' << std::tr2::getpid()
           << ' ' << b->_fd << endl;
#endif
      ::close( b->_fd );
      b->_fd = -1;
      b->ucnd.notify_all();
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
