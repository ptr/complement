// -*- C++ -*- Time-stamp: <06/09/21 21:36:36 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __SOCKMGR_H
#define __SOCKMGR_H

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifndef __SOCKSTREAM__
#include <sockios/sockstream>
#endif

#include <vector>
#include <deque>
#include <cerrno>

#ifndef __XMT_H
#include <mt/xmt.h>
#endif

#ifndef __THR_MGR_H
#include <mt/thr_mgr.h>
#endif

#ifdef __unix
#include <poll.h>
#endif

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

union _xsockaddr {
    sockaddr_in inet;
    sockaddr    any;
};

class basic_sockmgr :
	public sock_base
{
  public:
    basic_sockmgr() :
	_errno( 0 ),
	_mode( ios_base::in | ios_base::out ),
	_state( ios_base::goodbit ),
	_fd( -1 )
      {
        xmt::Locker _l( _idx_lck );
        if ( _idx == -1 ) {
          _idx = xmt::Thread::xalloc();
        }
      }

    virtual ~basic_sockmgr()
      { basic_sockmgr::close(); }

  protected:
    __FIT_DECLSPEC void open( const in_addr& addr, int port, sock_base::stype type, sock_base::protocol prot );
    __FIT_DECLSPEC void open( unsigned long addr, int port, sock_base::stype type, sock_base::protocol prot );
    __FIT_DECLSPEC void open( int port, sock_base::stype type, sock_base::protocol prot );

    virtual __FIT_DECLSPEC void close();
    bool is_open_unsafe() const
      { return _fd != -1; }
    sock_base::socket_type fd_unsafe() const
      { return _fd; }
    __FIT_DECLSPEC
    void setoptions_unsafe( sock_base::so_t optname, bool on_off = true,
                     int __v = 0 );

  public:
    bool is_open() const
      { MT_REENTRANT( _fd_lck, _1 ); return is_open_unsafe(); }
    bool good() const
      { return _state == ios_base::goodbit; }

    sock_base::socket_type fd() const
      { MT_REENTRANT( _fd_lck, _1 ); return fd_unsafe(); }

    __FIT_DECLSPEC
    void shutdown( sock_base::shutdownflg dir );
    void setoptions( sock_base::so_t optname, bool on_off = true,
                     int __v = 0 )
      {
        MT_REENTRANT( _fd_lck, _1 );
        setoptions_unsafe( optname, on_off, __v );
      }

  private:
    sock_base::socket_type _fd;    // master socket
    unsigned long _mode;  // open mode
    unsigned long _state; // state flags
    int           _errno; // error state
    _xsockaddr    _address;

  protected:
    static int _idx;

  private:
    static xmt::Mutex _idx_lck;

  protected:
    xmt::Mutex _fd_lck;
    xmt::Condition _loop_cnd;
};

class ConnectionProcessorTemplate_MP // As reference
{
  public:
    ConnectionProcessorTemplate_MP( std::sockstream& )
      { }

    void connect( std::sockstream& )
      { }
    void close()
      { }
};

#ifndef __FIT_NO_POLL

template <class Connect>
class sockmgr_stream_MP :
    public basic_sockmgr
{
  public:
    sockmgr_stream_MP() :
	basic_sockmgr()
      { }

    explicit sockmgr_stream_MP( const in_addr& addr, int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr()
      { open( addr, port, t ); }

    explicit sockmgr_stream_MP( unsigned long addr, int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr()
      { open( addr, port, t ); }

    explicit sockmgr_stream_MP( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr()
      { open( port, t ); }

    ~sockmgr_stream_MP()
      { }

    void open( const in_addr& addr, int port, sock_base::stype t = sock_base::sock_stream );
    void open( unsigned long addr, int port, sock_base::stype t = sock_base::sock_stream );
    void open( int port, sock_base::stype t = sock_base::sock_stream );

    virtual void close()
      { basic_sockmgr::close(); }

    void wait()
      {	loop_id.join(); }

    void detach( sockstream& ) // remove sockstream from polling in manager
      { }

  protected:

    struct _Connect {
        sockstream *s;
        Connect *_proc;
    };

    typedef std::vector<pollfd> _fd_sequence;
    typedef std::deque<_Connect *> _connect_pool_sequence;

    void _open( sock_base::stype t = sock_base::sock_stream );
    static xmt::Thread::ret_code loop( void * );
    static xmt::Thread::ret_code connect_processor( void * );


    struct fd_equal :
        public std::binary_function<_Connect *,int,bool> 
    {
        bool operator()(const _Connect *__x, int __y) const
          { return __x->s->rdbuf()->fd() == __y; }
    };

    struct pfd_equal :
        public std::binary_function<typename _fd_sequence::value_type,int,bool>
    {
        bool operator()(const typename _fd_sequence::value_type& __x, int __y) const
          { return __x.fd == __y; }
    };
    
    struct _ProcState
    {
        _ProcState() :
            follow( false )
          { }

        bool is_follow() const
          { MT_REENTRANT( lock, _1 ); return follow; }

        xmt::Mutex lock;
        bool follow;
    };

    typedef bool (sockmgr_stream_MP<Connect>::*accept_type)();

#if 0
    accept_type _accept;
    _Connect *accept() // workaround for CC
      { return (this->*_accept)(); }
#else
    accept_type _accept;
#endif
    bool accept_tcp();
    bool accept_udp();

  private:
    xmt::Thread     loop_id;

  protected:
    typedef sockmgr_stream_MP<Connect> _Self_type;
    typedef std::vector<_Connect *> _Sequence;
    typedef fd_equal _Compare;
    typedef typename _Sequence::value_type      value_type;
    typedef typename _Sequence::size_type       size_type;
    typedef          _Sequence                  container_type;

    typedef typename _Sequence::reference       reference;
    typedef typename _Sequence::const_reference const_reference;

    _Sequence _M_c;
    _Compare  _M_comp;
    pfd_equal _pfdcomp;
    xmt::Mutex _c_lock;

    _fd_sequence _pfd;
    int _cfd; // sock_base::socket_type
    _connect_pool_sequence _conn_pool;
    xmt::Condition _pool_cnd;
    xmt::Mutex _dlock;

    _ProcState _state;

  private:
    bool _shift_fd();
    static void _close_by_signal( int );
};

#endif // !__FIT_NO_POLL

#ifndef __FIT_NO_SELECT

template <class Connect>
class sockmgr_stream_MP_SELECT :
    public basic_sockmgr
{
  public:
    sockmgr_stream_MP_SELECT() :
	basic_sockmgr(),
        _fdmax( 0 )
      {
      }

    explicit sockmgr_stream_MP_SELECT( const in_addr& addr, int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _fdmax( 0 )
      {
        open( addr, port, t );
      }

    explicit sockmgr_stream_MP_SELECT( unsigned long addr, int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _fdmax( 0 )
      {
        open( addr, port, t );
      }

    explicit sockmgr_stream_MP_SELECT( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _fdmax( 0 )
      {
        open( port, t );
      }

    ~sockmgr_stream_MP_SELECT()
      {
      }

    void open( const in_addr& addr, int port, sock_base::stype t = sock_base::sock_stream );
    void open( unsigned long addr, int port, sock_base::stype t = sock_base::sock_stream );
    void open( int port, sock_base::stype t = sock_base::sock_stream );

    virtual void close()
      { basic_sockmgr::close(); }

    void wait()
      {	loop_id.join(); }

    void detach( sockstream& ) // remove sockstream from polling in manager
      { }

  protected:
    void _open( sock_base::stype t = sock_base::sock_stream );
    static xmt::Thread::ret_code loop( void * );

    struct _Connect {
        sockstream *s;
        Connect *_proc;
    };

    struct fd_equal :
        public std::binary_function<_Connect *,int,bool> 
    {
        bool operator()(const _Connect *__x, int __y) const
          { return __x->s->rdbuf()->fd() == __y; }
    };

    struct in_buf_avail :
        public std::unary_function<_Connect *,bool> 
    {
        bool operator()(const _Connect *__x) const
          { return __x->s->rdbuf()->in_avail() > 0; }
    };

    typedef _Connect *(sockmgr_stream_MP_SELECT<Connect>::*accept_type)();

    accept_type _accept;
    _Connect *accept() // workaround for CC
      { return (this->*_accept)(); }
    _Connect *accept_tcp();
    _Connect *accept_udp();

  private:
    xmt::Thread     loop_id;

  protected:
    typedef sockmgr_stream_MP_SELECT<Connect> _Self_type;
    typedef std::vector<_Connect *> _Sequence;
    typedef fd_equal _Compare;
    typedef typename _Sequence::value_type      value_type;
    typedef typename _Sequence::size_type       size_type;
    typedef          _Sequence                  container_type;

    typedef typename _Sequence::reference       reference;
    typedef typename _Sequence::const_reference const_reference;

    _Sequence _M_c;
    _Compare  _M_comp;
    in_buf_avail _M_av;
    xmt::Mutex _c_lock;

    fd_set _pfdr;
    fd_set _pfde;
    int _fdmax;

  private:
    _Connect *_shift_fd();
    static void _close_by_signal( int );
};
#endif // !__FIT_NO_SELECT

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif

#ifndef __STL_LINK_TIME_INSTANTIATION
#include <sockios/sockmgr.cc>
#endif

#endif // __SOCKMGR_H
