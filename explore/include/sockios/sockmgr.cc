// -*- C++ -*- Time-stamp: <10/05/21 15:22:14 ptr>

/*
 * Copyright (c) 2008-2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <mt/system_error>
#include <exam/defs.h>

namespace std {

namespace detail {

template<class charT, class traits, class _Alloc>
sockmgr<charT,traits,_Alloc>::sockmgr( int hint, int ret ) :
    efd( -1 ),
    _worker( 0 ),
    n_ret( ret )
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

  epoll_event ev_add;
  ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP;
  ev_add.data.u64 = 0ULL;

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
    if ( (i->second.flags & (fd_info::listener | fd_info::dgram_proc)) == 0 ) {
      sockbuf_t* b = i->second.b;
      if ( b != 0 ) {
        std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
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
    } // fd_info::dgram_proc?
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
          continue;
        default:
          p.release();
          throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>::push( socks_processor_t& p )" ) );
      }
    }
    r += ret;
  } while ( (r != sizeof(ctl)) /* || (ret != 0) */ );
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::push_dp( socks_processor_t& p )
{
  ctl _ctl;
  _ctl.cmd = dgram_proc;
  _ctl.data.ptr = static_cast<void *>(&p);

  p.addref();
  int r = 0;
  int ret = 0;
  do {
    ret = ::write( pipefd[1], &_ctl, sizeof(ctl) );
    if ( ret < 0 ) {
      switch ( errno ) {
        case EINTR:
          continue;
        default:
          p.release();
          throw std::system_error( errno, std::get_posix_category(), std::string( __PRETTY_FUNCTION__ ) );
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
          continue;
        default:
          throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>::push( sockbuf_t& s )" ) );
      }
    }
    r += ret;
  } while ( (r != sizeof(ctl)) /* || (ret != 0) */ );
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::restore( sockbuf_t& s )
{
  ctl _ctl;
  _ctl.cmd = tcp_buffer_back;
  _ctl.data.fd = s._fd;

  errno = 0;

  int r = 0;
  int ret = 0;
  do {
    ret = ::write( pipefd[1], &_ctl, sizeof(ctl) );
    if ( ret < 0 ) {
      switch ( errno ) {
        case EINTR:
          continue;
        default:
          throw std::system_error( errno, std::get_posix_category(), std::string( "sockmgr<charT,traits,_Alloc>::restore( sockbuf_t& s )" ) );
      }
    }
    r += ret;
  } while ( (r != sizeof(ctl)) /* || (ret != 0) */ );
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::io_worker()
{
  epoll_event ev[ n_ret ];

  std::tr2::this_thread::signal_handler( SIGPIPE, SIG_IGN );

  memset( ev, 0, n_ret * sizeof(epoll_event) );

  try {
    for ( ; ; ) {
      int n = epoll_wait( efd, &ev[0], n_ret, -1 );

      if ( n < 0 ) {
        if ( errno == EINTR ) {
          errno = 0;
          continue;
        }

        extern std::tr2::mutex _se_lock;
        extern std::ostream* _se_stream;

        std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
        if ( _se_stream != 0 ) {
          *_se_stream << HERE << ' '
                      << efd << ' '
                      << std::posix_error::make_error_code( static_cast<std::posix_error::posix_errno>(errno) ).message()
                      << std::endl;
        }

        throw std::detail::stop_request();
      }

      for ( int i = 0; i < n; ++i ) {
        try {
          if ( ev[i].data.fd == pipefd[0] ) {
            cmd_from_pipe();
          } else {
            typename fd_container_type::iterator ifd = descr.find( ev[i].data.fd );
            if ( ifd != descr.end() ) {
              fd_info& info = ifd->second;
              if ( info.flags & fd_info::listener ) {
                process_listener( ev[i], ifd );
              } else if ( info.flags & fd_info::dgram_proc ) {
                process_dgram_srv( ev[i], ifd );
              } else {
                process_regular( ev[i], ifd );
              }
            } // otherwise already closed
              // and [should be] already removed from efd's vector,
              // so no epoll_ctl( efd, EPOLL_CTL_DEL, ev[i].data.fd, 0 ) here
          }
        }
        catch ( const std::system_error& err ) {
          extern std::tr2::mutex _se_lock;
          extern std::ostream* _se_stream;

          std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
          if ( _se_stream != 0 ) {
            *_se_stream << err.what() << std::endl;
          }
        }
        catch ( const std::runtime_error& err ) {
          extern std::tr2::mutex _se_lock;
          extern std::ostream* _se_stream;

          std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
          if ( _se_stream != 0 ) {
            *_se_stream << err.what() << std::endl;
          }
        }
      }
    }
  }
  catch ( std::detail::stop_request& ) {
    try {
      // this is possible, normal flow of operation
      for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
        if ( (i->second.flags & (fd_info::listener | fd_info::dgram_proc)) == 0 ) {
          sockbuf_t* b = i->second.b;
          if ( b != 0 ) {
            std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
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
      try {
        extern std::tr2::mutex _se_lock;
        extern std::ostream* _se_stream;

        std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
        if ( _se_stream != 0 ) {
          *_se_stream << HERE << ' ' << e.what() << std::endl;
        }
      }
      catch ( ... ) {
      }
    }
    catch ( ... ) {
      try {
        extern std::tr2::mutex _se_lock;
        extern std::ostream* _se_stream;

        std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
        if ( _se_stream != 0 ) {
          *_se_stream << HERE << " unknown exception" << std::endl;
        }
      }
      catch ( ... ) {
      }
    }
  }
  catch ( std::exception& e ) {
    try {
      extern std::tr2::mutex _se_lock;
      extern std::ostream* _se_stream;

      std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
      if ( _se_stream != 0 ) {
        *_se_stream << HERE << ' ' << e.what() << std::endl;
      }
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
    try {
      extern std::tr2::mutex _se_lock;
      extern std::ostream* _se_stream;

      std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
      if ( _se_stream != 0 ) {
        *_se_stream << HERE << " unknown exception" << std::endl;
      }
    }
    catch ( ... ) {
    }
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::cmd_from_pipe()
{
  epoll_event ev_add;

  ctl _ctl;

  int ret = 0;
  do {
    ret = read( pipefd[0], &_ctl, sizeof(ctl) );
    if ( ret < 0 ) {
      switch ( errno ) {
        case EINTR:
          continue;
        default:
          throw std::detail::stop_request(); // runtime_error( "Stop request (normal flow)" );
      }
    } else if ( ret == 0 ) {
      throw runtime_error( "Read pipe return 0" );
    }
  } while ( ret != sizeof(ctl) );

  switch ( _ctl.cmd ) {
    case listener:
      ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.u64 = 0ULL;

      ev_add.data.fd = static_cast<socks_processor_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        if ( fcntl( ev_add.data.fd, F_SETFL, fcntl( ev_add.data.fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
          static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
          throw std::runtime_error( "can't establish nonblock mode on listener" );
        }
        if ( descr.find( ev_add.data.fd ) != descr.end() ) { // reuse?
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev_add.data.fd, &ev_add ) < 0 ) {
            extern std::tr2::mutex _se_lock;
            extern std::ostream* _se_stream;

            std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
            if ( _se_stream != 0 ) {
              *_se_stream << HERE << ' '
                          << std::posix_error::make_error_code( static_cast<std::posix_error::posix_errno>(errno) ).message()
                          << std::endl;
            }
            // descr.erase( ev_add.data.fd );
            static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
            return;
          }
        } else {
          if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
            extern std::tr2::mutex _se_lock;
            extern std::ostream* _se_stream;

            std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
            if ( _se_stream != 0 ) {
              *_se_stream << HERE << ' '
                          << std::posix_error::make_error_code( static_cast<std::posix_error::posix_errno>(errno) ).message()
                          << std::endl;
            }
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
      ev_add.data.u64 = 0ULL;

      ev_add.data.fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
          extern std::tr2::mutex _se_lock;
          extern std::ostream* _se_stream;

          std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
          if ( _se_stream != 0 ) {
            *_se_stream << HERE << ' '
                        << std::posix_error::make_error_code( static_cast<std::posix_error::posix_errno>(errno) ).message()
                        << std::endl;
          }

          descr.erase( ev_add.data.fd );

          return; // already closed?
        }
        descr[ev_add.data.fd] = fd_info( static_cast<sockbuf_t*>(_ctl.data.ptr) );
      }
      break;
    case rqstop:
      throw std::detail::stop_request();
      // break;
    case tcp_buffer_back:
      // return back to epoll
      ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.u64 = 0ULL;

      ev_add.data.fd = _ctl.data.fd;
      if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev_add.data.fd, &ev_add ) < 0 ) {
        return; // already closed?
      }
      break;
    case dgram_proc:
      ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.u64 = 0ULL;

      ev_add.data.fd = static_cast<socks_processor_t*>(_ctl.data.ptr)->fd();
      if ( ev_add.data.fd >= 0 ) {
        if ( fcntl( ev_add.data.fd, F_SETFL, fcntl( ev_add.data.fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
          static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
          throw std::runtime_error( "can't establish nonblock mode on listener" );
        }
        if ( descr.find( ev_add.data.fd ) != descr.end() ) { // reuse?
          if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev_add.data.fd, &ev_add ) < 0 ) {
            extern std::tr2::mutex _se_lock;
            extern std::ostream* _se_stream;

            std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
            if ( _se_stream != 0 ) {
              *_se_stream << HERE << ' '
                          << std::posix_error::make_error_code( static_cast<std::posix_error::posix_errno>(errno) ).message()
                          << std::endl;
            }

            // descr.erase( ev_add.data.fd );
            static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
            return;
          }
        } else {
          if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
            extern std::tr2::mutex _se_lock;
            extern std::ostream* _se_stream;

            std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
            if ( _se_stream != 0 ) {
              *_se_stream << HERE << ' '
                          << std::posix_error::make_error_code( static_cast<std::posix_error::posix_errno>(errno) ).message()
                          << std::endl;
            }
            static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
            return;
          }
        }
        descr[ev_add.data.fd] = fd_info( fd_info::dgram_proc, 0, static_cast<socks_processor_t*>(_ctl.data.ptr) );
      }
      static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
      break;
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_listener( const epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  if ( ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR) ) {
    // close listener:
    fd_info info = ifd->second;

    std::list<typename fd_container_type::key_type> trash;
    for ( typename fd_container_type::const_iterator i = descr.begin(); i != descr.end(); ++i ) {
      if ( i->second.p == info.p ) {
        if ( (i->second.flags & fd_info::listener) == 0 ) { // it's not me!
          sockbuf_t* b = i->second.b;
          {
            std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
            ::close( b->_fd );
            b->_fd = -1;
            b->ucnd.notify_all();
          }
          (*info.p)( i->first, typename socks_processor_t::adopt_close_t() );
        }
        trash.push_back( i->first );
      }
    }
    for ( typename std::list<typename fd_container_type::key_type>::const_iterator i = trash.begin(); i != trash.end(); ++i ) {
      descr.erase( *i );
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

  const int acc_lim = 3;

  for ( int i = 0; i < acc_lim; ++i ) {
    int fd = accept( ev.data.fd, &addr, &sz );
    if ( fd < 0 ) {
      // if ( (errno == EINTR) || (errno == ECONNABORTED) /* || (errno == ERESTARTSYS) */ ) {
      //  errno = 0;
      //  continue;
      // }
      // if i == 0, then suspect that listener closed
      if ( i > 0 && ((errno == EAGAIN) || (errno == EINTR) || (errno == ECONNABORTED)) ) { // EWOULDBLOCK == EAGAIN
        // back to listen
        errno = 0;
        epoll_event xev;
        xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
        xev.data.u64 = 0ULL;

        xev.data.fd = ev.data.fd;
        if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) == 0 ) {
          return; // normal flow, back to epoll
        }
        // closed?
      } else if ( (errno == EMFILE) || (errno == ENFILE) ) {
        // back to listen
        int save_errno = errno;
        errno = 0;
        epoll_event xev;
        xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
        xev.data.u64 = 0ULL;

        xev.data.fd = ev.data.fd;
        if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) == 0 ) {
          throw std::system_error( save_errno, std::get_posix_category(), std::string( __PRETTY_FUNCTION__ ) );
        }
        // closed?
      }

      // close listener:

      std::list<typename fd_container_type::key_type> trash;
      for ( typename fd_container_type::const_iterator i = descr.begin(); i != descr.end(); ++i ) {
        if ( i->second.p == info.p ) {
          if ( (i->second.flags & fd_info::listener) == 0 ) { // it's not me!
            sockbuf_t* b = i->second.b;
            {
              std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
              ::close( b->_fd );
              b->_fd = -1;
              b->ucnd.notify_all();
            }
            (*info.p)( i->first, typename socks_processor_t::adopt_close_t() );
          }
          trash.push_back( i->first );
        }
      }
      for ( typename std::list<typename fd_container_type::key_type>::const_iterator i = trash.begin(); i != trash.end(); ++i ) {
        descr.erase( *i );
      }

      // no more connection with this listener
      info.p->stop();
      ::close( ev.data.fd );

      return;
    }

    if ( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      ::close( fd );
      throw std::system_error( errno, std::get_posix_category(), std::string( __PRETTY_FUNCTION__ ) );
    }
      
    try {
      epoll_event ev_add;
      ev_add.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
      ev_add.data.u64 = 0ULL;

      ev_add.data.fd = fd;

      if ( epoll_ctl( efd, EPOLL_CTL_ADD, fd, &ev_add ) < 0 ) {
        ::close( fd );
        throw std::system_error( errno, std::get_posix_category(), std::string( __PRETTY_FUNCTION__ ) );
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
        extern std::tr2::mutex _se_lock;
        extern std::ostream* _se_stream;

        {
          std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
          if ( _se_stream != 0 ) {
            *_se_stream << HERE << std::endl;
          }
        }
        try {
          descr.erase( fd );
          {
            std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
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
      extern std::tr2::mutex _se_lock;
      extern std::ostream* _se_stream;

      {
        std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
        if ( _se_stream != 0 ) {
          *_se_stream << HERE << ' ' << fd << std::endl;
        }
      }
      ::close( fd );
      (*info.p)( fd, typename socks_processor_t::adopt_close_t() );
    }
  }

  // restricted accept, acc_lim reached;
  // then try to return listener back to epoll
  epoll_event xev;
  xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
  xev.data.u64 = 0ULL;

  xev.data.fd = ev.data.fd;
  if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev.data.fd, &xev ) == 0 ) {
    return; // normal flow, back to epoll
  }
  // listener closed?

  // do procedure when listener closed:
  std::list<typename fd_container_type::key_type> trash;
  for ( typename fd_container_type::const_iterator i = descr.begin(); i != descr.end(); ++i ) {
    if ( i->second.p == info.p ) {
      if ( (i->second.flags & fd_info::listener) == 0 ) { // it's not me!
        sockbuf_t* b = i->second.b;
        {
          std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
          ::close( b->_fd );
          b->_fd = -1;
          b->ucnd.notify_all();
        }
        (*info.p)( i->first, typename socks_processor_t::adopt_close_t() );
      }
      trash.push_back( i->first );
    }
  }
  for ( typename std::list<typename fd_container_type::key_type>::const_iterator i = trash.begin(); i != trash.end(); ++i ) {
    descr.erase( *i );
  }

  // no more connection with this listener
  info.p->stop();
  ::close( ev.data.fd );
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_dgram_srv( const epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  if ( ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR) ) {
    // close listener:
    fd_info info = ifd->second;

    std::list<typename fd_container_type::key_type> trash;
    for ( typename fd_container_type::const_iterator i = descr.begin(); i != descr.end(); ++i ) {
      if ( i->second.p == info.p ) {
        if ( (i->second.flags & fd_info::dgram_proc) == 0 ) { // it's not me!
          sockbuf_t* b = i->second.b;
          {
            std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
            ::close( b->_fd );
            b->_fd = -1;
            b->ucnd.notify_all();
          }
          // possible problem, because of it may happen in dtor of info.p(...):
          (*info.p)( i->first, typename socks_processor_t::adopt_close_t() );
        }
        trash.push_back( i->first );
      }
    }
    for ( typename std::list<typename fd_container_type::key_type>::const_iterator i = trash.begin(); i != trash.end(); ++i ) {
      descr.erase( *i );
    }

    // no more connection with this processor
    info.p->stop();
    ::close( ev.data.fd );

    return;
  }

  if ( (ev.events & EPOLLIN) == 0 ) {
    // sockbuf_t* b = (*info.p)( ifd->first, addr );
    return; // I don't know what to do this case...
  }

  fd_info& info = ifd->second;
  sockaddr addr;
  socklen_t sz = sizeof( sockaddr ); // sockaddr_un or sockaddr_in

  // Urgent: socket is NONBLOCK and polling ONESHOT here!
  char c;
  ssize_t len = ::recvfrom( ifd->first, &c, 1, MSG_PEEK, &addr, &sz );

  if ( /* len <= 0 */ len < 0 ) { // epoll notified, no data
    // cerr << HERE << ' ' << len << ' ' << errno << ' ' << std::system_error( errno, std::get_posix_category() ).what() << endl;
    switch ( errno ) {
      default:
        // case EAGAIN: // <---
        // case EBADF:
        // case ECONNREFUSED:
        // case ENOMEM:
        // case ENOTCONN:
        descr.erase( ifd );
        // may be in dtor of info.p!
        // no more connection with this processor
        // info.p->stop();
        // ::close( ev.data.fd );
        break;
      case EINTR:
        break;
    }
    return;
  }

  // cerr << HERE << endl;
  // (*info.p)( ifd->first );
  /* sockbuf_t* b = */ (*info.p)( ifd->first, addr );

#if 0
  try {
    /*
      Here b may be 0, if processor don't delegate control
      under sockbuf_t to sockmgr, but want to see notifications;
      see 'if ( b == 0 )' in process_regular below.
    */
    descr[fd] = fd_info( b, info.p );
  }
  catch ( ... ) {
    extern std::tr2::mutex _se_lock;
    extern std::ostream* _se_stream;

    {
      std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
      if ( _se_stream != 0 ) {
        *_se_stream << HERE << std::endl;
      }
    }
    try {
      descr.erase( fd );
      {
        std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
        ::close( b->_fd );
        b->_fd = -1;
        b->ucnd.notify_all();
      }
      (*info.p)( fd, typename socks_processor_t::adopt_close_t() );
    }
    catch ( ... ) {
    }
  }
#endif
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::net_read( typename sockmgr<charT,traits,_Alloc>::sockbuf_t& b ) throw (fdclose, no_free_space, retry, no_ready_data)
{
  std::tr2::unique_lock<std::tr2::recursive_mutex> lk( b.ulck, std::tr2::defer_lock_t() );
  if ( lk.try_lock() ) {
    if ( b._fr < b._ebuf ) {
      long offset = ::read( b._fd, b._fr, sizeof(charT) * (b._ebuf - b._fr) );
      if ( offset > 0 ) {
        b._fr += offset / sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!
        if ( (b._fr < b._ebuf) || (b._fl < b.gptr()) ) { // free space available?
          // return back to epoll
          epoll_event xev; // local var, don't modify ev
          xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
          xev.data.u64 = 0ULL;

          xev.data.fd = b._fd;

          if ( epoll_ctl( efd, EPOLL_CTL_MOD, xev.data.fd, &xev ) < 0 ) {
            throw fdclose(); // closed?
          }
        }
        b.ucnd.notify_one();
      } else if ( offset == 0 ) {
        // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
        throw fdclose();
      } else switch ( errno ) { // offset < 0
        case EAGAIN: // EWOULDBLOCK
          // no more ready data available
          errno = 0;
          // return back to epoll
          if ( b._type == std::sock_base::sock_stream ) {
            epoll_event xev; // local var, don't modify ev
            xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
            xev.data.u64 = 0ULL;

            xev.data.fd = b._fd;

            if ( epoll_ctl( efd, EPOLL_CTL_MOD, xev.data.fd, &xev ) < 0 ) {
              throw fdclose(); // closed?
            }
          } else if ( b._type == std::sock_base::sock_dgram ) {
            throw fdclose(); // closed?
          }
          throw no_ready_data();
        case EINTR: // if EINTR, continue
          throw retry();
        default:           // EBADF (already closed?), EFAULT (Bad address),
          throw fdclose(); // ECONNRESET (Connection reset by peer), ...
      }
    } else {
      charT* gptr = b.gptr();
      if ( b._fl < gptr ) {
        long offset = ::read( b._fd, b._fl, sizeof(charT) * (gptr - b._fl) );
        if ( offset > 0 ) {
          b._fl += offset / sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!
          if ( b._fl < gptr ) { // free space available?
            // return back to epoll
            epoll_event xev; // local var, don't modify ev
            xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
            xev.data.u64 = 0ULL;

            xev.data.fd = b._fd;

            if ( epoll_ctl( efd, EPOLL_CTL_MOD, xev.data.fd, &xev ) < 0 ) {
              throw fdclose(); // closed?
            }
          }
          b.ucnd.notify_one();
        } else if ( offset == 0 ) {
          // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
          throw fdclose();
        } else switch ( errno ) { // offset < 0
          case EAGAIN: // EWOULDBLOCK
            // no more ready data available; return back to epoll.
            errno = 0;
            if ( b._type == std::sock_base::sock_stream ) {
              epoll_event xev; // local var, don't modify ev
              xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
              xev.data.u64 = 0ULL;

              xev.data.fd = b._fd;
              if ( epoll_ctl( efd, EPOLL_CTL_MOD, xev.data.fd, &xev ) < 0 ) {
                throw fdclose(); // hmm, unexpected here; closed?
              }
            } else if ( b._type == std::sock_base::sock_dgram ) {
              throw fdclose();
            }
            throw no_ready_data();
          case EINTR: // if EINTR, continue
            throw retry();
          default:   // EBADF (already closed?), EFAULT (Bad address),
            throw fdclose(); // ECONNRESET (Connection reset by peer), ...
        }
      } else { // process extract data from buffer too slow for us!
        throw no_free_space(); // No free space in the buffer.
      }
    }
  } else { // it locked someware; let's return to this descriptor later
    // return back to epoll
    epoll_event xev; // local var, don't modify ev
    xev.events = EPOLLIN | /* EPOLLRDHUP | */ EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
    xev.data.u64 = 0ULL;

    xev.data.fd = b._fd;

    if ( epoll_ctl( efd, EPOLL_CTL_MOD, xev.data.fd, &xev ) < 0 ) {
      extern std::tr2::mutex _se_lock;
      extern std::ostream* _se_stream;

      std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
      if ( _se_stream != 0 ) {
        *_se_stream << HERE << ' '
                    << std::posix_error::make_error_code( static_cast<std::posix_error::posix_errno>(errno) ).message()
                    << std::endl;
      }

      throw fdclose(); // closed?
    }
    // really data may be available, but let's process another descriptor
    throw no_ready_data(); 
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_regular( const epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  fd_info& info = ifd->second;

  sockbuf_t* b = info.b;
  if ( b == 0 ) { // marginal case: sockbuf not delegated to sockmgr by processor...
    // This is mainly for debug and tests. Candidate for removing:
    // I don't know how use it in usefull way.

    if ( info.p == 0 ) { // this is a error
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
    // std::cerr << HERE << ' ' << res << ' ' << ret << endl;

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
    if ( ev.events & EPOLLIN ) {
      if ( b->stype() == std::sock_base::sock_stream ) {
        // loop here because epoll may report about few events:
        // data available + FIN (connection closed by peer)
        // only once. I should try to read while ::read return -1
        // and set EGAIN or return 0 (i.e. FIN discovered).
        for ( int k = 0; k < 4; ++k ) { // restrict by 4 read trials: give chance to others
          try {
            net_read( *b );
            if ( info.p != 0 ) {
              (*info.p)( ev.data.fd );
            }
          }
          catch ( const retry& ) {
          }
        }
      } else if ( b->stype() == std::sock_base::sock_dgram ) {
        try {
          net_read( *b );
          if ( info.p != 0 ) {
            (*info.p)( ev.data.fd );
          }
        }
        catch ( const retry& ) {
        }
      }
    }

    if ( (ev.events & (/* EPOLLRDHUP | */ EPOLLHUP | EPOLLERR) ) != 0 ) {
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
        std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
        // Note: close socket automatically remove it from epoll's vector,
        // so no epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ); here
        ::close( b->_fd );
        // Close and set to -1 _before_ (*info.p)( fd, adopt_close_t() );
        // to avoid deadlock! (wait condition: data or close in
        // basic_sockbuf::underflow())
        b->_fd = -1;
        b->ucnd.notify_all();
      }
      
      (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
      descr.erase( ifd );
    } else {
      descr.erase( ifd );
      std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
      // Note: close socket automatically remove it from epoll's vector,
      // so no epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ); here
      ::close( b->_fd );
      b->_fd = -1;
      b->ucnd.notify_all();
    }
    // dump_descr();
  }
  catch ( const no_ready_data& ) {
  }
  catch ( const no_free_space& ) {
    if ( info.p != 0 ) {
      (*info.p)( ev.data.fd );
    }
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::dump_descr()
{
  extern std::tr2::mutex _se_lock;
  extern std::ostream* _se_stream;

  std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
  if ( _se_stream != 0 ) {
    for ( typename fd_container_type::iterator i = descr.begin(); i != descr.end(); ++i ) {
      *_se_stream << i->first << " "
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
}


} // namespace detail

} // namespace std
