// -*- C++ -*-

/*
 * Copyright (c) 2008-2010, 2019
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#if !defined(STLPORT) && defined(__GNUC__) && (__GNUC__ >= 5)
#include <system_error>
#else
#include <mt/system_error>
#endif
#include <exam/defs.h>

namespace std {

namespace detail {

template<class charT, class traits, class _Alloc>
bool sockmgr<charT,traits,_Alloc>::epoll_push(int fd, int events)
{
  epoll_event ev_add;
  ev_add.data.u64 = 0ULL;
  ev_add.events = events;
  ev_add.data.fd = fd;

  return epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev_add) == 0;
}

template<class charT, class traits, class _Alloc>
bool sockmgr<charT,traits,_Alloc>::epoll_restore(int fd, int events)
{
  epoll_event ev_mod;
  ev_mod.data.u64 = 0ULL;
  ev_mod.events = events;
  ev_mod.data.fd = fd;

  return epoll_ctl(efd, EPOLL_CTL_MOD, fd, &ev_mod) == 0;
}

template<class charT, class traits, class _Alloc>
sockmgr<charT,traits,_Alloc>::sockmgr(int _fd_count_hint, int _maxevents) :
    efd( -1 ),
    _worker( 0 ),
    maxevents( _maxevents )
{
  pipefd[0] = -1;
  pipefd[1] = -1;

  efd = epoll_create(_fd_count_hint);
  if ( efd < 0 ) {
    throw std::system_error( errno, std::system_category(), std::string( "sockmgr<charT,traits,_Alloc>" ) );
  }

  if ( pipe( pipefd ) < 0 ) {
    ::close( efd );
    efd = -1;
    throw std::system_error( errno, std::system_category(), std::string( "sockmgr<charT,traits,_Alloc>" ) );
  }

  if (!epoll_push(pipefd[0], EPOLLIN | EPOLLERR | EPOLLHUP)) {
    ::close( efd );
    efd = -1;
    ::close( pipefd[1] );
    pipefd[1] = -1;
    ::close( pipefd[0] );
    pipefd[0] = -1;
    throw std::system_error( errno, std::system_category(), std::string( "sockmgr<charT,traits,_Alloc>" ) );
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
    if ((i->second.flags & (fd_info::listener |
                            fd_info::dgram_proc |
                            fd_info::nonsock_proc |
                            fd_info::nonsock_buffer)
          ) == 0) {
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
  push_proc(p, listener);
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::push_dp( socks_processor_t& p )
{
  push_proc(p, dgram_proc);
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::push_nsp( socks_processor_t& p )
{
  push_proc(p, nonsock_proc);
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::push( sockbuf_t& s )
{
  sockmgr<charT,traits,_Alloc>::push_cmd(tcp_buffer, static_cast<void *>(&s));
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::push_nsp( sockbuf_t& s )
{
  sockmgr<charT,traits,_Alloc>::push_cmd(nonsock_buffer, static_cast<void *>(&s));
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::push_close(sockbuf_t& s)
{
  sockmgr<charT,traits,_Alloc>::push_cmd(close_fd, static_cast<void *>(&s));
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::push_cmd(command_type cmd, void *data)
{
  ctl _ctl;
  _ctl.cmd = cmd;
  _ctl.data.ptr = data;

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
          throw std::system_error( errno, std::system_category(), std::string( "sockmgr<charT,traits,_Alloc>::push_cmd(command_type, data)" ) );
      }
    }
    r += ret;
  } while ( (r != sizeof(ctl)));
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::io_worker()
{
  epoll_event ev[maxevents];

  std::tr2::this_thread::signal_handler( SIGPIPE, SIG_IGN );

  memset( ev, 0, maxevents * sizeof(epoll_event) );

  try {
    for ( ; ; ) {
      int n = epoll_wait( efd, &ev[0], maxevents, -1 );

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
                      << std::make_error_code( static_cast<std::errc>(errno) ).message()
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
              } else if ( info.flags & fd_info::nonsock_proc ) {
                process_nonsock_srv( ev[i], ifd );
              } else { // tcp_buffer | nonsock_buffer
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
        if ((i->second.flags & (fd_info::listener |
                                fd_info::dgram_proc |
                                fd_info::nonsock_proc |
                                fd_info::nonsock_buffer)) == 0) {
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
    {
      int listener_fd = static_cast<socks_processor_t*>(_ctl.data.ptr)->fd();

      if (listener_fd >= 0) {
        if ( fcntl( listener_fd, F_SETFL, fcntl( listener_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
          static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
          throw std::runtime_error( "can't establish nonblock mode on listener" );
        }
        
        if ( descr.find(listener_fd) != descr.end() ) { // reuse?
          if (!epoll_restore(listener_fd)) {
            extern std::tr2::mutex _se_lock;
            extern std::ostream* _se_stream;

            std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
            if ( _se_stream != 0 ) {
              *_se_stream << HERE << ' '
                          << std::make_error_code( static_cast<std::errc>(errno) ).message()
                          << std::endl;
            }
            // descr.erase(listener_fd);
            static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
            return;
          }
        } else {
          if (!epoll_push(listener_fd)) {
            extern std::tr2::mutex _se_lock;
            extern std::ostream* _se_stream;

            std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
            if ( _se_stream != 0 ) {
              *_se_stream << HERE << ' '
                          << std::make_error_code( static_cast<std::errc>(errno) ).message()
                          << std::endl;
            }
            static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
            return;
          }
        }

        descr[listener_fd] = fd_info( static_cast<socks_processor_t*>(_ctl.data.ptr) );
      }

      static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
      break;
    }
    case tcp_buffer:
    {
      int tcp_buffer_fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();

      if ( tcp_buffer_fd >= 0 ) {
        if (!epoll_push(tcp_buffer_fd)) {
          extern std::tr2::mutex _se_lock;
          extern std::ostream* _se_stream;

          std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
          if ( _se_stream != 0 ) {
            *_se_stream << HERE << ' '
                        << std::make_error_code( static_cast<std::errc>(errno) ).message()
                        << std::endl;
          }

          descr.erase( tcp_buffer_fd );

          return; // already closed?
        }
        descr[tcp_buffer_fd] = fd_info( static_cast<sockbuf_t*>(_ctl.data.ptr) );
      }
      break;
    }
    case rqstop:
    {
      throw std::detail::stop_request();
    }
    case dgram_proc:
    case nonsock_proc:
    {
      int proc_fd = static_cast<socks_processor_t*>(_ctl.data.ptr)->fd();
      if ( proc_fd >= 0 ) {
        if ( fcntl( proc_fd, F_SETFL, fcntl( proc_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
          static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
          throw std::runtime_error( "can't establish nonblock mode on listener" );
        }
        if ( descr.find( proc_fd ) != descr.end() ) { // reuse?
          if (!epoll_restore(proc_fd)) {
            extern std::tr2::mutex _se_lock;
            extern std::ostream* _se_stream;

            std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
            if ( _se_stream != 0 ) {
              *_se_stream << HERE << ' '
                          << std::make_error_code( static_cast<std::errc>(errno) ).message()
                          << std::endl;
            }

            // descr.erase( ev_add.data.fd );
            static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
            return;
          }
        } else {
          if (!epoll_push(proc_fd)) {
            extern std::tr2::mutex _se_lock;
            extern std::ostream* _se_stream;

            std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
            if ( _se_stream != 0 ) {
              *_se_stream << HERE << ' '
                          << std::make_error_code( static_cast<std::errc>(errno) ).message()
                          << std::endl;
            }
            static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
            return;
          }
        }
        descr[proc_fd] = fd_info( _ctl.cmd == dgram_proc ? fd_info::dgram_proc : fd_info::nonsock_proc,
                                  0, static_cast<socks_processor_t*>(_ctl.data.ptr) );
      }
      static_cast<socks_processor_t*>(_ctl.data.ptr)->release();
      break;
    }
    case nonsock_buffer:
    {
      int nonsock_buffer_fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();

      if ( nonsock_buffer_fd >= 0 ) {
        if (!epoll_push(nonsock_buffer_fd)) {
          extern std::tr2::mutex _se_lock;
          extern std::ostream* _se_stream;

          std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
          if ( _se_stream != 0 ) {
            *_se_stream << HERE << ' '
                        << std::make_error_code( static_cast<std::errc>(errno) ).message()
                        << std::endl;
          }

          descr.erase( nonsock_buffer_fd );

          return; // already closed?
        }
        descr[nonsock_buffer_fd] = fd_info(fd_info::nonsock_buffer,
                                           static_cast<sockbuf_t*>(_ctl.data.ptr), 0);
      }
      break;
    }
    case close_fd:
    {
      int fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();
      if (fd >= 0) {
        for (auto i : descr) {
          sockbuf_t* b = i.second.b;
          if (b != 0) {
            /*
              It's intended for fd_info::nonsock_buffer descriptors only;
              Now it really done for fd_info::nonsock_buffer only:
                basic_sockbuf<charT, traits, _Alloc>::close() {
                  ...
                  if (_type == sock_base::tty) {
                    basic_socket_t::mgr->push_close( *this );
                  }
                  ...
                }
              but let's check flags about fd_info::nonsock_buffer once more.
             */
            if ((b->_fd == fd) && (i.second.flags & fd_info::nonsock_buffer)) {
              std::tr2::lock_guard<std::tr2::recursive_mutex> lk( b->ulck );
              ::close( b->_fd );
              b->_fd = -1;
              b->ucnd.notify_all();
              if (i.second.p != 0) { // if no processing server, it is 0
                (*i.second.p)( i.first, typename socks_processor_t::adopt_close_t() );
              }
              descr.erase(fd);

              return;
            }
          }
        }
      }
      break;
    }
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::close_listener(typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd)
{
  int listener_fd = ifd->first;
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
  ::close( listener_fd ); 
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_listener( const epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  if ( ev.events & (EPOLLHUP | EPOLLERR) ) {
    close_listener(ifd);
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
      if ( i > 0 && ((errno == EAGAIN) || (errno == EINTR) || (errno == ECONNABORTED)) ) { // EWOULDBLOCK == EAGAIN
        errno = 0;
        if (epoll_restore(ev.data.fd)) {
          return; // normal flow, back to epoll
        }
      } else if ( (errno == EMFILE) || (errno == ENFILE) ) {
        // back to listen
        int save_errno = errno;
        errno = 0;
        if (epoll_restore(ev.data.fd)) {
          throw std::system_error( save_errno, std::system_category(), std::string( __PRETTY_FUNCTION__ ) );
        }
      }

      close_listener(ifd);
      return;
    }

    if ( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      ::close( fd );
      throw std::system_error( errno, std::system_category(), std::string( __PRETTY_FUNCTION__ ) );
    }
      
    try {
      if (!epoll_push(fd)) {
        ::close( fd );
        throw std::system_error( errno, std::system_category(), std::string( __PRETTY_FUNCTION__ ) );
      }

      sockbuf_t* b = (*info.p)( fd, addr );
      descr[fd] = fd_info( b, info.p );
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
    }
  }

  // restricted accept, acc_lim reached;
  // then try to return listener back to epoll
  if (!epoll_restore(ev.data.fd)) {
    close_listener(ifd);
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_dgram_srv( const epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  if ( ev.events & (EPOLLHUP | EPOLLERR) ) {
    close_listener(ifd);
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

  if ( len < 0 ) { // epoll notified, no data
    switch ( errno ) {
      default:
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

  (*info.p)( ifd->first, addr );

  if (!epoll_restore(ifd->first)) {
    (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::process_nonsock_srv( const epoll_event& ev, typename sockmgr<charT,traits,_Alloc>::fd_container_type::iterator ifd )
{
  if ( ev.events & (EPOLLHUP | EPOLLERR) ) {
    close_listener(ifd);
    return;
  }

  if ( (ev.events & EPOLLIN) == 0 ) {
    // sockbuf_t* b = (*info.p)( ifd->first, addr );
    return; // I don't know what to do this case...
  }

  fd_info& info = ifd->second;
  sockaddr addr = { AF_UNSPEC };

  // Urgent: don't do ioctl here to check number of available chars for read:
  // it cause of notification via epoll and would lead to infinite loop
#if 0
  int len;
  ioctl(ifd->first, FIONREAD, &len);
  if ( len <= 0 ) {
    if (!epoll_restore(ifd->first)) {
      (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
    }
    return;
  }
#endif

  (*info.p)( ifd->first, addr );

  if (!epoll_restore(ifd->first)) {
    (*info.p)( ifd->first, typename socks_processor_t::adopt_close_t() );
  }
}

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::net_read( typename sockmgr<charT,traits,_Alloc>::sockbuf_t& b )
{
  std::tr2::unique_lock<std::tr2::recursive_mutex> lk( b.ulck, std::tr2::defer_lock_t() );
  if ( lk.try_lock() ) {
    if ( b._fr < b._ebuf ) {
      long offset = ::read( b._fd, b._fr, sizeof(charT) * (b._ebuf - b._fr) );
      if ( offset > 0 ) {
        b._fr += offset / sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!
        if ( (b._fr < b._ebuf) || (b._fl < b.gptr()) ) { // free space available?
          // return back to epoll
          if (!epoll_restore(b._fd)) {
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
          if ( (b._type == std::sock_base::sock_stream) || (b._type == std::sock_base::tty) ) {
            if (!epoll_restore(b._fd)) {
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
            if (!epoll_restore(b._fd)) {
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
              if (!epoll_restore(b._fd)) {
                throw fdclose(); // closed?
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
    if (!epoll_restore(b._fd)) {
      extern std::tr2::mutex _se_lock;
      extern std::ostream* _se_stream;

      std::tr2::lock_guard<std::tr2::mutex> lk(_se_lock);
      if ( _se_stream != 0 ) {
        *_se_stream << HERE << ' '
                    << std::make_error_code( static_cast<std::errc>(errno) ).message()
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
      } else if ( (b->stype() == std::sock_base::sock_dgram) || (b->stype() == std::sock_base::tty) ) {
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

    if ( (ev.events & (EPOLLHUP | EPOLLERR) ) != 0 ) {
      throw fdclose(); // closed connection
    }
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
void sockmgr<charT,traits,_Alloc>::push_proc( socks_processor_t& p, command_type cmd )
{
  ctl _ctl;
  _ctl.cmd = cmd;
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
          throw std::system_error( errno, std::system_category(), std::string( __PRETTY_FUNCTION__ ) );
      }
    }
    r += ret;
  } while ( (r != sizeof(ctl)) );
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
