// -*- C++ -*- Time-stamp: <04/01/21 16:55:44 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003
 * Petr Ovchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License Version 2.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#ifdef __unix
extern "C" int nanosleep(const struct timespec *, struct timespec *);
#endif

#if !defined(__sun) && !defined(_WIN32) // i.e. __linux and __hpux
#include <sys/poll.h> // pollfd
#endif

#if defined(__unix) && !defined(__UCLIBC__)
#include <stropts.h> // for ioctl() call
#endif


_STLP_BEGIN_NAMESPACE

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const char *name, int port,
				    sock_base::stype type,
				    sock_base::protocol prot )
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
      _M_allocate_block( 0xb00 ); // max 1460 (dec) [0x5b4] --- single segment
    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

    setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );
    setg( this->epptr(), this->epptr(), this->epptr() );

    // _STLP_ASSERT( this->pbase() != 0 );
    // _STLP_ASSERT( this->pptr() != 0 );
    // _STLP_ASSERT( this->epptr() != 0 );
    // _STLP_ASSERT( this->eback() != 0 );
    // _STLP_ASSERT( this->gptr() != 0 );
    // _STLP_ASSERT( this->egptr() != 0 );
    // _STLP_ASSERT( _bbuf != 0 );
    // _STLP_ASSERT( _ebuf != 0 );

    _errno = 0; // if any
    // __hostname();

//	in_port_t ppp = 0x5000;
//	cerr << hex << _address.inet.sin_port << " " << ppp << endl;
//	cerr << hex << _address.inet.sin_addr.s_addr << endl;
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

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( sock_base::socket_type s, const sockaddr& addr,
				    sock_base::stype t )
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
    _M_allocate_block( 0xb00 );
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

  setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );
  setg( this->epptr(), this->epptr(), this->epptr() );

  // _STLP_ASSERT( this->pbase() != 0 );
  // _STLP_ASSERT( this->pptr() != 0 );
  // _STLP_ASSERT( this->epptr() != 0 );
  // _STLP_ASSERT( this->eback() != 0 );
  // _STLP_ASSERT( this->gptr() != 0 );
  // _STLP_ASSERT( this->egptr() != 0 );
  // _STLP_ASSERT( _bbuf != 0 );
  // _STLP_ASSERT( _ebuf != 0 );

  _errno = 0; // if any
  // __hostname();
//	in_port_t ppp = 0x5000;
//	cerr << hex << _address.inet.sin_port << " " << ppp << endl;
//	cerr << hex << _address.inet.sin_addr.s_addr << endl;

//	sockaddr_in xxx;
//	int len = sizeof( xxx );

//	getpeername( fd(), (sockaddr *)&xxx, &len );
//	cerr << hex << xxx.sin_port << endl;
//	cerr << hex << xxx.sin_addr.s_addr << endl;

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::close()
{
  if ( !is_open() )
    return 0;

#ifdef WIN32
  ::closesocket( _fd );
#else
  ::close( _fd );
#endif

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

#ifdef __FIT_SELECT
  fd_set pfd;
  FD_ZERO( &pfd );
  FD_SET( fd(), &pfd );

  if ( select( fd() + 1, &pfd, 0, 0, 0 ) <= 0 ) {
    return traits::eof();
  }
#endif // __FIT_SELECT
#ifdef __FIT_POLL
  pollfd pfd;
  pfd.fd = fd();
  pfd.events = POLLIN;
  pfd.revents = 0;

  if ( !is_open() /* || (_state != ios_base::goodbit) */ ) {
    return traits::eof();
  }
  if ( poll( &pfd, 1, -1 ) <= 0 ) { // wait infinite
    return traits::eof();
  }
  if ( (pfd.revents & POLLERR) != 0 ) {
    // _state |= ios_base::failbit;
    return traits::eof();
  }
#endif // __FIT_POLL

  // _STLP_ASSERT( this->eback() != 0 );
  // _STLP_ASSERT( _ebuf != 0 );

  long offset = (this->*_xread)( this->eback(), sizeof(char_type) * (_ebuf - this->eback()) );
  // don't allow message of zero length:
  // in conjunction with POLLIN in revent of poll above this designate that
  // we receive FIN packet.
  if ( offset <= 0 )
    return traits::eof();
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
    // _STLP_ASSERT( this->pbase() != 0 );

    // Never do this: read and and write in basic_sockbuf are independent,
    // so reading here lead to lost message if reading and writing occur
    // simultaneously from different threads

//    if ( __rdsync() != 0 ) { // I should read something, if other side write
//      return traits::eof();  // otherwise I can't write without pipe broken
//    }

    if ( (this->*_xwrite)( this->pbase(), sizeof(charT) * count ) != count * sizeof(charT) )
      return traits::eof();
  }

  setp( this->pbase(), this->epptr() ); // require: set pptr
  if( !traits::eq_int_type(c,traits::eof()) ) {
    sputc( traits::to_char_type(c) );
  }

  return traits::not_eof(c);
}

template<class charT, class traits, class _Alloc>
streamsize basic_sockbuf<charT, traits, _Alloc>::
xsputn( const char_type *s, streamsize n )
{
  if ( !is_open() || s == 0 || n == 0 ) {
    return 0;
  }

  // _STLP_ASSERT( this->pbase() != 0 );
  // _STLP_ASSERT( this->pptr() != 0 );
  // _STLP_ASSERT( this->epptr() != 0 );
  // _STLP_ASSERT( _bbuf != 0 );
  // _STLP_ASSERT( _ebuf != 0 );

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

#if 0 // No needs: the sockets are essential bi-direction entities
template<class charT, class traits, class _Alloc>
int basic_sockbuf<charT, traits, _Alloc>::__rdsync()
{
#ifdef WIN32
  unsigned long nlen = 0;
  int nmsg = ioctlsocket( fd(), FIONREAD, &nlen );
#else
  long nlen = 0;
  int nmsg = ioctl( fd(), I_NREAD, &nlen ); // shouldn't work, as I understand...
#endif
  if ( nmsg > 0 && nlen > 0 ) {
    // _STLP_ASSERT( _bbuf != 0 );
    // _STLP_ASSERT( _ebuf != 0 );
    // _STLP_ASSERT( this->gptr() != 0 );
    // _STLP_ASSERT( this->egptr() != 0 );

    bool shift_req = this->gptr() == this->eback() ? false : (_ebuf - this->gptr()) > nlen ? false : true;
    if ( shift_req ) {
      // _STLP_ASSERT( this->gptr() > this->eback() );
      // _STLP_ASSERT( this->gptr() <= this->egptr() );
      traits::move( this->eback(), this->gptr(), this->egptr() - this->gptr() );
      setg( this->eback(), this->eback(), this->eback() + (this->egptr() - this->gptr()) );
    }
    if ( this->gptr() == _ebuf ) { // I should read something, if other side write
      return -1;             // otherwise I can't write without pipe broken
    }
    long offset = (this->*_xread)( this->egptr(), sizeof(char_type) * (_ebuf - this->egptr()) );
    if ( offset < 0 ) // allow message of zero length
      return -1;
    offset /= sizeof(charT);
    setg( this->eback(), this->gptr(), this->egptr() + offset );
  }

  return 0;
}
#endif // 0

#if defined(__HP_aCC) && (__HP_aCC == 1)
  union basic_sockbuf_sockaddr {
      sockaddr_in inet;
      sockaddr    any;
  };
#endif

template<class charT, class traits, class _Alloc>
int basic_sockbuf<charT, traits, _Alloc>::recvfrom( void *buf, size_t n )
{
#if defined(_WIN32) || (defined(__hpux) && !defined(_INCLUDE_POSIX1C_SOURCE))
  int sz = sizeof( sockaddr_in );
#else
  size_t sz = sizeof( sockaddr_in );
#endif

#if defined(__HP_aCC) && (__HP_aCC == 1)
  basic_sockbuf_sockaddr addr;
#else
  union {
      sockaddr_in inet;
      sockaddr    any;
  } addr;
#endif
#ifdef __FIT_POLL
  timespec t;

  t.tv_sec = 0;
  t.tv_nsec = 10000;

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

    if ( select( _fd + 1, &pfd, 0, 0, 0 ) > 0 ) {
      // get address of caller only
      char buff[32];
      ::recvfrom( _fd, buff, 32, MSG_PEEK, &addr.any, &sz );
    } else {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#endif // __FIT_SELECT
#ifdef __FIT_POLL
    pfd.revents = 0;
    if ( poll( &pfd, 1, -1 ) > 0 ) { // wait infinite
      // get address of caller only
      char buff[32];    
      ::recvfrom( _fd, buff, 32, MSG_PEEK, &addr.any, &sz );
    } else {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#endif // __FIT_POLL
    if ( port() == addr.inet.sin_port && inet_addr() == addr.inet.sin_addr.s_addr ) {
//    if ( memcmp( (void *)&_address.any, (const void *)&addr, sizeof(sockaddr) ) == 0 ) {
#ifdef WIN32
      return ::recvfrom( _fd, (char *)buf, n, 0, &_address.any, &sz );
#else
      return ::recvfrom( _fd, buf, n, 0, &_address.any, &sz );
#endif
    }
#ifdef __unix
//    cerr << "Sleeping in sockstream: "
//         << port() << "/" << addr.inet.sin_port << ", "
//         << inet_addr() << "/" << addr.inet.sin_addr.s_addr << endl;
    nanosleep( &t, 0 );
#endif
#ifdef WIN32
    Sleep( t );
#endif
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

_STLP_END_NAMESPACE

