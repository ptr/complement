// -*- C++ -*- Time-stamp: <00/09/08 14:52:34 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
 * ParallelGraphics Ltd.
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
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#ifdef __unix
extern "C" int nanosleep(const struct timespec *, struct timespec *);
#endif

// #ifdef __linux
#if !defined(__sun) && !defined(_WIN32) // i.e. __linux and __hpux
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
  if ( _bbuf == 0 )
    _M_allocate_block( 0xb00 ); // max 1460 (dec) [0x5b4] --- single segment
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

  __STL_ASSERT( this->pbase() != 0 );
  __STL_ASSERT( this->pptr() != 0 );
  __STL_ASSERT( this->epptr() != 0 );
  __STL_ASSERT( this->eback() != 0 );
  __STL_ASSERT( this->gptr() != 0 );
  __STL_ASSERT( this->egptr() != 0 );
  __STL_ASSERT( _bbuf != 0 );
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

  __STL_ASSERT( this->pbase() != 0 );
  __STL_ASSERT( this->pptr() != 0 );
  __STL_ASSERT( this->epptr() != 0 );
  __STL_ASSERT( this->eback() != 0 );
  __STL_ASSERT( this->gptr() != 0 );
  __STL_ASSERT( this->egptr() != 0 );
  __STL_ASSERT( _bbuf != 0 );
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

  __STL_ASSERT( _bbuf != 0 );
	// put area before get area
  setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );
  setg( this->epptr(), this->epptr(), this->epptr() );

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
                     (const void *)&_val,
#ifdef socklen_t
                     (socklen_t)sizeof(int)
#else // HP-UX 10.01
                     (int)sizeof(int)
#endif
                   ) != 0 ) {
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
#if defined(__HP_aCC) && (__HP_aCC == 1)
typename
#endif
basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::underflow()
{
  if( !_open )
    return traits::eof();

  if ( this->gptr() < this->egptr() )
    return traits::to_int_type(*this->gptr());

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

  __STL_ASSERT( this->eback() != 0 );
  __STL_ASSERT( _ebuf != 0 );

  // cerr << "Read" << endl;
  long offset = (this->*_xread)( this->eback(), sizeof(char_type) * (_ebuf - this->eback()) );
  if ( offset <= 0 ) // don't allow message of zero length
    return traits::eof();
  offset /= sizeof(charT);
  // cerr << "Read " << offset << endl;
        
//	cerr << "Underflow: " << hex << unsigned(eback()) << " + " << dec
//	     << offset << " = " << hex << unsigned( eback() + offset ) << dec << endl;
  setg( this->eback(), this->eback(), this->eback() + offset );
  
  return traits::to_int_type(*this->gptr());
}

template<class charT, class traits, class _Alloc>
#if defined(__HP_aCC) && (__HP_aCC == 1)
typename
#endif
basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::overflow( int_type c )
{
  if ( !_open )        
    return traits::eof();

  if ( !traits::eq_int_type( c, traits::eof() ) && this->pptr() < this->epptr() ) {
    sputc( traits::to_char_type(c) );
    return c;
  }

  long count = this->pptr() - this->pbase();

  if ( count ) {
    __STL_ASSERT( this->pbase() != 0 );

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

  __STL_ASSERT( this->pbase() != 0 );
  __STL_ASSERT( this->pptr() != 0 );
  __STL_ASSERT( this->epptr() != 0 );
  __STL_ASSERT( _bbuf != 0 );
  __STL_ASSERT( _ebuf != 0 );

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
    __STL_ASSERT( _bbuf != 0 );
    __STL_ASSERT( _ebuf != 0 );
    __STL_ASSERT( this->gptr() != 0 );
    __STL_ASSERT( this->egptr() != 0 );

//    cerr << "ioctl: " << dec << nmsg << ", " << nlen << endl;
    bool shift_req = this->gptr() == this->eback() ? false : (_ebuf - this->gptr()) > nlen ? false : true;
    if ( shift_req ) {
      __STL_ASSERT( this->gptr() > this->eback() );
      __STL_ASSERT( this->gptr() <= this->egptr() );
      traits::move( this->eback(), this->gptr(), this->egptr() - this->gptr() );
      setg( this->eback(), this->eback(), this->eback() + (this->egptr() - this->gptr()) );
    }
    if ( this->gptr() == _ebuf ) { // I should read something, if other side write
      return -1;             // otherwise I can't write without pipe broken
    }
    long offset = (this->*_xread)( this->egptr(), sizeof(char_type) * (_ebuf - this->egptr()) );
//    cerr << "I read here " << offset << endl;
    if ( offset < 0 ) // allow message of zero length
      return -1;
    offset /= sizeof(charT);
    setg( this->eback(), this->gptr(), this->egptr() + offset );
  }

  return 0;
}

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

// For WIN32 and HP-UX 11.00 gethostbyaddr is reeentrant
// ptr: _PTHREADS_DRAFT4 has sence only for HP-UX 11.00
// ptr: PTHREAD_THREADS_MAX defined in HP-UX 10.01 and
#if defined(WIN32) || (defined(__hpux) && \
                        (!defined(_REENTRANT) || \
                        (!defined(_PTHREADS_DRAFT4) && \
                         !defined(PTHREAD_THREADS_MAX))))
#  define __GETHOSTBYADDR__
#endif

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::__hostname()
{
#ifdef __GETHOSTBYADDR__
  hostent *he;
#else
  hostent he;
#ifndef __hpux
  char tmp_buff[1024];
#else
  hostent_data tmp_buff;
#endif
#  ifdef __linux
  hostent *phe = 0;
#  endif
#endif
  int err = 0;
  in_addr in;
  in.s_addr = inet_addr();
#ifdef __GETHOSTBYADDR__
  // For Win he is thread private data, so that's safe
  // It's MT-safe also for HP-UX 11.00
  he = gethostbyaddr( (char *)&in.s_addr, sizeof(in_addr), AF_INET );
  if ( he != 0 ) {
    _hostname = he->h_name;
  } else {
    _hostname = "unknown";
  }
#else // __GETHOSTBYADDR__
  if (
#  ifdef __sun
       gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
                        &he, tmp_buff, 1024, &err ) != 0
#  elif defined(__linux)
       gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
                        &he, tmp_buff, 1024, &phe, &err ) == 0
#  elif defined(__hpux) // reentrant variant for HP-UX before 11.00
       gethostbyaddr_r( (char *)&in.s_addr, sizeof(in_addr), AF_INET,
                        &he, &tmp_buff ) == 0
#  else
#    error "Check port of gethostbyaddr_r"
#  endif
     )
  {
    _hostname = he.h_name;
  } else {
    _hostname = "unknown";
  }
#endif // __GETHOSTBYADDR__

  _hostname += " [";
  _hostname += inet_ntoa( in );
  _hostname += "]";
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::findhost( const char *hostname )
{
#ifndef __GETHOSTBYADDR__
  hostent _host;
#ifndef __hpux
  char tmpbuf[1024];
#else
  hostent_data tmpbuf;
#endif
#ifdef __linux
  hostent *host = 0;
  gethostbyname_r( hostname, &_host, tmpbuf, 1024, &host, &_errno );
#elif defined(__hpux)
  _errno = gethostbyname_r( hostname, &_host, &tmpbuf );
  hostent *host = &_host;
#elif defined(__sun)
  hostent *host = gethostbyname_r( hostname, &_host, tmpbuf, 1024, &_errno );
#else
#  error "Check port of gethostbyname_r"
#endif
  if ( host != 0 ) {
    memcpy( (char *)&_address.inet.sin_addr,
            (char *)host->h_addr, host->h_length );
  } else {
    _state |= sock_base::hnamefailbit;
  }
#else // __GETHOSTBYADDR__
  hostent *host = gethostbyname( hostname );
  if ( host != 0 ) {
    memcpy( (char *)&_address.inet.sin_addr,
            (char *)host->h_addr, host->h_length );
  } else {
#  ifdef WIN32
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
        if ( host != 0 ) { // Oh, that's was IP indeed...
          memcpy( (char *)&_address.inet.sin_addr,
                  (char *)host->h_addr, host->h_length );
          WSASetLastError( 0 ); // clear error
          _errno = 0;
          return;
        }
        _errno = WSAGetLastError();
      }
    }
#  endif // WIN32
    _state |= sock_base::hnamefailbit;
  }
#endif // __GETHOSTBYADDR__
}

#ifdef __GETHOSTBYADDR__
#undef __GETHOSTBYADDR__
#endif


#ifdef __SGI_STL_OWN_IOSTREAMS
__STL_END_NAMESPACE
#else
} // namespace std
#endif
