// -*- C++ -*- Time-stamp: <99/05/27 21:00:55 ptr>

#ident "$SunId$ %Q%"

#include <cerrno>
#include <sockmgr.h>

namespace std {

__DLLEXPORT
void basic_sockmgr::open( int port, sock_base::stype type, sock_base::protocol prot )
{
  if ( is_open() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
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
      _errno = errno;
      if ( attempt++ == 10 || _errno != EADDRINUSE )
#endif
      {
	_state |= sock_base::bindfailbit;
#ifdef WIN32
	::closesocket( _fd );
#else
	::close( _fd );
#endif
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
      // so don't check return code from listen
      ::listen( _fd, SOMAXCONN );
    }
  } else if ( prot == sock_base::local ) {
    return;
  } else {
    return;
  }
  _state = ios_base::goodbit;
  _errno = 0; // if any
  _open = true;

  return;
}

__DLLEXPORT
void basic_sockmgr::close()
{
  if ( !is_open() ) {
    return;
  }
  shutdown( sock_base::stop_in | sock_base::stop_out );
#ifdef WIN32
  ::closesocket( _fd );
#else
  ::close( _fd );
#endif
  _fd = -1;
  _open = false;
}


} // namespace std
