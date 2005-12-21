// -*- C++ -*- Time-stamp: <05/12/21 10:25:41 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License Version 2.1
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

#ifndef __SOCKMGR_H
#define __SOCKMGR_H

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifndef __SOCKSTREAM__
#include <sockios/sockstream>
#endif

#include <vector>
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

_STLP_BEGIN_NAMESPACE

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
        __impl::Locker _l( _idx_lck );
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
    static __impl::Mutex _idx_lck;

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
	basic_sockmgr(),
        _fdcount( 0 )
      {
        _pfd = 0;
      }

    explicit sockmgr_stream_MP( const in_addr& addr, int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _fdcount( 0 )
      {
        _pfd = 0;
        open( addr, port, t );
      }

    explicit sockmgr_stream_MP( unsigned long addr, int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _fdcount( 0 )
      {
        _pfd = 0;
        open( addr, port, t );
      }

    explicit sockmgr_stream_MP( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _fdcount( 0 )
      {
        _pfd = 0;
        open( port, t );
      }

    ~sockmgr_stream_MP()
      {
        if ( _pfd != 0 ) {
          delete [] _pfd;
        }
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
    static int loop( void * );

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

    typedef _Connect *(sockmgr_stream_MP<Connect>::*accept_type)();

    accept_type _accept;
    _Connect *accept() // workaround for CC
      { return (this->*_accept)(); }
    _Connect *accept_tcp();
    _Connect *accept_udp();

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
    in_buf_avail _M_av;
    _STLP_mutex _c_lock;

    pollfd *_pfd;
    unsigned _fdcount;

  private:
    _Connect *_shift_fd();
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
    static int loop( void * );

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
    _STLP_mutex _c_lock;

    fd_set _pfdr;
    fd_set _pfde;
    int _fdmax;

  private:
    _Connect *_shift_fd();
    static void _close_by_signal( int );
};
#endif // !__FIT_NO_SELECT

_STLP_END_NAMESPACE

#ifndef __STL_LINK_TIME_INSTANTIATION
#include <sockios/sockmgr.cc>
#endif

#endif // __SOCKMGR_H
