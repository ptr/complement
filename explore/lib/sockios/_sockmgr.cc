// -*- C++ -*- Time-stamp: <99/01/26 16:11:17 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <sockmgr.h>

#include <cerrno>
#include <xxx/plock.h>

namespace std {

void basic_sockmgr::open( int port, sock_base::stype type, sock_base::protocol prot )
{
  if ( is_open() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = sock_base::goodbit;
  _errno = 0;
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
      return;
    }
    _address.inet.sin_family = AF_INET;
    _address.inet.sin_port = htons( port );
    _address.inet.sin_addr.s_addr = htons( INADDR_ANY );
    int attempt = 0;
    while ( ::bind( _fd, &_address.any, sizeof(_address) ) == -1 ) {
#ifdef WIN32
      _errno = WSAGetLastError();
      if ( attempt++ == 10 || _errno != WSAEADDRINUSE )
#else
       _errno = std::errno;
      if ( attempt++ == 10 || _errno != EADDRINUSE )
#endif
      {
	_state |= sock_base::bindfailbit;
	::closesocket( _fd );
	return;
      }
#ifdef WIN32
      ::Sleep( 20000 );
#else
      ::sleep( 20 );
#endif
    }	    
    if ( attempt > 0 ) {
      _errno = 0;
#ifdef WIN32
      WSASetLastError( 0 );
#else
      errno = 0;
#endif
    }
    if ( type == sock_base::sock_stream ||
	 type == sock_base::sock_seqpacket ) {
      // I am shure, this is socket of type SOCK_STREAM | SOCK_SEQPACKET,
      // so don't check listen return code
      ::listen( _fd, SOMAXCONN );
    }
  } else if ( prot == sock_base::local ) {
    return;
  } else {
    return;
  }
  _state = sock_base::goodbit;
  _errno = 0; // if any
  _open = true;

  return;
}

void basic_sockmgr::close()
{
  if ( !is_open() ) {
    return;
  }
  shutdown( sock_base::stop_in | sock_base::stop_out );
  ::closesocket( _fd );
  _fd = -1;
  _open = false;
}


} // namespace std
