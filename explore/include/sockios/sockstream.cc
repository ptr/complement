// -*- C++ -*- Time-stamp: <00/05/23 19:38:07 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
 * ParallelGraphics
 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ident "$SunId$"

#ifdef __unix
extern "C" int nanosleep(const struct timespec *, struct timespec *);
#endif

#ifdef __Linux
#include <sys/poll.h> // pollfd
#endif

#ifdef __SGI_STL_OWN_IOSTREAMS
__STL_BEGIN_NAMESPACE
#else
namespace std {
#endif

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const char *name, int port,
				    sock_base::stype type,
				    sock_base::protocol prot )
{
  if ( is_open() ) {
    return 0;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _errno = 0;
  _type = type;
#ifdef WIN32
  WSASetLastError( 0 );
#endif
  if ( prot == sock_base::inet ) {
    _fd = socket( PF_INET, type, 0 );
    if ( _fd == -1 ) {
      _state |= sock_base::sockfailbit;
#ifdef WIN32
      _errno = WSAGetLastError();
#else
      _errno = errno;
#endif
      return 0;
    }
    _address.inet.sin_family = AF_INET;
    _address.inet.sin_port = htons( port );
    findhost( name );
    if ( _state != ios_base::goodbit ) {
#ifdef WIN32
      ::closesocket( _fd );
#else
      ::close( _fd );
#endif
      return 0;
    }
	  
    // Generally, stream sockets may successfully connect() only once
    if ( connect( _fd, &_address.any, sizeof( _address ) ) == -1 ) {
      _state |= sock_base::connfailbit;
#ifdef WIN32
      _errno = WSAGetLastError();
      ::closesocket( _fd );
#else
      _errno = errno;
      ::close( _fd );
#endif
      return 0;
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
      _state |= sock_base::sockfailbit;
#ifdef WIN32
      _errno = WSAGetLastError();
#else
      _errno = errno;
#endif
      return 0;
    }
  } else { // other protocols not implemented yet
    return 0;
  }
  if ( _base == 0 )
    _M_allocate_block( 0xb00 ); // max 1460 (dec) [0x5b4] --- single segment
  if ( _base == 0 ) {
#ifdef WIN32
    ::closesocket( _fd );
#else
    ::close( _fd );
#endif
    _fd = -1;

    return 0;
  }

  setp( _base, _base + ((_ebuf - _base)>>1) );
  setg( epptr(), epptr(), epptr() );

  __STL_ASSERT( pbase() != 0 );
  __STL_ASSERT( pptr() != 0 );
  __STL_ASSERT( epptr() != 0 );
  __STL_ASSERT( eback() != 0 );
  __STL_ASSERT( gptr() != 0 );
  __STL_ASSERT( egptr() != 0 );
  __STL_ASSERT( _base != 0 );
  __STL_ASSERT( _ebuf != 0 );

  _state = ios_base::goodbit;
  _errno = 0; // if any
  _open = true;
  __hostname();

//	in_port_t ppp = 0x5000;
//	cerr << hex << _address.inet.sin_port << " " << ppp << endl;
//	cerr << hex << _address.inet.sin_addr.s_addr << endl;

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
  _state = ios_base::goodbit;
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
    return 0; // unsupported type
  }


  if ( _base == 0 ) {
    _M_allocate_block( 0xb00 );
  }

  if ( _base == 0 ) {
#ifdef WIN32
    ::closesocket( _fd );
#else
    ::close( _fd );
#endif
    _fd = -1;
    return 0;
  }

  setp( _base, _base + ((_ebuf - _base)>>1) );
  setg( epptr(), epptr(), epptr() );

  __STL_ASSERT( pbase() != 0 );
  __STL_ASSERT( pptr() != 0 );
  __STL_ASSERT( epptr() != 0 );
  __STL_ASSERT( eback() != 0 );
  __STL_ASSERT( gptr() != 0 );
  __STL_ASSERT( egptr() != 0 );
  __STL_ASSERT( _base != 0 );
  __STL_ASSERT( _ebuf != 0 );

  _state = ios_base::goodbit;
  _errno = 0; // if any
  _open = true;
  __hostname();
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
  if ( !_open )
    return 0;

//	cerr << "Closing" << endl;
	// overflow();
//	cerr << "Say shutdown on out" << endl;
  // shutdown( sock_base::stop_out );
//	cerr << "Sync" << endl;
  sync();
//	cerr << "Say shutdown on in" << endl;
  // shutdown( sock_base::stop_in );
  this->shutdown( sock_base::stop_in | sock_base::stop_out );
//	cerr << "Sync" << endl;
//	sync();
//	cerr << "Close" << endl;
#ifdef WIN32
  ::closesocket( _fd );
#else
  ::close( _fd );
#endif
//	cerr << "Pass" << endl;

  __STL_ASSERT( _base != 0 );
	// put area before get area
  setp( _base, _base + ((_ebuf - _base)>>1) );
  setg( epptr(), epptr(), epptr() );

  _fd = -1;
  _open = false;

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
void basic_sockbuf<charT, traits, _Alloc>::setoptions( int optname, bool __v )
{
#ifdef __unix
  if ( is_open() ) {
    int _val = __v ? 1 : 0;
    if ( setsockopt( _fd, SOL_SOCKET, optname,
                     (const void *)&_val, (socklen_t)sizeof(int) ) != 0 ) {
      _state |= sock_base::sockfailbit;
#ifdef WIN32
      _errno = WSAGetLastError();
#else
      _errno = errno;
#endif
    }
  } else {
    _state |= sock_base::sockfailbit;
  }
#endif
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::underflow()
{
  if( !_open )
    return traits::eof();

  if ( gptr() < egptr() )
    return traits::to_int_type(*gptr());

#ifdef WIN32
  fd_set pfd;
  FD_ZERO( &pfd );
  FD_SET( fd(), &pfd );

  // See comments for __unix section; I don't understand how in wins
  // buffers locked.

  // Seems with MS's iostream work fine without MT_UNLOCK/MT_LOCK
  // (and more: with MS's ios buffers and MT_UNLOCK/MT_LOCK this
  // shouldn't work) while with my ones it's required

  // !!!! If use native MS iostreams, comment three lines <---- i below !!!
#  ifndef __SGI_STL_OWN_IOSTREAMS 
  __locker()._M_release_lock();   // <---- 1
#  endif
  if ( select( fd() + 1, &pfd, 0, 0, 0 ) <= 0 ) {
#  ifndef __SGI_STL_OWN_IOSTREAMS 
    __locker()._M_acquire_lock(); // <---- 2
#  endif
    return traits::eof();
  }
#  ifndef __SGI_STL_OWN_IOSTREAMS 
  __locker()._M_acquire_lock();   // <---- 3
#  endif
#else // WIN32
  pollfd pfd;
  pfd.fd = fd();
  pfd.events = POLLIN;
  pfd.revents = 0;

  // Here trick with buffer locking: while I sleep on poll, buffer should
  // be unlocked to allow output operations; Before return, I should
  // recover status quo: indeed I am in critical section, that will
  // be unlocked outside this code (as it was locked outside it).
#  ifndef __SGI_STL_OWN_IOSTREAMS 
  __locker()._M_release_lock();
#  endif
  if ( poll( &pfd, 1, -1 ) <= 0 ) { // wait infinite
#  ifndef __SGI_STL_OWN_IOSTREAMS 
    __locker()._M_acquire_lock();
#  endif
    return traits::eof();
  }
#ifndef __SGI_STL_OWN_IOSTREAMS 
  __locker()._M_acquire_lock();
#endif
#endif

  __STL_ASSERT( eback() != 0 );
  __STL_ASSERT( _ebuf != 0 );

  // cerr << "Read" << endl;
  long offset = (this->*_xread)( eback(), sizeof(char_type) * (_ebuf - eback()) );
  if ( offset <= 0 ) // don't allow message of zero length
    return traits::eof();
  offset /= sizeof(charT);
  // cerr << "Read " << offset << endl;
        
//	cerr << "Underflow: " << hex << unsigned(eback()) << " + " << dec
//	     << offset << " = " << hex << unsigned( eback() + offset ) << dec << endl;
  setg( eback(), eback(), eback() + offset );
  
  return traits::to_int_type(*gptr());
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::overflow( int_type c )
{
  if ( !_open )        
    return traits::eof();

  if ( !traits::eq_int_type( c, traits::eof() ) && pptr() < epptr() ) {
    sputc( traits::to_char_type(c) );
    return c;
  }

  long count = pptr() - pbase();

  if ( count ) {
    __STL_ASSERT( pbase() != 0 );

    // Never do this: read and and write in basic_sockbuf are independent,
    // so reading here lead to lost message if reading and writing occur
    // simultaneously from different threads

//    if ( __rdsync() != 0 ) { // I should read something, if other side write
//      return traits::eof();  // otherwise I can't write without pipe broken
//    }

    if ( (this->*_xwrite)( pbase(), sizeof(charT) * count ) != count * sizeof(charT) )
      return traits::eof();
  }

  setp( pbase(), epptr() ); // require: set pptr
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

  __STL_ASSERT( pbase() != 0 );
  __STL_ASSERT( pptr() != 0 );
  __STL_ASSERT( epptr() != 0 );
  __STL_ASSERT( _base != 0 );
  __STL_ASSERT( _ebuf != 0 );

  if ( epptr() - pptr() > n ) {
    traits::copy( pptr(), s, n );
    pbump( n );
  } else {
    streamsize __n_put = epptr() - pptr();
    traits::copy( pptr(), s, __n_put );
    pbump( __n_put );

    if ( traits::eq_int_type(overflow(),traits::eof()) )
      return 0;

    setp( (char_type *)(s + __n_put), (char_type *)(s + n) );
    pbump( n - __n_put );

    if ( traits::eq_int_type(overflow(),traits::eof()) ) {
      setp( _base, _base + ((_ebuf - _base) >> 1) );
      return 0;
    }
    setp( _base, _base + ((_ebuf - _base) >> 1) );
  }
  return n;
}

template<class charT, class traits, class _Alloc>
int basic_sockbuf<charT, traits, _Alloc>::__rdsync()
{
#ifdef WIN32
  unsigned long nlen = 0;
  int nmsg = ioctlsocket( fd(), FIONREAD, &nlen );
#else
  long nlen = 0;
  int nmsg = ioctl( fd(), I_NREAD, &nlen );
#endif
  if ( nmsg > 0 && nlen > 0 ) {
    __STL_ASSERT( _base != 0 );
    __STL_ASSERT( _ebuf != 0 );
    __STL_ASSERT( gptr() != 0 );
    __STL_ASSERT( egptr() != 0 );

//    cerr << "ioctl: " << dec << nmsg << ", " << nlen << endl;
    bool shift_req = gptr() == eback() ? false : (_ebuf - gptr()) > nlen ? false : true;
    if ( shift_req ) {
      __STL_ASSERT( gptr() > eback() );
      __STL_ASSERT( gptr() <= egptr() );
      traits::move( eback(), gptr(), egptr() - gptr() );
      setg( eback(), eback(), eback() + (egptr() - gptr()) );
    }
    if ( gptr() == _ebuf ) { // I should read something, if other side write
      return -1;             // otherwise I can't write without pipe broken
    }
    long offset = (this->*_xread)( egptr(), sizeof(char_type) * (_ebuf - egptr()) );
//    cerr << "I read here " << offset << endl;
    if ( offset < 0 ) // allow message of zero length
      return -1;
    offset /= sizeof(charT);
    setg( eback(), gptr(), egptr() + offset );
  }

  return 0;
}

template<class charT, class traits, class _Alloc>
int basic_sockbuf<charT, traits, _Alloc>::recvfrom( void *buf, size_t n )
{
#ifdef _WIN32 // specific for Wins headers
  int sz = sizeof( sockaddr_in );
#else
  size_t sz = sizeof( sockaddr_in );
#endif

  union {
      sockaddr_in inet;
      sockaddr    any;
  } addr;
#ifdef __unix
  timespec t;

  t.tv_sec = 0;
  t.tv_nsec = 10000;

  pollfd pfd;
  pfd.fd = _fd;
  pfd.events = POLLIN;
#endif
#ifdef WIN32
  int t = 10;
  fd_set pfd;
#endif
  do {
#ifdef WIN32
    FD_ZERO( &pfd );
    FD_SET( _fd, &pfd );

    if ( select( _fd + 1, &pfd, 0, 0, 0 ) > 0 ) {
      // get address of caller only
      char buff[32];
      ::recvfrom( _fd, buff, 32, MSG_PEEK, &addr.any, &sz );
    } else {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#else
    pfd.revents = 0;
    if ( poll( &pfd, 1, -1 ) > 0 ) { // wait infinite
      // get address of caller only
      char buff[32];    
      ::recvfrom( _fd, buff, 32, MSG_PEEK, &addr.any, &sz );
    } else {
      return 0; // poll wait infinite, so it can't return 0 (timeout), so it return -1.
    }
#endif
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
void basic_sockbuf<charT, traits, _Alloc>::__hostname()
{
#ifdef WIN32
  hostent *he;
#endif
#ifdef __unix
  hostent he;
  char tmp_buff[1024];
#ifdef __Linux
  hostent* phe=0;
#endif
#endif
  int err = 0;
  in_addr in;
  in.s_addr = inet_addr();
#ifdef WIN32
  // For Win he is thread private data, so that's safe
  he = gethostbyaddr( (char *)&in.s_addr, sizeof(in_addr), AF_INET );
  if ( he != 0 ) {
    _hostname = he->h_name;
  } else {
    _hostname = "unknown";
  }
#else
  if (
#ifndef __Linux
       gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
                        &he, tmp_buff, 1024, &err ) != 0
#else
       gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
                        &he, tmp_buff, 1024, &phe, &err ) == 0
#endif
     )
  {
    _hostname = he.h_name;
  } else {
    _hostname = "unknown";
  }
#endif

  _hostname += " [";
  _hostname += inet_ntoa( in );
  _hostname += "]";
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::findhost( const char *hostname )
{
#ifndef WIN32
  hostent _host;
  char tmpbuf[1024];
#ifdef __Linux
  hostent *host = 0;
  gethostbyname_r( hostname, &_host, tmpbuf, 1024, &host,  &_errno );
#else
  hostent *host = gethostbyname_r( hostname, &_host, tmpbuf, 1024, &_errno );
#endif
  if ( host != 0 ) {
    memcpy( (char *)&_address.inet.sin_addr,
            (char *)host->h_addr, host->h_length );
  } else {
    _state |= sock_base::hnamefailbit;
  }
#else
  hostent *host = gethostbyname( hostname );
  if ( host != 0 ) {
    memcpy( (char *)&_address.inet.sin_addr,
            (char *)host->h_addr, host->h_length );
    return;
  }

  _errno = WSAGetLastError();

  // specific to Wins only:
  // cool M$ can't resolve IP address in gethostbyname, try once more
  // via inet_addr() and gethostbyaddr()
  // Returned _errno depend upon WinSock version, and applied patches,
  // with some of it even gethostbyname may be succeed.
  if ( _errno == WSAHOST_NOT_FOUND || _errno == WSATRY_AGAIN ) {
    unsigned long ipaddr = ::inet_addr( hostname );
    if ( ipaddr != INADDR_NONE ) {
      host = gethostbyaddr( (const char *)&ipaddr, sizeof(ipaddr), AF_INET );
      if ( host != 0 ) { // that's was IP indeed...
        memcpy( (char *)&_address.inet.sin_addr,
                (char *)host->h_addr, host->h_length );
        WSASetLastError( 0 ); // clear error
        _errno = 0;
        return;
      }
      _errno = WSAGetLastError();
    }
  }
  _state |= sock_base::hnamefailbit;
#endif
}

#ifdef __SGI_STL_OWN_IOSTREAMS
__STL_END_NAMESPACE
#else
} // namespace std
#endif
