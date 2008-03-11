// -*- C++ -*- Time-stamp: <08/03/07 01:40:59 ptr>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __SOCKIOS_SP_H
#define __SOCKIOS_SP_H

#include <sys/epoll.h>

#ifndef EPOLLRDHUP
#  define EPOLLRDHUP 0x2000
#endif

#include <fcntl.h>

#include <cerrno>
#include <mt/thread>
#include <mt/mutex>

#ifdef STLPORT
#  include <unordered_map>
#  include <unordered_set>
// #  include <hash_map>
// #  include <hash_set>
// #  define __USE_STLPORT_HASH
#  define __USE_STLPORT_TR1
#else
#  if defined(__GNUC__) && (__GNUC__ < 4)
#    include <ext/hash_map>
#    include <ext/hash_set>
#    define __USE_STD_HASH
#  else
#    include <tr1/unordered_map>
#    include <tr1/unordered_set>
#    define __USE_STD_TR1
#  endif
#endif

#include <sockios/sockstream>

namespace std {

template <class charT, class traits, class _Alloc> class basic_sockbuf2;
template <class charT, class traits, class _Alloc> class basic_sockstream2;
template <class charT, class traits, class _Alloc> class sock_processor_base;

namespace detail {

template<class charT, class traits, class _Alloc>
class _sock_processor_base :
	public sock_base2,
        public basic_socket<charT,traits,_Alloc>
{
  private:
    typedef basic_socket<charT,traits,_Alloc> basic_socket_t;

  protected:
    _sock_processor_base() :
        _mode( ios_base::in | ios_base::out ),
        _state( ios_base::goodbit )
      { }

    virtual ~_sock_processor_base()
      {
        _sock_processor_base::close();
      }

  public:
    void open( const in_addr& addr, int port, sock_base2::stype type, sock_base2::protocol prot );
    void open( unsigned long addr, int port, sock_base2::stype type, sock_base2::protocol prot );
    void open( int port, sock_base2::stype type, sock_base2::protocol prot );

    virtual void close();

  protected:
    void setoptions_unsafe( sock_base2::so_t optname, bool on_off = true, int __v = 0 );

  public:
    bool is_open() const
      { std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck); return basic_socket_t::is_open_unsafe(); }
    bool good() const
      { return _state == ios_base::goodbit; }

    sock_base2::socket_type fd() const
      { std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck); sock_base2::socket_type tmp = basic_socket_t::fd_unsafe(); return tmp; }

    void shutdown( sock_base2::shutdownflg dir );
    void setoptions( sock_base2::so_t optname, bool on_off = true, int __v = 0 )
      {
        std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
        setoptions_unsafe( optname, on_off, __v );
      }

  private:
    _sock_processor_base( const _sock_processor_base& );
    _sock_processor_base& operator =( const _sock_processor_base& );

  private:
    unsigned long _mode;  // open mode
    unsigned long _state; // state flags

  protected:
    std::tr2::mutex _fd_lck;
    // xmt::condition _loop_cnd;
};

template<class charT, class traits, class _Alloc>
void _sock_processor_base<charT,traits,_Alloc>::open( const in_addr& addr, int port, sock_base2::stype type, sock_base2::protocol prot )
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
      basic_socket_t::mgr->push( dynamic_cast<sock_processor_base<charT,traits,_Alloc>&>(*this) );
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
void _sock_processor_base<charT,traits,_Alloc>::open( unsigned long addr, int port, sock_base2::stype type, sock_base2::protocol prot )
{
  in_addr _addr;
  _addr.s_addr = htonl( addr );
  _sock_processor_base::open( _addr, port, type, prot );
}

template<class charT, class traits, class _Alloc>
void _sock_processor_base<charT,traits,_Alloc>::open( int port, sock_base2::stype type, sock_base2::protocol prot )
{
  _sock_processor_base::open(INADDR_ANY, port, type, prot);
}

template<class charT, class traits, class _Alloc>
void _sock_processor_base<charT,traits,_Alloc>::close()
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( !basic_socket_t::is_open_unsafe() ) {
    return;
  }
#ifdef WIN32
  ::closesocket( basic_socket_t::_fd );
#else
  ::shutdown( basic_socket_t::_fd, 2 );
  ::close( basic_socket_t::_fd );
#endif
  basic_socket_t::_fd = -1;
}

template<class charT, class traits, class _Alloc>
void _sock_processor_base<charT,traits,_Alloc>::shutdown( sock_base2::shutdownflg dir )
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
void _sock_processor_base<charT,traits,_Alloc>::setoptions_unsafe( sock_base2::so_t optname, bool on_off, int __v )
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

} // namespace detail

template <class charT, class traits, class _Alloc>
class sock_processor_base :
        public detail::_sock_processor_base<charT,traits,_Alloc>
{
  private:
    typedef detail::_sock_processor_base<charT,traits,_Alloc> sp_base_t;

  public:
    typedef basic_sockstream2<charT,traits,_Alloc> sockstream_t;

    struct adopt_new_t { };
    struct adopt_close_t { };
    struct adopt_data_t { };

    sock_processor_base()
      { }

    explicit sock_processor_base( int port, sock_base2::stype t = sock_base2::sock_stream )
      {
        sp_base_t::open( port, t, sock_base2::inet );
      }


    virtual void operator ()( sockstream_t& s, const adopt_new_t& ) = 0;
    virtual void operator ()( sockstream_t& s, const adopt_close_t& ) = 0;
    virtual void operator ()( sockstream_t& s, const adopt_data_t& ) = 0;
};

typedef sock_processor_base<char,char_traits<char>,allocator<char> > sock_basic_processor;

namespace detail {

template<class charT, class traits, class _Alloc>
class sockmgr
{
  private:
    typedef basic_sockstream2<charT,traits,_Alloc> sockstream_t;
    typedef basic_sockbuf2<charT,traits,_Alloc> sockbuf_t;
    typedef sock_processor_base<charT,traits,_Alloc> socks_processor_t;

    enum {
      listener,
      // tcp_stream,
      tcp_buffer,
      rqstop,
      rqstart
    };

    struct fd_info
    {
        enum {
          listener = 0x1,
          level_triggered = 0x2,
          owner = 0x4,
          buffer = 0x8
        };

        unsigned flags;
        union {
          sockstream_t* s;
          sockbuf_t*    b;
        } s;
        socks_processor_t *p;
    };

    struct ctl {
        int cmd;
        union {
            int fd;
            void *ptr;
        } data;
    };

    static void _loop( sockmgr *me )
      { me->io_worker(); }

  public:
    sockmgr( int hint = 1024, int ret = 512 ) :
         n_ret( ret )
      {
        efd = epoll_create( hint );
        if ( efd < 0 ) {
          // throw system_error( errno )
        }
        pipe( pipefd ); // check err
        // cfd = pipefd[1];

        epoll_event ev_add;
        ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
        ev_add.data.fd = pipefd[0];
        epoll_ctl( efd, EPOLL_CTL_ADD, pipefd[0], &ev_add );

        _worker = new std::tr2::thread( _loop, this );

        // ctl _ctl;
        // _ctl.cmd = rqstart;

        // write( pipefd[1], &_ctl, sizeof(ctl) );
      }

    ~sockmgr()
      {
        if ( _worker->joinable() ) {
          ctl _ctl;
          _ctl.cmd = rqstop;

          write( pipefd[1], &_ctl, sizeof(ctl) );

          _worker->join();
        }
        close( pipefd[1] );
        close( pipefd[0] );
        close( efd );
        delete _worker;
      }

    void push( socks_processor_t& p )
      {
        ctl _ctl;
        _ctl.cmd = listener;
        _ctl.data.ptr = static_cast<void *>(&p);

        write( pipefd[1], &_ctl, sizeof(ctl) );
      }

#if 0
    void push( sockstream_t& s )
      {
        ctl _ctl;
        _ctl.cmd = tcp_stream;
        _ctl.data.ptr = static_cast<void *>(&s);

        write( pipefd[1], &_ctl, sizeof(ctl) );
      }
#endif

    void push( sockbuf_t& s )
      {
        ctl _ctl;
        _ctl.cmd = tcp_buffer;
        _ctl.data.ptr = static_cast<void *>(&s);

        errno = 0;
        int r = write( pipefd[1], &_ctl, sizeof(ctl) );
      }

    void exit_notify( sockbuf_t* b, int fd )
      {
        fd_info info = { 0, reinterpret_cast<sockstream_t*>(b), 0 };
        std::tr2::lock_guard<std::tr2::mutex> lk( cll );
	closed_queue[fd] = info;
      }

  private:
    sockmgr( const sockmgr& )
      { }
    sockmgr& operator =( const sockmgr& )
      { return *this; }

    void io_worker();

    int efd;
    int pipefd[2];
    std::tr2::thread *_worker;
    const int n_ret;

#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<int,fd_info> fd_container_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<int, fd_info> fd_container_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<int, fd_info> fd_container_type;
#endif

    fd_container_type descr;
    fd_container_type closed_queue;
    std::tr2::mutex cll;
};

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::io_worker()
{
  epoll_event ev[n_ret];

/*
  ctl _xctl;
  int r = read( pipefd[0], &_xctl, sizeof(ctl) );

  if ( _xctl.cmd == rqstart ) {
    std::cerr << "io_worker fine" << std::endl;
  } else {
    std::cerr << "io_worker not fine, " << r << ", " << errno << std::endl;
  }
*/

  for ( ; ; ) {
    int n = epoll_wait( efd, &ev[0], n_ret, -1 );
    if ( n < 0 ) {
      if ( errno == EINTR ) {
        continue;
      }
      // throw system_error
    }
    for ( int i = 0; i < n; ++i ) {
      if ( ev[i].data.fd == pipefd[0] ) {
        epoll_event ev_add;
        ctl _ctl;
        int r = read( pipefd[0], &_ctl, sizeof(ctl) );
        if ( r < 0 ) {
          // throw system_error
        } else if ( r == 0 ) {
          return;
        }

        switch ( _ctl.cmd ) {
          case listener:
            ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
            ev_add.data.fd = static_cast<socks_processor_t*>(_ctl.data.ptr)->fd();
            if ( ev_add.data.fd >= 0 ) {
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
            return;
            break;
        }

        continue;
      }

      typename fd_container_type::iterator ifd = descr.find( ev[i].data.fd );
      if ( ifd == descr.end() ) {
        throw std::logic_error( "file descriptor in epoll, but not in descr[]" );
      }

      fd_info& info = ifd->second;
      if ( info.flags & fd_info::listener ) {
        if ( ev[i].events & EPOLLRDHUP ) {
          epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 );
          // walk through descr and detach every .p ?
          descr.erase( ifd );
        } else if ( ev[i].events & EPOLLIN ) {
          sockaddr addr;
          socklen_t sz = sizeof( sockaddr_in );

          int fd = accept( ev[i].data.fd, &addr, &sz );
          if ( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
            throw std::runtime_error( "can't establish nonblock mode" );
          }
          sockstream_t* s;

          try {
            s = new sockstream_t();
            if ( s->rdbuf()->_open_sockmgr( fd, addr ) ) {
              epoll_event ev_add;
              ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
              ev_add.data.fd = fd;
              fd_info new_info = { fd_info::owner, s, info.p };
              descr[fd] = new_info;

              if ( epoll_ctl( efd, EPOLL_CTL_ADD, fd, &ev_add ) < 0 ) {
                std::cerr << "Accept, add " << fd << ", errno " << errno << std::endl;
                descr.erase( fd );
                // throw system_error
              }
              (*info.p)( *s, typename socks_processor_t::adopt_new_t() );
            } else {
              std::cerr << "Accept, delete " << fd << std::endl;
              delete s;
            }
          }
          catch ( const std::bad_alloc& ) {
            // nothing
          }
          catch ( ... ) {
            descr.erase( fd );
            delete s;
          }
        }
      } else {
        if ( ev[i].events & EPOLLIN ) {
          if ( (info.flags & fd_info::owner) == 0 ) {
            // marginal case: me not owner (registerd via push(),
            // when I owner, I know destroy point),
            // already closed, but I not see closed event yet;
            // object may be deleted already, so I can't
            // call b->egptr() etc. here
            std::tr2::lock_guard<std::tr2::mutex> lck( cll );
            typename fd_container_type::iterator closed_ifd = closed_queue.find( ev[i].data.fd );
            if ( closed_ifd != closed_queue.end() ) {
              closed_queue.erase( closed_ifd );
              if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
                // throw system_error
              }
              descr.erase( ifd );
              continue;
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
                xev.data.fd = ev[i].data.fd;
                info.flags |= fd_info::level_triggered;
                if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev[i].data.fd, &xev ) < 0 ) {
                  std::cerr << "X " << ev[i].data.fd << ", " << errno << std::endl;
                }
              }
              std::cerr << "Z " << ev[i].data.fd << ", " << errno << std::endl;
              if ( info.p != 0 ) {
                (*info.p)( *info.s.s, typename socks_processor_t::adopt_data_t() );
              }
              break;
            }
            long offset = read( ev[i].data.fd, b->egptr(), sizeof(charT) * (b->_ebuf - b->egptr()) );
            if ( offset < 0 ) {
              if ( errno == EAGAIN ) {
                errno = 0;
                epoll_event xev;
                xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
                xev.data.fd = ev[i].data.fd;
                epoll_ctl( efd, EPOLL_CTL_MOD, ev[i].data.fd, &xev );
                break;
              } else {
                // process error
                std::cerr << "not listener, other " << ev[i].data.fd << std::hex << ev[i].events << std::dec << errno << std::endl;
              }
            } else if ( offset > 0 ) {
              offset /= sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!
            
              if ( info.flags & fd_info::level_triggered ) {
                epoll_event xev;
                xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
                xev.data.fd = ev[i].data.fd;
                info.flags &= ~static_cast<unsigned>(fd_info::level_triggered);
                if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev[i].data.fd, &xev ) < 0 ) {
                  std::cerr << "Y " << ev[i].data.fd << ", " << errno << std::endl;
                }
              }
              std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
              b->setg( b->eback(), b->gptr(), b->egptr() + offset );
              b->ucnd.notify_one();
              if ( info.p != 0 ) {
                (*info.p)( *info.s.s, typename socks_processor_t::adopt_data_t() );
              }
            } else {
              // std::cerr << "K " << ev[i].data.fd << ", " << errno << std::endl;
              // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
              ev[i].events |= EPOLLRDHUP; // will be processed below
              break;
            }
          }
        }
        if ( ev[i].events & EPOLLRDHUP ) {
          // std::cerr << "Poll EPOLLRDHUP " << ev[i].data.fd << ", " << errno << std::endl;
          if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
            // throw system_error
          }
          if ( info.p != 0 ) {
            (*info.p)( *info.s.s, typename socks_processor_t::adopt_close_t() );
          }
          if ( (info.flags & fd_info::owner) != 0 ) {
            delete info.s.s;
          } else {
            if ( (info.flags & fd_info::buffer) != 0 ) {
              info.s.b->close();
            } else {
              info.s.s->close();
            }
            std::tr2::lock_guard<std::tr2::mutex> lck( cll );
            closed_queue.erase( ev[i].data.fd );
          }
          descr.erase( ifd );
        }
        if ( ev[i].events & EPOLLHUP ) {
          std::cerr << "Poll HUP" << std::endl;
        }
        if ( ev[i].events & EPOLLERR ) {
          std::cerr << "Poll ERR" << std::endl;
        }
        if ( ev[i].events & EPOLLERR ) {
          std::cerr << "Poll ERR" << std::endl;
        }
        if ( ev[i].events & EPOLLPRI ) {
          std::cerr << "Poll PRI" << std::endl;
        }
        if ( ev[i].events & EPOLLRDNORM ) {
          std::cerr << "Poll RDNORM" << std::endl;
        }
        if ( ev[i].events & EPOLLRDBAND ) {
          std::cerr << "Poll RDBAND" << std::endl;
        }
        if ( ev[i].events & EPOLLMSG ) {
          std::cerr << "Poll MSG" << std::endl;
        }
      }
    }
  }
}

} //detail

} // namesapce std

#ifdef __USE_STLPORT_HASH
#  undef __USE_STLPORT_HASH
#endif
#ifdef __USE_STD_HASH
#  undef __USE_STD_HASH
#endif
#ifdef __USE_STLPORT_TR1
#  undef __USE_STLPORT_TR1
#endif
#ifdef __USE_STD_TR1
#  undef __USE_STD_TR1
#endif

#endif /* __SOCKIOS_SP_H */
