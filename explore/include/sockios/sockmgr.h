// -*- C++ -*- Time-stamp: <99/05/07 10:16:11 ptr>

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
using __impl::Condition;

namespace std {

union _xsockaddr {
    sockaddr_in inet;
    sockaddr    any;
};

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

    __DLLEXPORT void open( int port, sock_base::stype type, sock_base::protocol prot );

    __DLLEXPORT void close();

    bool is_open() const
      { return _open; }
    bool good() const
      { return _state == ios_base::goodbit; }

    sock_base::socket_type fd() const { return _fd;}

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
    sock_base::socket_type _fd;    // master socket
    unsigned long _mode;  // open mode
    unsigned long _state; // state flags
    int           _errno; // error state
    _xsockaddr    _address;
    bool          _open;  // true if open
};

struct sockmgr_client
{
    sockmgr_client() :
        thrID( unsigned(Thread::daemon | Thread::detached) )
      { }

    sockstream s;

    Thread thrID;

    string    hostname;
    string    info;
};

template <class Connect>
struct sockmgr_client_MP
{
    sockmgr_client_MP()
      { }

    Connect    _proc;
    sockstream s;
    string     hostname;
    string     info;
};

struct bad_thread :
    public unary_function<sockmgr_client *,bool>
{
    bool operator()(const sockmgr_client *__x) const
      { return !__x->thrID.good(); }
};

// struct bad_sock :
//    public unary_function<sockmgr_client *,bool>
// {
//    bool operator()(const sockmgr_client *__x) const
//      { return !__x->s.good(); }
// };

template <class Connect>
struct bad_connect :
    public unary_function<sockmgr_client_MP<Connect> *,bool>
{
    bool operator()(const sockmgr_client_MP<Connect> * __x) const
      { return !__x->s.good(); }
};

struct remove_client :
    public unary_function<sockmgr_client *,int>
{
    int operator()(sockmgr_client *__x) const
      { delete __x; return 0; }
};

template <class Connect>
struct remove_client_MP :
    public unary_function<sockmgr_client_MP<Connect> *,int>
{
    int operator()(sockmgr_client_MP<Connect> *__x) const
      { delete __x; return 0; }
};

// Policy: thread per client connection
template <class Connect>
class sockmgr_stream :
    public basic_sockmgr
{
  private:
    typedef unsigned key_type;
    typedef map<key_type,sockmgr_client*,less<key_type>,
                __STL_DEFAULT_ALLOCATOR(sockmgr_client*) > container_type;

    // typedef pair<const _xsockaddr,sockmgr_client*> value_type;
    // typedef list<value_type,__STL_DEFAULT_ALLOCATOR(value_type)> container_type;
    // 

  public:
    sockmgr_stream() :
	basic_sockmgr()
      { }

    explicit sockmgr_stream( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr()
      {	open( port, t ); }

    ~sockmgr_stream()
      {
        if ( _gc_id.good() ) {
          _garbage_end.wait();
        }
      }

    void open( int port, sock_base::stype t = sock_base::sock_stream );

    void wait()
      {	loop_id.join(); }

  protected:
    struct params
    {
	sockmgr_stream *me;
	sockmgr_client *client;
    };

    static int loop( void * );
    static int connection( void * );
    static int garbage_collector( void * );

    typedef sockmgr_client *(sockmgr_stream<Connect>::*accept_type)();

    accept_type _accept;
    sockmgr_client *accept() // workaround for CC
      { return (this->*_accept)(); }
    sockmgr_client *accept_tcp();
    sockmgr_client *accept_udp();

  private:
// #ifdef __unix
//    static __impl::Thread::thread_key_type _mt_key;
//    static int _mt_idx;
// #endif

    container_type _storage; // clients connections db
    key_type create_unique();

    Thread     loop_id;
    Thread    _gc_id;    // garbage collector thread
    Condition _garbage_end;

    __impl::Mutex _storage_lock;
    __impl::Mutex _params_lock;

    static const key_type _low;
    static const key_type _high;

    static key_type _id;
};

// Policy: multiplex all clients connections in one thread
template <class Connect>
class sockmgr_stream_MP : // multiplexor
    public basic_sockmgr
{
  private:
    typedef unsigned key_type;
    typedef map<key_type,sockmgr_client_MP<Connect>*,less<key_type>,
                __STL_DEFAULT_ALLOCATOR(sockmgr_client_MP<Connect>*) > container_type;

  public:
    sockmgr_stream_MP() :
	basic_sockmgr()
      { }

    explicit sockmgr_stream_MP( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr()
      {	open( port, t ); }

    ~sockmgr_stream_MP()
      {
        if ( _gc_id.good() ) {
          _garbage_end.wait();
        }
      }

    void open( int port, sock_base::stype t = sock_base::sock_stream );
    void wait()
      {	loop_id.join(); }

  protected:
    static int loop( void * );
    static int garbage_collector( void * );

    typedef sockmgr_client_MP<Connect> *(sockmgr_stream_MP<Connect>::*accept_type)();

    accept_type _accept;
    sockmgr_client_MP<Connect> *accept() // workaround for CC
      { return (this->*_accept)(); }
    sockmgr_client_MP<Connect> *accept_tcp();
    sockmgr_client_MP<Connect> *accept_udp();
#ifdef __unix
    pollfd *_pfd;
#endif
#ifdef WIN32
    fd_set _pfd;
#endif

  private:
    container_type _storage; // clients connections db
    key_type create_unique();

    Thread     loop_id;
    Thread    _gc_id;    // garbage collector thread
    Condition _garbage_end;

    __impl::Mutex _storage_lock;

    static const key_type _low;
    static const key_type _high;

    static key_type _id;
};

} // namespace std

#ifndef __STL_SEPARATE_INSTANTIATION
#include <sockmgr.cc>
#endif

#endif // __SOCKMGR_H
