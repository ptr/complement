// -*- C++ -*- Time-stamp: <08/04/11 22:14:39 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __SOCKIOS_SP_H
#define __SOCKIOS_SP_H

#include <sys/epoll.h>

#ifndef EPOLLRDHUP
#  define EPOLLRDHUP 0x2000
#endif

#include <fcntl.h>

#include <cerrno>
#include <mt/thread>
#include <mt/mutex>
#include <mt/condition_variable>

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

namespace std {

template <class charT, class traits, class _Alloc> class basic_sockbuf2;
template <class charT, class traits, class _Alloc> class basic_sockstream2;
template <class charT, class traits, class _Alloc> class sock_processor_base;

template <class charT, class traits, class _Alloc>
class sock_processor_base :
	public sock_base2,
        public basic_socket<charT,traits,_Alloc>
{
  private:
    typedef basic_socket<charT,traits,_Alloc> basic_socket_t;

  public:
    typedef basic_sockstream2<charT,traits,_Alloc> sockstream_t;

    struct adopt_new_t { };
    struct adopt_close_t { };
    struct adopt_data_t { };

    sock_processor_base() :
        _mode( ios_base::in | ios_base::out ),
        _state( ios_base::goodbit )
      { }

    explicit sock_processor_base( int port, sock_base2::stype t = sock_base2::sock_stream )
      { sock_processor_base::open( port, t, sock_base2::inet ); }

    virtual ~sock_processor_base()
      {
        sock_processor_base::close();

        std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)this << " " << std::tr2::getpid() << std::endl;
        // Never uncomment next line:
        // basic_socket<charT,traits,_Alloc>::mgr->final( *this );
        // this lead to virtual fuction call, that is already pure here.
      }

    void open( const in_addr& addr, int port, sock_base2::stype type, sock_base2::protocol prot );

    void open( unsigned long addr, int port, sock_base2::stype type, sock_base2::protocol prot )
      {
        in_addr _addr;
        _addr.s_addr = htonl( addr );
        sock_processor_base::open( _addr, port, type, prot );
      }

    void open( int port, sock_base2::stype type, sock_base2::protocol prot )
      { sock_processor_base::open(INADDR_ANY, port, type, prot); }

    virtual void close();

    virtual void operator ()( int fd, const adopt_new_t& ) = 0;
    virtual void operator ()( int fd, const adopt_close_t& ) = 0;
    virtual void operator ()( int fd, const adopt_data_t& ) = 0;

  private:
    sock_processor_base( const sock_processor_base& );
    sock_processor_base& operator =( const sock_processor_base& ); 

  protected:
    void setoptions_unsafe( sock_base2::so_t optname, bool on_off = true, int __v = 0 );
    sockstream_t* create_stream( int fd )
      {
        typename base_t::sockstream_t* s = new typename base_t::sockstream_t();
        s->rdbuf()->_open_sockmgr( fd, addr );
        return s;
      }

  public:
    bool is_open() const
      { std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck); return basic_socket_t::is_open_unsafe(); }
    bool good() const
      { return _state == ios_base::goodbit; }

    sock_base2::socket_type fd() const
      { std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck); sock_base2::socket_type tmp = basic_socket_t::fd_unsafe(); return tmp; }

    void shutdown( sock_base2::shutdownflg dir );
    void setoptions( sock_base2::so_t optname, bool on_off = true, int __v = 0 )
      {
        std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
        setoptions_unsafe( optname, on_off, __v );
      }

  private:
    unsigned long _mode;  // open mode
    unsigned long _state; // state flags

  protected:
    std::tr2::mutex _fd_lck;
};

typedef sock_processor_base<char,char_traits<char>,allocator<char> > sock_basic_processor;

template <class Connect, class charT = char, class traits = std::char_traits<charT>, class _Alloc = std::allocator<charT>, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& ) = &Connect::connect >
class connect_processor :
        public sock_processor_base<charT,traits,_Alloc>
{
  private:
    typedef sock_processor_base<charT,traits,_Alloc> base_t;

    class Init
    {
      public:
        Init()
          { _guard( 1 ); }
        ~Init()
          { _guard( 0 ); }

      private:
        static void _guard( int direction );
        static void __at_fork_prepare();
        static void __at_fork_child();
        static void __at_fork_parent();
        static std::tr2::mutex _init_lock;
        static int _count;
        static bool _at_fork;
    };

    static char Init_buf[];

  public:
    connect_processor() :
         not_empty( *this ),
         _in_work( false ),
         ploop( loop, this )
      { new( Init_buf ) Init(); }

    explicit connect_processor( int port ) :
        base_t( port, sock_base2::sock_stream ),
        not_empty( *this ),
        _in_work( false ),
        ploop( loop, this )
      { new( Init_buf ) Init(); }

    virtual ~connect_processor()
      {
        connect_processor::close();
        if ( ploop.joinable() ) {
          ploop.join();
        }
        // {
        // std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
        for ( typename worker_pool_t::iterator i = worker_pool.begin(); i != worker_pool.end(); ++i ) {
          delete i->second;
        }
        worker_pool.clear();
        // }
        for ( typename ready_pool_t::iterator j = ready_pool.begin(); j != ready_pool.end(); ++j ) {
          delete j->c;
        }
        ready_pool.clear();

        std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)this << " " << std::tr2::getpid() << std::endl;
        basic_socket<charT,traits,_Alloc>::mgr->final( *this );

        ((Init *)Init_buf)->~Init();
      }

    virtual void close();

    void wait()
      { if ( ploop.joinable() ) { ploop.join(); } }

  private:
    virtual void operator ()( int fd, const typename base_t::adopt_new_t& );
    virtual void operator ()( int fd, const typename base_t::adopt_close_t& );
    virtual void operator ()( int fd, const typename base_t::adopt_data_t& );

    static void loop( connect_processor* me )
      { me->worker(); }

    void worker();

  private:
    connect_processor( const connect_processor& )
      { }

    connect_processor& operator =( const connect_processor& )
      { return *this; }


    struct processor
    { 
        processor() :
            c(0),
            s(0)
          { }
        processor( Connect* __c, typename sock_processor_base<charT,traits,_Alloc>::sockstream_t* __s ) :
            c(__c),
            s(__s)
          { }
        processor( const processor& p ) :
            c( p.c ),
            s( p.s )
          { }

        processor& operator =( const processor& p )
          { c = p.c; s = p.s; return *this; }

       Connect* c;
       typename sock_processor_base<charT,traits,_Alloc>::sockstream_t* s;

       bool operator ==( const processor& p ) const
         { return s == p.s; }
       bool operator ==( const typename sock_processor_base<charT,traits,_Alloc>::sockstream_t* st ) const
         { return const_cast<const typename sock_processor_base<charT,traits,_Alloc>::sockstream_t*>(s) == st; }
       bool operator ==( int fd ) const
         {
           if ( s == 0 ) {
             return fd == -1;
           }
           return s->rdbuf()->fd() == fd;
         }

/*
       struct equal_to :
          public std::binary_function<processor, typename sock_processor_base<charT,traits,_Alloc>::sockstream_t*, bool>
       {
          bool operator()(const processor& __x, const typename sock_processor_base<charT,traits,_Alloc>::sockstream_t* __y) const
            { return __x == __y; }
       };
*/
    };

    bool pop_ready( processor& );

#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<int,processor> worker_pool_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<int,processor> worker_pool_t;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<int,processor> worker_pool_t;
#endif
    typedef std::deque<processor> ready_pool_t;

    struct _not_empty
    {
      _not_empty( connect_processor& p ) :
          me( p )
        { }

      bool operator()() const
        { return !me.ready_pool.empty(); }

      connect_processor& me;
    } not_empty;

    worker_pool_t worker_pool;
    ready_pool_t ready_pool;
    bool _in_work;
    std::tr2::mutex wklock;
    std::tr2::mutex rdlock;
    std::tr2::condition_variable cnd;
    std::tr2::mutex inwlock;
    std::tr2::condition_variable cnd_inwk;
    std::tr2::thread ploop;

    friend struct _not_empty;
};

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
    typedef basic_sockstream2<charT,traits,_Alloc> sockstream_t;
    typedef basic_sockbuf2<charT,traits,_Alloc> sockbuf_t;
    typedef sock_processor_base<charT,traits,_Alloc> socks_processor_t;

    enum {
      listener,
      // tcp_stream,
      tcp_buffer,
      rqstop,
      rqstart
    };

    struct fd_info
    {
        enum {
          listener = 0x1,
          level_triggered = 0x2,
          owner = 0x4,
          buffer = 0x8
        };

        unsigned flags;
        union {
          sockstream_t* s;
          sockbuf_t*    b;
        } s;
        socks_processor_t *p;
    };

    struct ctl
    {
        int cmd;
        union {
            int fd;
            void *ptr;
        } data;
    };

    static void _loop( sockmgr *me )
      { me->io_worker(); }

  public:
    sockmgr( int hint = 1024, int ret = 512 ) :
         n_ret( ret )
      {
        efd = epoll_create( hint );
        if ( efd < 0 ) {
          // throw system_error( errno )
          throw std::runtime_error( "epoll_create" );
        }
        if ( pipe( pipefd ) < 0 ) { // check err
          ::close( efd );
          // throw system_error;
          throw std::runtime_error( "pipe" );
        }
        // cfd = pipefd[1];

        epoll_event ev_add;
        ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
        ev_add.data.fd = pipefd[0];
        epoll_ctl( efd, EPOLL_CTL_ADD, pipefd[0], &ev_add );

        _worker = new std::tr2::thread( _loop, this );

        // ctl _ctl;
        // _ctl.cmd = rqstart;

        // write( pipefd[1], &_ctl, sizeof(ctl) );
      }

    ~sockmgr()
      {
        if ( _worker->joinable() ) {
          ctl _ctl;
          _ctl.cmd = rqstop;
          _ctl.data.ptr = 0;

          ::write( pipefd[1], &_ctl, sizeof(ctl) );

          _worker->join();
        }
        ::close( pipefd[1] );
        ::close( pipefd[0] );
        ::close( efd );
        delete _worker;
      }

    void push( socks_processor_t& p )
      {
        ctl _ctl;
        _ctl.cmd = listener;
        _ctl.data.ptr = static_cast<void *>(&p);

        int r = ::write( pipefd[1], &_ctl, sizeof(ctl) );
        if ( r < 0 || r != sizeof(ctl) ) {
          throw std::runtime_error( "can't write to pipe" );
        }
      }

#if 0
    void push( sockstream_t& s )
      {
        ctl _ctl;
        _ctl.cmd = tcp_stream;
        _ctl.data.ptr = static_cast<void *>(&s);

        write( pipefd[1], &_ctl, sizeof(ctl) );
      }
#endif

    void push( sockbuf_t& s )
      {
        ctl _ctl;
        _ctl.cmd = tcp_buffer;
        _ctl.data.ptr = static_cast<void *>(&s);

        errno = 0;
        int r = ::write( pipefd[1], &_ctl, sizeof(ctl) );
        if ( r < 0 || r != sizeof(ctl) ) {
          throw std::runtime_error( "can't write to pipe" );
        }
      }

    void pop( socks_processor_t& p, int _fd )
      {
        fd_info info = { fd_info::listener, 0, &p };
        std::tr2::lock_guard<std::tr2::mutex> lk( cll );
        closed_queue[_fd] = info;
        std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)&p << " " << std::tr2::getpid() << std::endl;
      }

    void final( socks_processor_t& p );

    void exit_notify( sockbuf_t* b, int fd )
      {
        fd_info info = { 0, reinterpret_cast<sockstream_t*>(b), 0 };
        std::tr2::lock_guard<std::tr2::mutex> lk( cll );
	closed_queue[fd] = info;
        std::cerr << __FILE__ << ":" << __LINE__ << " " << (void*)b << " " << std::tr2::getpid() << std::endl;
      }

  private:
    sockmgr( const sockmgr& )
      { }
    sockmgr& operator =( const sockmgr& )
      { return *this; }


#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<int,fd_info> fd_container_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<int, fd_info> fd_container_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<int, fd_info> fd_container_type;
#endif

    void io_worker();
    void cmd_from_pipe();
    void process_listener( epoll_event&, typename fd_container_type::iterator );
    void process_regular( epoll_event&, typename fd_container_type::iterator );

    int efd;
    int pipefd[2];
    std::tr2::thread *_worker;
    const int n_ret;

    fd_container_type descr;
    fd_container_type closed_queue;
    std::tr2::mutex cll;
    std::tr2::mutex dll;
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
struct hash<std::basic_sockstream2<charT, traits, _Alloc>* >
{
    size_t operator()(const std::basic_sockstream2<charT, traits, _Alloc>* __x) const
      { return reinterpret_cast<size_t>(__x); }
};

#ifdef __USE_STD_TR1
}
#endif

#if defined(__GNUC__) && (__GNUC__ < 4)
template<>
struct hash<void *>
{
   size_t operator()(const void *__x) const
     { return reinterpret_cast<size_t>(__x); }
};
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

#include <sockios/sp.cc>

#endif /* __SOCKIOS_SP_H */
