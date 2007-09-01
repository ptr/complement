// -*- C++ -*- Time-stamp: <07/08/31 09:55:36 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005-2007
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
  private:
    class Init
    {
      public:
        Init();
        ~Init();
      private:
        static void _guard( int );
        static void __at_fork_prepare();
        static void __at_fork_child();
        static void __at_fork_parent();
    };

  public:
    basic_sockmgr();
    virtual ~basic_sockmgr();

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
      { xmt::scoped_lock lk(_fd_lck); return is_open_unsafe(); }
    bool good() const
      { return _state == ios_base::goodbit; }

    sock_base::socket_type fd() const
      { xmt::scoped_lock lk(_fd_lck); return fd_unsafe(); }

    __FIT_DECLSPEC
    void shutdown( sock_base::shutdownflg dir );
    void setoptions( sock_base::so_t optname, bool on_off = true,
                     int __v = 0 )
      {
        xmt::scoped_lock lk(_fd_lck);
        setoptions_unsafe( optname, on_off, __v );
      }

  private:
    basic_sockmgr( const basic_sockmgr& );
    basic_sockmgr& operator =( const basic_sockmgr& );

  private:
    sock_base::socket_type _fd;    // master socket
    unsigned long _mode;  // open mode
    unsigned long _state; // state flags
    int           _errno; // error state
    _xsockaddr    _address;

  protected:
    static int _idx;
    friend class Init;

  protected:
    xmt::mutex _fd_lck;
    xmt::condition _loop_cnd;
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
	basic_sockmgr(),
        _thr_limit( 31 )
      {
        _busylimit.tv_sec = 0;
        _busylimit.tv_nsec = 90000000; // i.e 0.09 sec
        _alarm.tv_sec = 0;
        _alarm.tv_nsec = 50000000; // i.e 0.05 sec
        _idle.tv_sec = 10;
        _idle.tv_nsec = 0; // i.e 10 sec
      }

    explicit sockmgr_stream_MP( const in_addr& addr, int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _thr_limit( 31 )
      {
        open( addr, port, t );
        _busylimit.tv_sec = 0;
        _busylimit.tv_nsec = 90000000; // i.e 0.09 sec
        _alarm.tv_sec = 0;
        _alarm.tv_nsec = 50000000; // i.e 0.05 sec
        _idle.tv_sec = 10;
        _idle.tv_nsec = 0; // i.e 10 sec
      }

    explicit sockmgr_stream_MP( unsigned long addr, int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _thr_limit( 31 )
      {
        open( addr, port, t );
        _busylimit.tv_sec = 0;
        _busylimit.tv_nsec = 90000000; // i.e 0.09 sec
        _alarm.tv_sec = 0;
        _alarm.tv_nsec = 50000000; // i.e 0.05 sec
        _idle.tv_sec = 10;
        _idle.tv_nsec = 0; // i.e 10 sec
      }

    explicit sockmgr_stream_MP( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _thr_limit( 31 )
      {
        open( port, t );
        _busylimit.tv_sec = 0;
        _busylimit.tv_nsec = 50000000; // i.e 0.05 sec
        _alarm.tv_sec = 0;
        _alarm.tv_nsec = 50000000; // i.e 0.05 sec
        _idle.tv_sec = 10;
        _idle.tv_nsec = 0; // i.e 10 sec
      }

    ~sockmgr_stream_MP()
      { loop_id.join(); }

  private:
    sockmgr_stream_MP( const sockmgr_stream_MP<Connect>& );
    sockmgr_stream_MP<Connect>& operator =( const sockmgr_stream_MP<Connect>& );

  public:
    void open( const in_addr& addr, int port, sock_base::stype t = sock_base::sock_stream );
    void open( unsigned long addr, int port, sock_base::stype t = sock_base::sock_stream );
    void open( int port, sock_base::stype t = sock_base::sock_stream );

    virtual void close()
      { basic_sockmgr::close(); }

    void wait()
      {	loop_id.join(); }

    void detach( sockstream& ) // remove sockstream from polling in manager
      { }

    void setbusytime( const timespec& t )
      { _busylimit = t; }

    void setobservertime( const timespec& t )
      { _alarm = t; }

    void setidletime( const timespec& t )
      { _idle = t; }

    void setthreadlimit( unsigned lim )
      { _thr_limit = std::max( 3U, lim ) - 1; }

  protected:

    struct _Connect
    {
        _Connect() :
            _proc( 0 )
          { }

        _Connect( const _Connect& ) :
            _proc( 0 )
          { }

        ~_Connect()
          { if ( _proc ) { s.close(); _proc->close(); } delete _proc; }

        void open( sock_base::socket_type st, const sockaddr& addr, sock_base::stype t = sock_base::sock_stream )
          { s.open( st, addr, t ); _proc = new Connect( s ); }

        sockstream s;
        Connect *_proc;
    };

    typedef std::vector<pollfd> _fd_sequence;
    typedef std::list<_Connect> _Sequence;
    typedef std::deque<typename _Sequence::iterator> _connect_pool_sequence;

    void _open( sock_base::stype t = sock_base::sock_stream );
    static xmt::Thread::ret_code loop( void * );
    static xmt::Thread::ret_code connect_processor( void * );
    static xmt::Thread::ret_code observer( void * );

    struct fd_equal :
        public std::binary_function<_Connect,int,bool> 
    {
        bool operator()(const _Connect& __x, int __y) const
          { return __x.s.rdbuf()->fd() == __y; }
    };

    struct iaddr_equal :
        public std::binary_function<_Connect,sockaddr,bool> 
    {
        bool operator()(const _Connect& __x, const sockaddr& __y) const
          { return memcmp( &(__x.s.rdbuf()->inet_sockaddr()), reinterpret_cast<const sockaddr_in *>(&__y), sizeof(sockaddr_in) ) == 0; }
    };

    struct pfd_equal :
        public std::binary_function<typename _fd_sequence::value_type,int,bool>
    {
        bool operator()(const typename _fd_sequence::value_type& __x, int __y) const
          { return __x.fd == __y; }
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
    xmt::Thread loop_id;

  protected:
    typedef sockmgr_stream_MP<Connect> _Self_type;
    typedef fd_equal _Compare;
    typedef iaddr_equal _Compare_inet;
    typedef typename _Sequence::value_type      value_type;
    typedef typename _Sequence::size_type       size_type;
    typedef          _Sequence                  container_type;

    typedef typename _Sequence::reference       reference;
    typedef typename _Sequence::const_reference const_reference;

    _Sequence _M_c;
    _Compare  _M_comp;
    _Compare_inet  _M_comp_inet;
    pfd_equal _pfdcomp;
    xmt::mutex _c_lock;

    _fd_sequence _pfd;
    int _cfd; // sock_base::socket_type
    _connect_pool_sequence _conn_pool;
    xmt::condition _pool_cnd;
    xmt::mutex _dlock;
    timespec _tpop;

    xmt::mutex _flock;
    bool _follow;

    xmt::condition _observer_cnd;
    timespec _busylimit; // start new thread to process incoming
                         // requests, if processing thread busy
                         // more then _busylimit
    timespec _alarm;     // check and make decision about start
                         // new thread with _alarm interval
    timespec _idle;      // do nothing _idle time before thread
                         // terminate

    unsigned _thr_limit;

  private:
    bool _shift_fd();
    static void _close_by_signal( int );
    bool _is_follow() const
      { xmt::scoped_lock lk( _flock ); bool tmp = _follow; return tmp; }
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
    xmt::mutex _c_lock;

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
