// -*- C++ -*- Time-stamp: <02/06/20 10:24:48 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2001
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

#ifndef __SOCKMGR_H
#define __SOCKMGR_H

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

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

using __impl::Thread;
using __impl::Condition;

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
          _idx = Thread::xalloc();
        }
      }

    virtual ~basic_sockmgr()
      { basic_sockmgr::close(); }

  protected:
    __FIT_DECLSPEC void open( int port, sock_base::stype type, sock_base::protocol prot );

    virtual __FIT_DECLSPEC void close();

  public:
    bool is_open() const
      { return _fd != -1; }
    bool good() const
      { return _state == ios_base::goodbit; }

    sock_base::socket_type fd() const { return _fd;}

    // void shutdown( sock_base::shutdownflg dir );
    __FIT_DECLSPEC
    void setoptions( sock_base::so_t optname, bool on_off = true,
                     int __v = 0 );

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
};

struct sockmgr_client
{
    sockmgr_client()
      { }

    sockstream s;
};

// template <class Connect>
// struct sockmgr_client_MP :
//     public sockmgr_client
// {
//     sockmgr_client_MP()
//       { }
//
//     Connect    _proc;
// };

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

struct in_buf_avail :
    public unary_function<sockmgr_client *,bool> 
{
  bool operator()(const sockmgr_client *__x) const
      { return __x->s.rdbuf()->in_avail() > 0; }
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

    // To do: virtual
    void open( int port, sock_base::stype t = sock_base::sock_stream );
    // To do: 
    // void __open( int port, sock_base::stype t = sock_base::sock_stream );

    virtual void close()
      {
        basic_sockmgr::close();
        // loop_id.kill( SIGINT );
        // loop_id.kill( SIGPIPE );
      }

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
    // __impl::Mutex _params_lock;
    __impl::ThreadMgr  thr_mgr;

  protected:
    typedef sockmgr_stream<Connect> _Self_type;
    typedef std::vector<sockmgr_client *> _Sequence;
    // typedef less_sockmgr_client _Compare;
    typedef fd_equal _Compare;
#if !defined(__HP_aCC) || (__HP_aCC > 1)
    typedef typename _Sequence::value_type      value_type;
    typedef typename _Sequence::size_type       size_type;
    typedef          _Sequence                  container_type;

    typedef typename _Sequence::reference       reference;
    typedef typename _Sequence::const_reference const_reference;
#else
    typedef _Sequence::value_type      value_type;
    typedef _Sequence::size_type       size_type;
    typedef          _Sequence                  container_type;

    typedef _Sequence::reference       reference;
    typedef _Sequence::const_reference const_reference;
#endif

    _Sequence _M_c;
    _Compare  _M_comp;
    // __impl::Mutex _c_lock;
    // __STLPORT_STD::_STL_mutex_lock _c_lock;
    _STLP_mutex _c_lock;
};


class ConnectionProcessorTemplate_MP // As reference
{
  public:
    ConnectionProcessorTemplate_MP( std::sockstream& )
      { }

//    void open( std::sockstream& );
    void connect( std::sockstream& )
      { }
    void close()
      { }
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
#ifdef __FIT_POLL
        _pfd = 0;
#endif
      }

    explicit sockmgr_stream_MP( int port, sock_base::stype t = sock_base::sock_stream ) :
	basic_sockmgr(),
        _fdcount( 0 )
      {
#ifdef __FIT_POLL
        _pfd = 0;
#endif
        open( port, t );
      }

    ~sockmgr_stream_MP()
      {
#ifdef __FIT_POLL
        if ( _pfd != 0 ) {
          delete [] _pfd;
        }
#endif
      }

    void open( int port, sock_base::stype t = sock_base::sock_stream );

    virtual void close()
      {
        basic_sockmgr::close();
        // loop_id.kill( SIGINT );
        loop_id.kill( SIGPIPE );
      }

    void wait()
      {	loop_id.join(); }

  protected:
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
    // typedef sockmgr_client_MP<Connect> *(sockmgr_stream_MP<Connect>::*accept_type)();
    typedef _Connect *(sockmgr_stream_MP<Connect>::*accept_type)();

    accept_type _accept;
    // sockmgr_client_MP<Connect> *accept() // workaround for CC
    _Connect *accept() // workaround for CC
      { return (this->*_accept)(); }
    // sockmgr_client_MP<Connect> *accept_tcp();
    // sockmgr_client_MP<Connect> *accept_udp();
    _Connect *accept_tcp();
    _Connect *accept_udp();

  private:
    Thread     loop_id;

  protected:
    typedef sockmgr_stream_MP<Connect> _Self_type;
    // typedef std::vector<sockmgr_client_MP<Connect> *> _Sequence;
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

#ifdef __FIT_POLL
    pollfd *_pfd;
#endif
#ifdef __FIT_SELECT
    fd_set _pfd;
#endif
    unsigned _fdcount;

  private:
    // sockmgr_client_MP<Connect> *_shift_fd();
    _Connect *_shift_fd();
    static void _close_by_signal( int );
};

_STLP_END_NAMESPACE

#ifndef __STL_LINK_TIME_INSTANTIATION
#include <sockios/sockmgr.cc>
#endif

#endif // __SOCKMGR_H
