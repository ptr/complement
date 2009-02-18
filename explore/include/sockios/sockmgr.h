// -*- C++ -*- Time-stamp: <09/02/18 15:20:49 ptr>

/*
 * Copyright (c) 2008, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __SOCKIOS_SOCKMGR_H
#define __SOCKIOS_SOCKMGR_H

#include <sys/epoll.h>

/*
#ifndef EPOLLRDHUP
#  define EPOLLRDHUP 0x2000
#endif
*/

#include <fcntl.h>

#include <cerrno>
#include <mt/thread>
#include <mt/mutex>
#include <mt/condition_variable>

#include <sockios/socksrv.h>

#ifdef STLPORT
#  include <unordered_map>
#  include <unordered_set>
// #  include <hash_map>
// #  include <hash_set>
// #  define __USE_STLPORT_HASH
#  define __USE_STLPORT_TR1
#else
#  if defined(__GNUC__) && (__GNUC__ < 4)
#    include <ext/hash_map>
#    include <ext/hash_set>
#    define __USE_STD_HASH
#  else
#    include <tr1/unordered_map>
#    include <tr1/unordered_set>
#    define __USE_STD_TR1
#  endif
#endif

#include <sockios/sockstream>
#include <deque>
#include <functional>

// #include <boost/shared_ptr.hpp>

namespace std {

template <class charT, class traits, class _Alloc> class basic_sockbuf;
template <class charT, class traits, class _Alloc> class basic_sockstream;
template <class charT, class traits, class _Alloc> class sock_processor_base;


template <class charT, class traits, class _Alloc> class basic_sockbuf;

namespace detail {

class stop_request :
        public std::exception
{
  public:
    virtual const char* what() throw()
      { return "sockmgr receive stop reqest"; }
};

template<class charT, class traits, class _Alloc>
class sockmgr
{
  private:
    typedef basic_sockstream<charT,traits,_Alloc> sockstream_t;
    typedef basic_sockbuf<charT,traits,_Alloc> sockbuf_t;
    typedef sock_processor_base<charT,traits,_Alloc> socks_processor_t;

    enum {
      listener,
      tcp_buffer,
      rqstop // ,
      // rqstart,
      // listener_on_exit,
      // tcp_buffer_on_exit
    };

    struct fd_info
    {
        enum {
          listener = 0x1,
          level_triggered = 0x2
        };

        fd_info() :
            flags(0U),
            b(0),
            p(0)
          { }

        fd_info( unsigned f, sockbuf_t* buf, socks_processor_t* proc ) :
            flags(f),
            b(buf),
            p(proc)
          { }

        fd_info( sockbuf_t* buf, socks_processor_t* proc ) :
            flags(0U),
            b(buf),
            p(proc)
          { }

        fd_info( sockbuf_t* buf ) :
            flags(0U),
            b(buf),
            p(0)
          { }

        fd_info( socks_processor_t* proc ) :
            flags(listener),
            b(0),
            p(proc)
          { }

        fd_info( const fd_info& info ) :
            flags( info.flags ),
            b( info.b ),
            p( info.p )
          { }

        unsigned flags;
        sockbuf_t* b;
        socks_processor_t* p;
    };

    struct ctl
    {
        int cmd;
        union {
            sock_base::socket_type fd;
            void *ptr;
        } data;
    };

    static void _loop( sockmgr *me )
      { me->io_worker(); }

  public:
    sockmgr( int hint = 1024, int ret = 1024 );

    ~sockmgr();

    void push( socks_processor_t& p );
    void push( sockbuf_t& s );
    // void pop( socks_processor_t& p, sock_base::socket_type _fd );
    void exit_notify( sockbuf_t* b, sock_base::socket_type fd );

  private:
    sockmgr( const sockmgr& )
      { }
    sockmgr& operator =( const sockmgr& )
      { return *this; }

    struct fdclose
    { };

    void dump_descr();

#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<sock_base::socket_type,fd_info> fd_container_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<sock_base::socket_type, fd_info> fd_container_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<sock_base::socket_type, fd_info> fd_container_type;
#endif

    void io_worker();
    void cmd_from_pipe();
    void process_listener( epoll_event&, typename fd_container_type::iterator );
    void process_regular( epoll_event&, typename fd_container_type::iterator );

    int efd;
    int pipefd[2];
    std::tr2::thread* _worker;
    const int n_ret;

    fd_container_type descr;
    std::tr2::mutex dll;

    friend class std::basic_socket<charT,traits,_Alloc>;
};

} //detail

} // namesapce std

#if defined(__USE_STLPORT_HASH) || defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
#  define __HASH_NAMESPACE std
#endif
#if defined(__USE_STD_HASH)
#  define __HASH_NAMESPACE __gnu_cxx
#endif

namespace __HASH_NAMESPACE {

#ifdef __USE_STD_TR1
namespace tr1 {
#endif

template <class charT, class traits, class _Alloc>
struct hash<std::basic_sockstream<charT, traits, _Alloc>* >
{
    size_t operator()(const std::basic_sockstream<charT, traits, _Alloc>* __x) const
      { return reinterpret_cast<size_t>(__x); }
};

#ifdef __USE_STD_TR1
}
#endif

#if defined(__GNUC__) && (__GNUC__ < 4) && !defined(HASH_VOID_PTR_DEFINED)
template<>
struct hash<void *>
{
   size_t operator()(const void *__x) const
     { return reinterpret_cast<size_t>(__x); }
};

#define HASH_VOID_PTR_DEFINED

#endif // __GNUC__ < 4

} // namespace __HASH_NAMESPACE

#undef __HASH_NAMESPACE

#ifdef __USE_STLPORT_HASH
#  undef __USE_STLPORT_HASH
#endif
#ifdef __USE_STD_HASH
#  undef __USE_STD_HASH
#endif
#ifdef __USE_STLPORT_TR1
#  undef __USE_STLPORT_TR1
#endif
#ifdef __USE_STD_TR1
#  undef __USE_STD_TR1
#endif

#include <sockios/sockmgr.cc>

#endif /* __SOCKIOS_SOCKMGR_H */
