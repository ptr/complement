// -*- C++ -*- Time-stamp: <99/05/26 20:37:10 ptr>

#ifndef __SOCKMGR_H
#define __SOCKMGR_H

#ident "$SunId$ %Q%"

#ifndef __SOCKSTREAM__
#include <sockstream>
#endif

#include <vector>
#include <cerrno>

#ifndef __XMT_H
#include <xmt.h>
#endif

#ifndef __THR_MGR_H
#include <thr_mgr.h>
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
    sockmgr_client()
      { }

    sockstream s;
};

template <class Connect>
struct sockmgr_client_MP :
    public sockmgr_client
{
    sockmgr_client_MP()
      { }

    Connect    _proc;
};

struct less_sockmgr_client :
    public binary_function<sockmgr_client *,sockmgr_client *,bool> 
{
  bool operator()(const sockmgr_client *__x, const sockmgr_client *__y) const { return !__y->s.is_open() && __x->s.is_open(); }
};

struct fd_equal :
    public binary_function<sockmgr_client *,int,bool> 
{
  bool operator()(const sockmgr_client *__x, int __y) const
      { return __x->s.rdbuf()->fd() == __y; }
};

// Policy: thread per client connection
template <class Connect>
class sockmgr_stream :
    public basic_sockmgr
{
  public:
    sockmgr_stream() :
	basic_sockmgr()
      { }

    explicit sockmgr_stream( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr()
      {	open( port, t ); }

    ~sockmgr_stream()
      {
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

    typedef sockmgr_client *(sockmgr_stream<Connect>::*accept_type)();

    accept_type _accept;
    sockmgr_client *accept() // workaround for CC
      { return (this->*_accept)(); }
    sockmgr_client *accept_tcp();
    sockmgr_client *accept_udp();

  private:
    Thread     loop_id;
    __impl::Mutex _params_lock;
    __impl::ThreadMgr  thr_mgr;

  protected:
    typedef std::vector<sockmgr_client *,__STL_DEFAULT_ALLOCATOR(sockmgr_client *)> _Sequence;
    // typedef less_sockmgr_client _Compare;
    typedef fd_equal _Compare;
    typedef typename _Sequence::value_type      value_type;
    typedef typename _Sequence::size_type       size_type;
    typedef          _Sequence                  container_type;

    typedef typename _Sequence::reference       reference;
    typedef typename _Sequence::const_reference const_reference;

    _Sequence _M_c;
    _Compare  _M_comp;
    __impl::Mutex _c_lock;
};

// Policy: multiplex all clients connections in one thread
template <class Connect>
class sockmgr_stream_MP : // multiplexor
    public basic_sockmgr
{
  public:
    sockmgr_stream_MP() :
	basic_sockmgr(),
        _fdcount( 0 )
      {
#ifdef __unix
        _pfd = 0;
#endif
      }

    explicit sockmgr_stream_MP( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _fdcount( 0 )
      {
#ifdef __unix
        _pfd = 0;
#endif
        open( port, t );
      }

    ~sockmgr_stream_MP()
      {
#ifdef __unix
        if ( _pfd != 0 ) {
          delete [] _pfd;
        }
#endif
      }

    void open( int port, sock_base::stype t = sock_base::sock_stream );
    void wait()
      {	loop_id.join(); }

  protected:
    static int loop( void * );
    typedef sockmgr_client_MP<Connect> *(sockmgr_stream_MP<Connect>::*accept_type)();

    accept_type _accept;
    sockmgr_client_MP<Connect> *accept() // workaround for CC
      { return (this->*_accept)(); }
    sockmgr_client_MP<Connect> *accept_tcp();
    sockmgr_client_MP<Connect> *accept_udp();

  private:
    Thread     loop_id;

  protected:
    typedef std::vector<sockmgr_client_MP<Connect> *,__STL_DEFAULT_ALLOCATOR(sockmgr_client_MP<Connect> *)> _Sequence;
    typedef fd_equal _Compare;
    typedef typename _Sequence::value_type      value_type;
    typedef typename _Sequence::size_type       size_type;
    typedef          _Sequence                  container_type;

    typedef typename _Sequence::reference       reference;
    typedef typename _Sequence::const_reference const_reference;

    _Sequence _M_c;
    _Compare  _M_comp;
    __impl::Mutex _c_lock;

#ifdef __unix
    pollfd *_pfd;
#endif
#ifdef WIN32
    fd_set _pfd;
#endif
    unsigned _fdcount;
};

} // namespace std

#ifndef __STL_SEPARATE_INSTANTIATION
#include <sockmgr.cc>
#endif

#endif // __SOCKMGR_H
