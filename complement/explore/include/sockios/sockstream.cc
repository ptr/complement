// -*- C++ -*- Time-stamp: <07/09/06 23:48:33 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005-2007
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifdef __unix
extern "C" int nanosleep(const struct timespec *, struct timespec *);
#endif

#if defined(__unix) && !defined(__UCLIBC__) && !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
# include <stropts.h> // for ioctl() call
#endif


#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const char *name, int port,
                                            sock_base::stype type,
                                            sock_base::protocol prot,
                                            const timespec *timeout )
{
  if ( is_open() ) {
    return 0;
  }
  try {
    _mode = ios_base::in | ios_base::out;
    _errno = 0;
    _type = type;
#ifdef WIN32
    WSASetLastError( 0 );
#endif
#ifdef __FIT_POLL
    if ( timeout != 0 ) {
      _timeout = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000;
    } else {
      _timeout = -1;
    }
#endif
#ifdef __FIT_SELECT
    if ( timeout != 0 ) {
      _timeout.tv_sec = timeout->tv_sec;
      _timeout.tv_usec = timeout->tv_nsec / 1000;
      _timeout_ref = &_timeout;
    } else {
      _timeout_ref = 0;
    }
#endif
    if ( prot == sock_base::inet ) {
      _fd = socket( PF_INET, type, 0 );
      if ( _fd == -1 ) {
        throw std::runtime_error( "can't open socket" );
      }
      _address.inet.sin_family = AF_INET;
      // htons is a define at least in Linux 2.2.5-15, and it's expantion fail
      // for gcc 2.95.3
#if defined(linux) && defined(htons) && defined(__bswap_16)
      _address.inet.sin_port = ((((port) >> 8) & 0xff) | (((port) & 0xff) << 8));
#else
      _address.inet.sin_port = htons( port );
#endif // linux && htons
      _address.inet.sin_addr = std::findhost( name );
	  
      // Generally, stream sockets may successfully connect() only once
      if ( connect( _fd, &_address.any, sizeof( _address ) ) == -1 ) {
        throw std::domain_error( "connect fail" );
      }
      if ( type == sock_base::sock_stream ) {
        _xwrite = &_Self_type::write;
        _xread = &_Self_type::read;
      } else if ( type == sock_base::sock_dgram ) {
        _xwrite = &_Self_type::send;
        _xread = &_Self_type::recv;
      }
    } else if ( prot == sock_base::local ) {
      _fd = socket( PF_UNIX, type, 0 );
      if ( _fd == -1 ) {
        throw std::runtime_error( "can't open socket" );
      }
    } else { // other protocols not implemented yet
      throw std::invalid_argument( "protocol not implemented" );
    }
    if ( _bbuf == 0 )
      _M_allocate_block( type == sock_base::sock_stream ? 0xb00 : 0xffff ); // max 1460 (dec) [0x5b4] --- single segment
    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

#ifdef __FIT_NONBLOCK_SOCKETS
    if ( fcntl( _fd, F_SETFL, fcntl( _fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      throw std::runtime_error( "can't establish nonblock mode" );
    }
#endif
#ifdef __FIT_POLL
    pfd.fd = _fd;
    pfd.events = POLLIN | POLLHUP | POLLRDNORM;
#endif
    setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );
    setg( this->epptr(), this->epptr(), this->epptr() );

    _errno = 0; // if any
  }
  catch ( std::domain_error& ) {
#ifdef WIN32
    _errno = WSAGetLastError();
    ::closesocket( _fd );
#else
    _errno = errno;
    ::close( _fd );
#endif
    _fd = -1;
    return 0;
  }
  catch ( std::length_error& ) {
#ifdef WIN32
    ::closesocket( _fd );
#else
    ::close( _fd );
#endif
    _fd = -1;
    return 0;
  }
  catch ( std::runtime_error& ) {
#ifdef WIN32
    _errno = WSAGetLastError();
#else
    _errno = errno;
#endif
    return 0;
  }
  catch ( std::invalid_argument& ) {
    return 0;
  }

  return this;
}

// --
template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const in_addr& addr, int port,
                                            sock_base::stype type,
                                            sock_base::protocol prot,
                                            const timespec *timeout )
{
  if ( is_open() ) {
    return 0;
  }
  try {
    _mode = ios_base::in | ios_base::out;
    _errno = 0;
    _type = type;
#ifdef WIN32
    WSASetLastError( 0 );
#endif
#ifdef __FIT_POLL
    if ( timeout != 0 ) {
      _timeout = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000;
    } else {
      _timeout = -1;
    }
#endif
#ifdef __FIT_SELECT
    if ( timeout != 0 ) {
      _timeout.tv_sec = timeout->tv_sec;
      _timeout.tv_usec = timeout->tv_nsec / 1000;
      _timeout_ref = &_timeout;
    } else {
      _timeout_ref = 0;
    }
#endif
    if ( prot == sock_base::inet ) {
      _fd = socket( PF_INET, type, 0 );
      if ( _fd == -1 ) {
        throw std::runtime_error( "can't open socket" );
      }
      _address.inet.sin_family = AF_INET;
      // htons is a define at least in Linux 2.2.5-15, and it's expantion fail
      // for gcc 2.95.3
#if defined(linux) && defined(htons) && defined(__bswap_16)
      _address.inet.sin_port = ((((port) >> 8) & 0xff) | (((port) & 0xff) << 8));
#else
      _address.inet.sin_port = htons( port );
#endif // linux && htons
      _address.inet.sin_addr = addr;
  
      // Generally, stream sockets may successfully connect() only once
      if ( connect( _fd, &_address.any, sizeof( _address ) ) == -1 ) {
        throw std::domain_error( "connect fail" );
      }
      if ( type == sock_base::sock_stream ) {
        _xwrite = &_Self_type::write;
        _xread = &_Self_type::read;
      } else if ( type == sock_base::sock_dgram ) {
        _xwrite = &_Self_type::send;
        _xread = &_Self_type::recv;
      }
    } else if ( prot == sock_base::local ) {
      _fd = socket( PF_UNIX, type, 0 );
      if ( _fd == -1 ) {
        throw std::runtime_error( "can't open socket" );
      }
    } else { // other protocols not implemented yet
      throw std::invalid_argument( "protocol not implemented" );
    }
    if ( _bbuf == 0 )
      _M_allocate_block( type == sock_base::sock_stream ? 0xb00 : 0xffff ); // max 1460 (dec) [0x5b4] --- single segment
    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

#ifdef __FIT_NONBLOCK_SOCKETS
    if ( fcntl( _fd, F_SETFL, fcntl( _fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      throw std::runtime_error( "can't establish nonblock mode" );
    }
#endif
#ifdef __FIT_POLL
    pfd.fd = _fd;
    pfd.events = POLLIN | POLLHUP | POLLRDNORM;
#endif
    setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );
    setg( this->epptr(), this->epptr(), this->epptr() );

    _errno = 0; // if any
  }
  catch ( std::domain_error& ) {
#ifdef WIN32
    _errno = WSAGetLastError();
    ::closesocket( _fd );
#else
    _errno = errno;
    ::close( _fd );
#endif
    _fd = -1;
    return 0;
  }
  catch ( std::length_error& ) {
#ifdef WIN32
    ::closesocket( _fd );
#else
    ::close( _fd );
#endif
    _fd = -1;
    return 0;
  }
  catch ( std::runtime_error& ) {
#ifdef WIN32
    _errno = WSAGetLastError();
#else
    _errno = errno;
#endif
    return 0;
  }
  catch ( std::invalid_argument& ) {
    return 0;
  }

  return this;
}
// --

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( sock_base::socket_type s,
                                            sock_base::stype t,
                                            const timespec *timeout )
{
  if ( is_open() || s == -1 ) {
    return 0;
  }

  sockaddr sa;
  socklen_t sl = sizeof(sa);
  getsockname( s, &sa, &sl );

  return basic_sockbuf<charT, traits, _Alloc>::open( s, sa, t, timeout );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( sock_base::socket_type s,
                                            const sockaddr& addr,
                                            sock_base::stype t,
                                            const timespec *timeout )
{
  if ( is_open() || s == -1 ) {
    return 0;
  }
  _fd = s;
  memcpy( (void *)&_address.any, (const void *)&addr, sizeof(sockaddr) );
  _mode = ios_base::in | ios_base::out;
  _errno = 0;
  _type = t;
#ifdef WIN32
  WSASetLastError( 0 );
#endif
#ifdef __FIT_POLL
  if ( timeout != 0 ) {
    _timeout = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000;
  } else {
    _timeout = -1;
  }
#endif
#ifdef __FIT_SELECT
  if ( timeout != 0 ) {
    _timeout.tv_sec = timeout->tv_sec;
    _timeout.tv_usec = timeout->tv_nsec / 1000;
    _timeout_ref = &_timeout;
  } else {
    _timeout_ref = 0;
  }
#endif
  if ( t == sock_base::sock_stream ) {
    _xwrite = &_Self_type::write;
    _xread = &_Self_type::read;
  } else if ( t == sock_base::sock_dgram ) {
    _xwrite = &_Self_type::sendto;
    _xread = &_Self_type::recvfrom;
  } else {
    _fd = -1;
    return 0; // unsupported type
  }

  if ( _bbuf == 0 ) {
    _M_allocate_block( t == sock_base::sock_stream ? 0xb00 : 0xffff );
  }

  if ( _bbuf == 0 ) {
#ifdef WIN32
    ::closesocket( _fd );
#else
    ::close( _fd );
#endif
    _fd = -1;
    return 0;
  }

#ifdef __FIT_NONBLOCK_SOCKETS
  if ( fcntl( _fd, F_SETFL, fcntl( _fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
    throw std::runtime_error( "can't establish nonblock mode" );
  }
#endif
#ifdef __FIT_POLL
  pfd.fd = _fd;
  pfd.events = POLLIN | POLLHUP | POLLRDNORM;
#endif
  setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );
  setg( this->epptr(), this->epptr(), this->epptr() );

  _errno = 0; // if any

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::attach( sock_base::socket_type s,
                                              sock_base::stype t,
                                              const timespec *timeout )
{
  if ( is_open() || s == -1 ) {
    return 0;
  }

  sockaddr sa;
  socklen_t sl = sizeof(sa);
  getsockname( s, &sa, &sl );

  return basic_sockbuf<charT, traits, _Alloc>::attach( s, sa, t, timeout );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::attach( sock_base::socket_type s,
                                              const sockaddr& addr,
                                              sock_base::stype t,
                                              const timespec *timeout )
{
  if ( is_open() || s == -1 ) {
    return 0;
  }

  // _doclose = false;
  return basic_sockbuf<charT, traits, _Alloc>::open( dup(s), addr, t, timeout );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::close()
{
  if ( !is_open() )
    return 0;

  // if ( _doclose ) {
#ifdef WIN32
    ::closesocket( _fd );
#else
    ::close( _fd );
#endif
  // }

  // _STLP_ASSERT( _bbuf != 0 );
  // put area before get area
  setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );
  setg( this->epptr(), this->epptr(), this->epptr() );

  _fd = -1;

  return this;
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::shutdown( sock_base::shutdownflg dir )
{
  if ( is_open() ) {
    if ( (dir & (sock_base::stop_in | sock_base::stop_out)) ==
         (sock_base::stop_in | sock_base::stop_out) ) {
      ::shutdown( _fd, 2 );
    } else if ( dir & sock_base::stop_in ) {
      ::shutdown( _fd, 0 );
    } else if ( dir & sock_base::stop_out ) {
      ::shutdown( _fd, 1 );
    }
  }
}

template<class charT, class traits, class _Alloc>
__FIT_TYPENAME basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::underflow()
{
  if( !is_open() )
    return traits::eof();

  if ( this->gptr() < this->egptr() )
    return traits::to_int_type(*this->gptr());

#ifndef __FIT_NONBLOCK_SOCKETS
#  ifdef __FIT_SELECT
  FD_ZERO( &pfd );
  FD_SET( fd(), &pfd );

  while ( select( fd() + 1, &pfd, 0, 0, _timeout_ref ) <= 0 ) {
    if ( errno == EINTR ) {  // may be interrupted, check and ignore
      errno = 0;
      continue;
    }
    return traits::eof();
  }
#  endif // __FIT_SELECT
#  ifdef __FIT_POLL
  pfd.revents = 0;

  while ( poll( &pfd, 1, _timeout ) <= 0 ) { // wait infinite
    if ( errno == EINTR ) { // may be interrupted, check and ignore
      errno = 0;
      continue;
    }
    return traits::eof();
  }
  if ( (pfd.revents & POLLERR) != 0 ) {
    // _state |= ios_base::failbit;
    return traits::eof();
  }
#  endif // __FIT_POLL
#endif // !__FIT_NONBLOCK_SOCKETS
  long offset = (this->*_xread)( this->eback(), sizeof(char_type) * (_ebuf - this->eback()) );
#ifdef __FIT_NONBLOCK_SOCKETS
  if ( offset < 0 && errno == EAGAIN ) {
    errno = 0;
#  ifdef __FIT_SELECT
    FD_ZERO( &pfd );
    FD_SET( fd(), &pfd );

    while ( select( fd() + 1, &pfd, 0, 0, _timeout_ref ) <= 0 ) {
      if ( errno == EINTR ) {  // may be interrupted, check and ignore
        errno = 0;
        continue;
      }
      return traits::eof();
    }
#  endif // __FIT_SELECT
#  ifdef __FIT_POLL
    pfd.revents = 0;

    while ( poll( &pfd, 1, _timeout ) <= 0 ) { // wait infinite
      if ( errno == EINTR ) { // may be interrupted, check and ignore
        errno = 0;
        continue;
      }
      return traits::eof();
    }
    if ( (pfd.revents & POLLERR) != 0 ) {
      // _state |= ios_base::failbit;
      return traits::eof();
    }
#  endif // __FIT_POLL
    offset = (this->*_xread)( this->eback(), sizeof(char_type) * (_ebuf - this->eback()) );
  }
#endif // __FIT_NONBLOCK_SOCKETS
  // Without  __FIT_NONBLOCK_SOCKETS:
  //   don't allow message of zero length:
  //   in conjunction with POLLIN in revent of poll above this designate that
  //   we receive FIN packet.
  // With __FIT_NONBLOCK_SOCKETS:
  //   0 is eof, < 0 --- second read also return < 0, but shouldn't
  if ( offset <= 0 ) {
    return traits::eof();
  }

  offset /= sizeof(charT);
        
  setg( this->eback(), this->eback(), this->eback() + offset );
  
  return traits::to_int_type(*this->gptr());
}

template<class charT, class traits, class _Alloc>
__FIT_TYPENAME basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::overflow( int_type c )
{
  if ( !is_open() )        
    return traits::eof();

  if ( !traits::eq_int_type( c, traits::eof() ) && this->pptr() < this->epptr() ) {
    sputc( traits::to_char_type(c) );
    return c;
  }

  long count = this->pptr() - this->pbase();

  if ( count ) {
    count *= sizeof(charT);
#ifndef __FIT_NONBLOCK_SOCKETS
    if ( (this->*_xwrite)( this->pbase(), count ) != count ) {
      return traits::eof();
    }
#else
    long offset = (this->*_xwrite)( this->pbase(), count );
    if ( offset < 0 ) {
      if ( errno == EAGAIN ) {
        pollfd wpfd;
        wpfd.fd = _fd;
        wpfd.events = POLLOUT | POLLHUP | POLLWRNORM;
        wpfd.revents = 0;
        while ( poll( &wpfd, 1, _timeout ) <= 0 ) { // wait infinite
          if ( errno == EINTR ) { // may be interrupted, check and ignore
            errno = 0;
            continue;
          }
          return traits::eof();
        }
        if ( (wpfd.revents & POLLERR) != 0 ) {
          return traits::eof();
        }
        offset = (this->*_xwrite)( this->pbase(), count );
        if ( offset < 0 ) {
          return traits::eof();
        }
      } else {
        return traits::eof();
      }
    }
    if ( offset < count ) {
      // MUST BE: (offset % sizeof(char_traits)) == 0 !
      offset /= sizeof(char_traits);
      count /= sizeof(char_traits);
      traits::move( this->pbase(), this->pbase() + offset, count - offset );
      // std::copy_backword( this->pbase() + offset, this->pbase() + count, this->pbase() );
      setp( this->pbase(), this->epptr() ); // require: set pptr
      this->pbump( count - offset );
      if( !traits::eq_int_type(c,traits::eof()) ) {
        sputc( traits::to_char_type(c) );
      }

      return traits::not_eof(c);
    }
#endif
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
    // _STLP_ASSERT( this->pbase() != 0 );
    count *= sizeof(charT);
#ifndef __FIT_NONBLOCK_SOCKETS
    if ( (this->*_xwrite)( this->pbase(), count ) != count ) {
      return -1;
    }
#else
    long start = 0;
    while ( count > 0 ) {
      long offset = (this->*_xwrite)( this->pbase() + start, count );
      if ( offset < 0 ) {
        if ( errno == EAGAIN ) {
          pollfd wpfd;
          wpfd.fd = _fd;
          wpfd.events = POLLOUT | POLLHUP | POLLWRNORM;
          wpfd.revents = 0;
          while ( poll( &wpfd, 1, _timeout ) <= 0 ) { // wait infinite
            if ( errno == EINTR ) { // may be interrupted, check and ignore
              errno = 0;
              continue;
            }
            return -1;
          }
          if ( (wpfd.revents & POLLERR) != 0 ) {
            return -1;
          }
          offset = (this->*_xwrite)( this->pbase() + start, count );
          if ( offset < 0 ) {
            return -1;
          }
        } else {
          return -1;
        }
      }
      count -= offset;
      start += offset;
    }
#endif
    setp( this->pbase(), this->epptr() ); // require: set pptr
  }

  return 0;
}

template<class charT, class traits, class _Alloc>
streamsize basic_sockbuf<charT, traits, _Alloc>::
xsputn( const char_type *s, streamsize n )
{
  if ( !is_open() || s == 0 || n == 0 ) {
    return 0;
  }

  if ( this->epptr() - this->pptr() > n ) {
    traits::copy( this->pptr(), s, n );
    this->pbump( n );
  } else {
    streamsize __n_put = this->epptr() - this->pptr();
    traits::copy( this->pptr(), s, __n_put );
    this->pbump( __n_put );

    if ( traits::eq_int_type(overflow(),traits::eof()) )
      return 0;

    setp( (char_type *)(s + __n_put), (char_type *)(s + n) );
    this->pbump( n - __n_put );

    if ( traits::eq_int_type(overflow(),traits::eof()) ) {
      setp( _bbuf, _bbuf + ((_ebuf - _bbuf) >> 1) );
      return 0;
    }
    setp( _bbuf, _bbuf + ((_ebuf - _bbuf) >> 1) );
  }
  return n;
}

template<class charT, class traits, class _Alloc>
int basic_sockbuf<charT, traits, _Alloc>::recvfrom( void *buf, size_t n )
{
#if defined(_WIN32) || (defined(__hpux) && !defined(_INCLUDE_POSIX1C_SOURCE))
  int sz = sizeof( sockaddr_in );
#else
  socklen_t sz = sizeof( sockaddr_in );
#endif

  sockaddr_t addr;

#ifdef __FIT_POLL
  pollfd pfd;
  pfd.fd = _fd;
  pfd.events = POLLIN;
#endif // __FIT_POLL
#ifdef __FIT_SELECT
  int t = 10;
  fd_set pfd;
#endif // __FIT_SELECT
  do {
#ifdef __FIT_SELECT
    FD_ZERO( &pfd );
    FD_SET( _fd, &pfd );

    if ( select( _fd + 1, &pfd, 0, 0, _timeout_ref ) > 0 ) {
      // get address of caller only
      char buff[32];
      ::recvfrom( _fd, buff, 32, MSG_PEEK, &addr.any, &sz );
    } else {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#endif // __FIT_SELECT
#ifdef __FIT_POLL
    pfd.revents = 0;
    if ( poll( &pfd, 1, _timeout ) > 0 ) { // wait infinite
      // get address of caller only
      char buff[32];    
      ::recvfrom( _fd, buff, 32, MSG_PEEK, &addr.any, &sz );
    } else {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#endif // __FIT_POLL
    if ( memcmp( &_address.inet, &addr.inet, sizeof(sockaddr_in) ) == 0 ) {
#ifdef WIN32
      return ::recvfrom( _fd, (char *)buf, n, 0, &_address.any, &sz );
#else
      return ::recvfrom( _fd, buf, n, 0, &_address.any, &sz );
#endif
    }
    xmt::Thread::yield();
  } while ( true );

  return 0; // never
}

template<class charT, class traits, class _Alloc>
void basic_sockstream<charT, traits, _Alloc>::setoptions( sock_base::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  if ( _sb.is_open() ) {
    if ( optname != sock_base::so_linger ) {
      int turn = on_off ? 1 : 0;
      if ( setsockopt( _sb.fd(), SOL_SOCKET, (int)optname, (const void *)&turn,
                       (socklen_t)sizeof(int) ) != 0 ) {
        this->setstate( ios_base::failbit );
      }
    } else {
      linger l;
      l.l_onoff = on_off ? 1 : 0;
      l.l_linger = __v;
      if ( setsockopt( _sb.fd(), SOL_SOCKET, (int)optname, (const void *)&l,
                       (socklen_t)sizeof(linger) ) != 0 ) {
        this->setstate( ios_base::failbit );
      }
    }
  } else {
    this->setstate( ios_base::failbit );
  }
#endif // __unix
}

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif

