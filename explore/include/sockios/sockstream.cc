// -*- C++ -*- Time-stamp: <99/02/01 14:23:45 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

namespace std {

template<class charT, class traits>
basic_sockbuf<charT, traits> *
basic_sockbuf<charT, traits>::open( SOCKET s, const sockaddr& addr,
				    sock_base::stype t )
{
  if ( is_open() || s == -1 ) {
    return 0;
  }
  _fd = s;
  memcpy( (void *)&_address.any, (const void *)&addr, sizeof(sockaddr) );
  _mode = ios_base::in | ios_base::out;
  _state = sock_base::goodbit;
  _errno = 0;
#ifdef WIN32
  WSASetLastError( 0 );
#endif
  if ( t == sock_base::sock_stream ) {
    _xwrite = write;
    _xread = read;
  } else if ( t == sock_base::sock_dgram ) {
    _xwrite = sendto;
    _xread = recvfrom;
  } else {
    return 0; // unsupported type
  }


  if ( _base == 0 ) {
    _M_allocate_block( 0x2000 );
  }

  if ( _base == 0 ) {
    ::closesocket( _fd );
    _fd = -1;
    return 0;
  }

  setp( _base, _base + ((_ebuf - _base)>>1) );
  setg( epptr(), epptr(), epptr() );

  __stl_assert( pbase() != 0 );
  __stl_assert( pptr() != 0 );
  __stl_assert( epptr() != 0 );
  __stl_assert( eback() != 0 );
  __stl_assert( gptr() != 0 );
  __stl_assert( egptr() != 0 );
  __stl_assert( _base != 0 );
  __stl_assert( _ebuf != 0 );

  _state = sock_base::goodbit;
  _errno = 0; // if any
  _open = true;
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

template<class charT, class traits>
basic_sockbuf<charT, traits>::int_type
basic_sockbuf<charT, traits>::underflow()
{
  if( !_open )
    return traits::eof();

  if ( gptr() < egptr() )
    return traits::to_int_type(*gptr());

  // Here shift operator is 'divide by 2'
  // Oh, poll here --- find how many chars available!!!!

  pollfd pfd;
  pfd.fd = fd();
  pfd.events = POLLIN;
  pfd.revents = 0;
  // cerr << "Poll" << endl;
  if ( poll( &pfd, 1, -1 ) <= 0 ) { // timeout 200 (milliseconds)
    return traits::eof();
  }

  __stl_assert( eback() != 0 );
  __stl_assert( _ebuf != 0 );

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

template<class charT, class traits>
basic_sockbuf<charT, traits>::int_type
basic_sockbuf<charT, traits>::overflow( int_type c )
{
  if ( !_open )        
    return traits::eof();

  if ( !traits::eq_int_type( c, traits::eof() ) && pptr() < epptr() ) {
    sputc( traits::to_char_type(c) );
    return c;
  }

  long count = pptr() - pbase();

  if ( count ) {
    __stl_assert( pbase() != 0 );

    if ( __rdsync() != 0 ) { // I should read something, if other side write
      return traits::eof();  // otherwise I can't write without pipe broken
    }

    if ( (this->*_xwrite)( pbase(), sizeof(charT) * count ) != count * sizeof(charT) )
      return traits::eof();
  }

  setp( pbase(), epptr() ); // require: set pptr
  if( !traits::eq_int_type(c,traits::eof()) ) {
    sputc( traits::to_char_type(c) );
  }

  return traits::not_eof(c);
}

template<class charT, class traits>
streamsize basic_sockbuf<charT, traits>::
xsputn( const char_type *s, streamsize n )
{
  if ( !is_open() || s == 0 || n == 0 ) {
    return 0;
  }

  __stl_assert( pbase() != 0 );
  __stl_assert( pptr() != 0 );
  __stl_assert( epptr() != 0 );
  __stl_assert( _base != 0 );
  __stl_assert( _ebuf != 0 );

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

}
