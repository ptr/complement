// -*- C++ -*- Time-stamp: <99/02/08 14:16:17 ptr>

#ifndef __SOCKMGR_H
#define __SOCKMGR_H

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifndef __SOCKSTREAM__
#include <sockstream>
#endif

#include <map>
#include <cerrno>

#ifndef __XMT_H
#include <xmt.h>
#endif

using __impl::Thread;

namespace std {

class basic_sockmgr :
	public sock_base
{
  public:
    basic_sockmgr() :
	_open( false ),
	_errno( 0 ),
	_mode( ios_base::in | ios_base::out ),
	_state( ios_base::failbit ),
	_fd( -1 )
      { }

    ~basic_sockmgr()
      { close(); }

    void open( int port, sock_base::stype type, sock_base::protocol prot );

    void close();

    bool is_open() const
      { return _open; }
    bool good() const
      { return _state == ios_base::goodbit; }

    SOCKET fd() const { return _fd;}

    void shutdown( sock_base::shutdownflg dir )
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

  private:
    SOCKET        _fd;    // master socket
    unsigned long _mode;  // open mode
    unsigned long _state; // state flags
    int           _errno; // error state
    union {
	sockaddr_in inet;
	sockaddr    any;
    } _address;
    bool          _open;  // true if open
};

class _xsockaddr
{
  public:
    union {
	sockaddr_in inet;
	sockaddr    any;
    } _address;

    bool operator ==( const _xsockaddr& a ) const
      {
	if ( _address.any.sa_family == a._address.any.sa_family ) {
	  if ( _address.any.sa_family == AF_INET ) {
	    if ( _address.inet.sin_port ==  a._address.inet.sin_port ) {
	      if ( _address.inet.sin_addr.s_addr == a._address.inet.sin_addr.s_addr ) {
		return true;
	      }
	    }
	  } else {  // strcmp?
	    return memcmp( _address.any.sa_data, a._address.any.sa_data, 14 ) == 0;
	  }
	}
	return false;
      }

    bool operator <( const _xsockaddr& a ) const
      {
	if ( _address.any.sa_family < a._address.any.sa_family ) {
	  return true;
	}
	if ( _address.any.sa_family > a._address.any.sa_family ) {
	  return false;
	}
	if ( _address.any.sa_family == AF_INET ) {
	  if ( _address.inet.sin_port < a._address.inet.sin_port ) {
	    return true;
	  }
	  if ( _address.inet.sin_port > a._address.inet.sin_port ) {
	    return false;
	  }
	  if ( _address.inet.sin_addr.s_addr < a._address.inet.sin_addr.s_addr ) {
	    return true;
	  }
	} else {  // strcmp?
	  return (memcmp( _address.any.sa_data, a._address.any.sa_data, 14 ) < 0);
	}
	return false;
      }

    bool operator >( const _xsockaddr& a ) const
      {
	if ( _address.any.sa_family > a._address.any.sa_family ) {
	  return true;
	}
	if ( _address.any.sa_family < a._address.any.sa_family ) {
	  return false;
	}
	if ( _address.any.sa_family == AF_INET ) {
	  if ( _address.inet.sin_port > a._address.inet.sin_port ) {
	    return true;
	  }
	  if ( _address.inet.sin_port < a._address.inet.sin_port ) {
	    return false;
	  }
	  if ( _address.inet.sin_addr.s_addr > a._address.inet.sin_addr.s_addr ) {
	    return true;
	  }
	} else {  // strcmp?
	  return (memcmp( _address.any.sa_data, a._address.any.sa_data, 14 ) > 0);
	}
	return false;
      }
};

struct sockmgr_client
{
    sockstream s;

    Thread thrID;

    string    hostname;
    string    info;
};

// #ifdef _MSC_VER
// #pragma warning( disable : 4786 )
// #endif

template <class Connect>
class sockmgr_stream :
    public basic_sockmgr
{
  private:
    typedef map<_xsockaddr,sockmgr_client*,less<_xsockaddr>,
                __STL_DEFAULT_ALLOCATOR(sockmgr_client*) > container_type;

  public:
    sockmgr_stream() :
	basic_sockmgr()
      { }

    explicit sockmgr_stream( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr()
      {	open( port, t ); }

    ~sockmgr_stream()
      {
	container_type::iterator i = _storage.begin();
	for ( ; i != _storage.end(); ++i ) {
	  (*i).second->s.close();
	  delete (*i).second;
	}
      }

    void open( int port, sock_base::stype t = sock_base::sock_stream )
      {
	basic_sockmgr::open( port, t, sock_base::inet );
	if ( is_open() ) {
	  loop_id.launch( loop, this );
	}
      }

    void wait()
      {	loop_id.join(); }

    sockstream *accept_dgram();

    void erase( sockstream *s );

  protected:
    struct params
    {
	sockmgr_stream *me;
	sockmgr_client *client;
    };

    static int loop( void * );
    static int connection( void * );
    static void broken_pipe( int );

    sockmgr_client *accept();

  private:
    container_type _storage; // clients connections db

    Thread loop_id;

    __impl::Mutex _storage_lock;
    __impl::Mutex _params_lock;
};

} // namespace std

#ifndef __STL_SEPARATE_INSTANTIATION
#include <sockmgr.cc>
#endif

#endif // __SOCKMGR_H
