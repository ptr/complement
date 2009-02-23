// -*- C++ -*- Time-stamp: <09/02/22 22:20:25 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005-2009
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <sockios/netinfo.h>

#if defined(__unix) && !defined(__UCLIBC__) && !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
# include <stropts.h> // for ioctl() call
#endif

#include <fcntl.h>

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const char *name, int port,
                                            sock_base::stype type,
                                            sock_base::protocol prot )
{ return basic_sockbuf<charT, traits, _Alloc>::open( std::findhost( name ), port, type, prot ); }

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const in_addr& addr, int port,
                                            sock_base::stype type,
                                            sock_base::protocol prot )
{
  if ( is_open() ) {
    return 0;
  }
  try {
    _mode = ios_base::in | ios_base::out;
    _type = type;
#ifdef WIN32
    WSASetLastError( 0 );
#endif
    if ( prot == sock_base::inet ) {
      basic_socket_t::_fd = socket( PF_INET, type, 0 );
      if ( basic_socket_t::_fd == -1 ) {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
      basic_socket_t::_address.inet.sin_family = AF_INET;
      // htons is a define at least in Linux 2.2.5-15, and it's expantion fail
      // for gcc 2.95.3
#if defined(linux) && defined(htons) && defined(__bswap_16)
      basic_socket_t::_address.inet.sin_port = ((((port) >> 8) & 0xff) | (((port) & 0xff) << 8));
#else
      basic_socket_t::_address.inet.sin_port = htons( port );
#endif // linux && htons
      basic_socket_t::_address.inet.sin_addr = addr;
  
      // Generally, stream sockets may successfully connect() only once
      if ( connect( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof( basic_socket_t::_address ) ) == -1 ) {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
      if ( type == sock_base::sock_stream ) {
        _xwrite = &_Self_type::write;
        _xread = &_Self_type::read;
      } else if ( type == sock_base::sock_dgram ) {
        _xwrite = &_Self_type::send;
        _xread = &_Self_type::recv;
      }
    } else if ( prot == sock_base::local ) {
      basic_socket_t::_fd = socket( PF_UNIX, type, 0 );
      if ( basic_socket_t::_fd == -1 ) {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
    } else { // other protocols not implemented yet
      throw std::invalid_argument( "protocol not implemented" );
    }

    if ( _bbuf == 0 ) {
      struct ifconf ifc;
      struct ifreq  ifr;
      ifc.ifc_len = sizeof(ifreq);
      ifc.ifc_req = &ifr;
      int mtu = ((ioctl(basic_socket_t::_fd, SIOCGIFMTU, &ifc) < 0 ? 1500 : ifr.ifr_mtu) - 20 - (type == sock_base::sock_stream ? 20 : 8 )) / sizeof(charT);
      int qlen = ioctl(basic_socket_t::_fd, SIOCGIFTXQLEN, &ifc) < 0 ? 2 : ifr.ifr_qlen;
      _M_allocate_block( type == sock_base::sock_stream ? mtu * qlen * 2 : mtu * 2 );
    }

    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

    if ( fcntl( basic_socket_t::_fd, F_SETFL, fcntl( basic_socket_t::_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }
    setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );

    std::tr2::lock_guard<std::tr2::mutex> lk( ulck );
    setg( this->epptr(), this->epptr(), this->epptr() );
    basic_socket_t::mgr->push( *this );
  }
  catch ( std::system_error& ) {
    if ( basic_socket_t::_fd != -1 ) {
#ifdef WIN32
      // _errno = WSAGetLastError();
      ::closesocket( basic_socket_t::_fd );
#else
      ::close( basic_socket_t::_fd );
#endif
      basic_socket_t::_fd = -1;
    }
   
    // std::tr2::lock_guard<std::tr2::mutex> lk( ulck );
    // ucnd.notify_all();

    return 0;
  }
  catch ( std::length_error& ) {
#ifdef WIN32
    ::closesocket( basic_socket_t::_fd );
#else
    ::close( basic_socket_t::_fd );
#endif
    basic_socket_t::_fd = -1;

    // std::tr2::lock_guard<std::tr2::mutex> lk( ulck );
    // ucnd.notify_all();

    return 0;
  }
  catch ( std::runtime_error& ) {
#ifdef WIN32
    // _errno = WSAGetLastError();
#else
#endif
    return 0;
  }
  catch ( std::invalid_argument& ) {
    return 0;
  }

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( sock_base::socket_type s, sock_base::stype t )
{
  if ( /* basic_socket_t:: */ is_open() || s == -1 ) {
    return 0;
  }

  sockaddr sa;
  socklen_t sl = sizeof(sa);
  getsockname( s, &sa, &sl );

  return basic_sockbuf<charT, traits, _Alloc>::open( s, sa, t );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( sock_base::socket_type s,
                                            const sockaddr& addr,
                                            sock_base::stype t )
{
  basic_sockbuf<charT, traits, _Alloc>* ret = _open_sockmgr( s, addr, t );
  if ( ret != 0 ) {
    basic_socket_t::mgr->push( *this );
  }
  return ret;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::attach( sock_base::socket_type s,
                                              sock_base::stype t )
{
  if ( basic_socket_t::is_open() || s == -1 ) {
    return 0;
  }

  sockaddr sa;
  socklen_t sl = sizeof(sa);
  getsockname( s, &sa, &sl );

  return basic_sockbuf<charT, traits, _Alloc>::attach( s, sa, t );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::attach( sock_base::socket_type s,
                                              const sockaddr& addr,
                                              sock_base::stype t )
{
  if ( basic_socket_t::is_open() || s == -1 ) {
    return 0;
  }

  return basic_sockbuf<charT, traits, _Alloc>::open( dup(s), addr, t );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::_open_sockmgr( sock_base::socket_type s,
                                                     const sockaddr& addr,
                                                     sock_base::stype t )
{
  std::tr2::lock_guard<std::tr2::mutex> lk( ulck );

  if ( basic_socket_t::is_open_unsafe() || s == -1 ) {
    return 0;
  }

  basic_socket_t::_fd = s;
  memcpy( (void *)&basic_socket_t::_address.any, (const void *)&addr, sizeof(sockaddr) );
  _mode = ios_base::in | ios_base::out;
  _type = t;
#ifdef WIN32
  WSASetLastError( 0 );
#endif
  if ( t == sock_base::sock_stream ) {
    _xwrite = &_Self_type::write;
    _xread = &_Self_type::read;
  } else if ( t == sock_base::sock_dgram ) {
    _xwrite = &_Self_type::sendto;
    _xread = &_Self_type::recvfrom;
  } else {
    basic_socket_t::_fd = -1;
    return 0; // unsupported type
  }

  if ( _bbuf == 0 ) {
    struct ifconf ifc;
    struct ifreq  ifr;
    ifc.ifc_len = sizeof(ifreq);
    ifc.ifc_req = &ifr;
    int mtu = ((ioctl(basic_socket_t::_fd, SIOCGIFMTU, &ifc) < 0 ? 1500 : ifr.ifr_mtu) - 20 - (t == sock_base::sock_stream ? 20 : 8 )) / sizeof(charT);
    int qlen = ioctl(basic_socket_t::_fd, SIOCGIFTXQLEN, &ifc) < 0 ? 2 : ifr.ifr_qlen;
    _M_allocate_block( t == sock_base::sock_stream ? mtu * qlen * 2 : mtu * 2);
  }

  if ( _bbuf == 0 ) {
#ifdef WIN32
    ::closesocket( basic_socket_t::_fd );
#else
    ::close( basic_socket_t::_fd );
#endif
    basic_socket_t::_fd = -1;

    // std::tr2::lock_guard<std::tr2::mutex> lk( ulck );
    // ucnd.notify_all();

    return 0;
  }

  if ( fcntl( basic_socket_t::_fd, F_SETFL, fcntl( basic_socket_t::_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
    ::close( basic_socket_t::_fd );
    basic_socket_t::_fd = -1;
    throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>" ) );
  }
  setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );

  setg( this->epptr(), this->epptr(), this->epptr() );

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::close()
{
  std::tr2::unique_lock<std::tr2::mutex> lk( ulck );

  if ( !basic_socket_t::is_open_unsafe() ) {
    return 0;
  }
  
  // int x = basic_socket_t::_fd;
  shutdown_unsafe( sock_base::stop_in | sock_base::stop_out );

  // std::cerr << __FILE__ << ':' << __LINE__ << ' ' << basic_socket_t::_fd << ' ' << std::tr2::getpid() << std::endl;
  // xmt::callstack( cerr );

  // basic_socket_t::mgr->exit_notify( this, basic_socket_t::_fd );

  // shutdown_unsafe( sock_base::stop_in | sock_base::stop_out );

  // put area before get area
  //setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );
  //setg( this->epptr(), this->epptr(), this->epptr() );

  // if ( basic_socket_t::_fd != -1 ) {
  // std::cerr << __FILE__ << ':' << __LINE__ << ' ' << basic_socket_t::_fd << std::endl;
  // }

  ucnd.wait( lk, closed_t( *this ) );
  // std::cerr << __FILE__ << ':' << __LINE__ << ' ' << basic_socket_t::_fd << ' ' << x << std::endl;  
  // std::cerr << __FILE__ << ':' << __LINE__ << ' ' << basic_socket_t::_fd << std::endl;

  return this;
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::shutdown_unsafe( sock_base::shutdownflg dir )
{
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
__FIT_TYPENAME basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::underflow()
{
  std::tr2::unique_lock<std::tr2::mutex> lk( ulck );

  if( !basic_socket_t::is_open_unsafe() ) {
    return traits::eof();
  }

  if ( this->gptr() < this->egptr() ) {
    return traits::to_int_type(*this->gptr());
  }

  if ( this->egptr() == this->gptr() ) { // fullfilled: _ebuf == gptr()
    setg( this->eback(), this->eback(), this->eback() );
  }

  // setg( this->eback(), this->eback(), this->eback() + offset );
  // wait on condition
  if ( basic_socket_t::_use_rdtimeout ) {
    if ( !ucnd.timed_wait( lk, basic_socket_t::_rdtimeout, rdready ) ) {
      return traits::eof();
    }
  } else {
    ucnd.wait( lk, rdready );
  }

  return this->gptr() < this->egptr() ? traits::to_int_type(*this->gptr()) : traits::eof();
}

template<class charT, class traits, class _Alloc>
__FIT_TYPENAME basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::overflow( int_type c )
{
  if ( !is_open() ) {
    return traits::eof();
  }

  if ( !traits::eq_int_type( c, traits::eof() ) && this->pptr() < this->epptr() ) {
    sputc( traits::to_char_type(c) );
    return c;
  }

  long count = this->pptr() - this->pbase();

  if ( count ) {
    count *= sizeof(charT);
    long offset;
    std::tr2::nanoseconds sl( 5000ULL );
    do {
      offset = (this->*_xwrite)( this->pbase(), count );
      if ( offset < 0 ) {
        if ( (errno != EAGAIN) && (errno != EINTR) ) {
          return traits::eof();
        }
        errno = 0;
        std::tr2::this_thread::sleep( sl );
        if ( sl < 30000000000ULL ) {
          sl *= 2;
        }
      }
    } while ( offset < 0 );

    if ( offset < count ) {
      // MUST BE: (offset % sizeof(char_traits)) == 0 !
      offset /= sizeof(charT);
      count /= sizeof(charT);
      traits::move( this->pbase(), this->pbase() + offset, count - offset );
      // std::copy_backword( this->pbase() + offset, this->pbase() + count, this->pbase() );
      setp( this->pbase(), this->epptr() ); // require: set pptr
      this->pbump( count - offset );
      if( !traits::eq_int_type(c,traits::eof()) ) {
        sputc( traits::to_char_type(c) );
      }

      return traits::not_eof(c);
    }
  }

  setp( this->pbase(), this->epptr() ); // require: set pptr
  if( !traits::eq_int_type(c,traits::eof()) ) {
    sputc( traits::to_char_type(c) );
  }

  return traits::not_eof(c);
}

template<class charT, class traits, class _Alloc>
int basic_sockbuf<charT, traits, _Alloc>::sync()
{
  if ( !is_open() ) {
    return -1;
  }

  long count = this->pptr() - this->pbase();
  if ( count ) {
    count *= sizeof(charT);
    long start = 0L;
    long offset = 0L;
    while ( count > 0 ) {

      std::tr2::nanoseconds sl( 5000ULL );
      do {
        offset = (this->*_xwrite)( this->pbase() + start, count );
        if ( offset < 0 ) {
          if ( (errno != EAGAIN) && (errno != EINTR) ) {
            return traits::eof();
          }
          errno = 0;
          std::tr2::this_thread::sleep( sl );
          if ( sl < 30000000000ULL ) {
            sl *= 2;
          }
        }
      } while ( offset < 0 );

      count -= offset;
      start += offset;
    }
    setp( this->pbase(), this->epptr() ); // require: set pptr
  }

  return 0;
}

template<class charT, class traits, class _Alloc>
streamsize basic_sockbuf<charT, traits, _Alloc>::xsputn( const char_type *s, streamsize n )
{
  if ( !/* basic_socket_t:: */ is_open() || s == 0 || n == 0 ) {
    return 0;
  }

  streamsize __n_put = this->epptr() - this->pptr();

  if ( __n_put > n ) {
    traits::copy( this->pptr(), s, n );
    this->pbump( n );
    return n;
  }

  if ( __n_put == 0 ) {
    if ( traits::eq_int_type(overflow(),traits::eof()) ) {
      return 0;
    }
    __n_put = this->epptr() - this->pptr();
  }

  streamsize count = 0;

  while ( __n_put < n ) { // __n_put > 0 here
    traits::copy( this->pptr(), s, __n_put );
    this->pbump( __n_put );
    n -= __n_put;
    s += __n_put;
    count += __n_put;
    if ( traits::eq_int_type(overflow(),traits::eof()) ) {
      return count;
    }
    __n_put = this->epptr() - this->pptr();
  }

  traits::copy( this->pptr(), s, n );
  this->pbump( n );
  count += n;

  return count;
}

template<class charT, class traits, class _Alloc>
int basic_sockbuf<charT, traits, _Alloc>::recvfrom( void *buf, size_t n )
{
#if defined(_WIN32) || (defined(__hpux) && !defined(_INCLUDE_POSIX1C_SOURCE))
  int sz = sizeof( sockaddr_in );
#else
  socklen_t sz = sizeof( sockaddr_in );
#endif

  typename basic_socket_t::sockaddr_t addr;

#ifdef __FIT_POLL
  pollfd pfd;
  pfd.fd = basic_socket_t::_fd;
  pfd.events = POLLIN;
#endif // __FIT_POLL
  do {
#ifdef __FIT_POLL
    pfd.revents = 0;
    if ( poll( &pfd, 1, /* _timeout */ -1 ) > 0 ) { // wait infinite
      // get address of caller only
      char buff[32];    
      ::recvfrom( basic_socket_t::_fd, buff, 32, MSG_PEEK, &addr.any, &sz );
    } else {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#endif // __FIT_POLL
    if ( memcmp( &basic_socket_t::_address.inet, &addr.inet, sizeof(sockaddr_in) ) == 0 ) {
#ifdef WIN32
      return ::recvfrom( basic_socket_t::_fd, (char *)buf, n, 0, &basic_socket_t::_address.any, &sz );
#else
      return ::recvfrom( basic_socket_t::_fd, buf, n, 0, &basic_socket_t::_address.any, &sz );
#endif
    }
  } while ( true );

  return 0; // never
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::setoptions( sock_base::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  std::tr2::lock_guard<std::tr2::mutex> lk( ulck );

  if ( basic_socket_t::is_open_unsafe() ) {
    int turn = on_off ? 1 : 0;
    int ret = 0;
    switch ( optname ) {
      case sock_base::so_debug:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_DEBUG, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_keepalive:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_KEEPALIVE, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_linger:
        {
          linger l;
          l.l_onoff = on_off ? 1 : 0;
          l.l_linger = __v;
          ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_LINGER, (const void *)&l, (socklen_t)sizeof(linger) );
        }
        break;
      case sock_base::so_oobinline:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_OOBINLINE, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_nodelay:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_NODELAY, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_cork:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_CORK, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_quickack:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_QUICKACK, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
#ifdef TCP_CONGESTION
      case sock_base::so_tcp_congestion:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_CONGESTION, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
#endif
      default:
        throw std::invalid_argument( "bad socket option" );
    }
    if ( ret != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "socket option" ) );
    }
  } else {
    throw std::invalid_argument( "socket is closed" );
  }
#endif // __unix
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::setoptions( sock_base::so_t optname, int __v )
{
#ifdef __unix
  std::tr2::lock_guard<std::tr2::mutex> lk( ulck );

  if ( basic_socket_t::is_open_unsafe() ) {
    int ret = 0;
    switch ( optname ) {
      case sock_base::so_sndbuf:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_SNDBUF, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_rcvbuf:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_RCVBUF, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_sndlowat:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_SNDLOWAT, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_rcvlowat:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_RCVLOWAT, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_sndtimeo:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_SNDTIMEO, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_rcvtimeo:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_maxseg:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_MAXSEG, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_keepidle:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_KEEPIDLE, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_keepintvl:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_KEEPINTVL, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_keepcnt:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_KEEPCNT, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_syncnt:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_SYNCNT, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_linger2:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_LINGER2, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_window_clamp:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_WINDOW_CLAMP, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      default:
        throw std::invalid_argument( "bad socket option" );
    }
    if ( ret != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "socket option" ) );
    }
  } else {
    throw std::invalid_argument( "socket is closed" );
  }
#endif // __unix
}

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif

