// -*- C++ -*- Time-stamp: <08/06/19 20:14:01 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __SOCKIOS_SOCKSRV_H
#define __SOCKIOS_SOCKSRV_H

#include <cerrno>
#include <mt/thread>
#include <mt/mutex>
#include <mt/condition_variable>

#ifdef STLPORT
#  include <unordered_map>
#  include <unordered_set>
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

template <class charT, class traits, class _Alloc> class basic_sockbuf;
template <class charT, class traits, class _Alloc> class basic_sockstream;
template <class charT, class traits, class _Alloc> class sock_processor_base;

template <class charT, class traits, class _Alloc>
class sock_processor_base :
	public sock_base,
        public basic_socket<charT,traits,_Alloc>
{
  private:
    typedef basic_socket<charT,traits,_Alloc> basic_socket_t;

  public:
    typedef basic_sockstream<charT,traits,_Alloc> sockstream_t;
    typedef basic_sockbuf<charT,traits,_Alloc>    sockbuf_t;

    struct adopt_close_t { };

    sock_processor_base() :
        _mode( ios_base::in | ios_base::out ),
        _state( ios_base::goodbit )
      { }

    explicit sock_processor_base( int port, sock_base::stype t = sock_base::sock_stream )
      { sock_processor_base::open( port, t, sock_base::inet ); }

    virtual ~sock_processor_base()
      {
        sock_processor_base::close();

        // Never uncomment next line:
        // basic_socket<charT,traits,_Alloc>::mgr->final( *this );
        // this lead to virtual fuction call, that is already pure here.
      }

    void open( const in_addr& addr, int port, sock_base::stype type, sock_base::protocol prot );

    void open( unsigned long addr, int port, sock_base::stype type, sock_base::protocol prot )
      {
        in_addr _addr;
        _addr.s_addr = htonl( addr );
        sock_processor_base::open( _addr, port, type, prot );
      }

    void open( int port, sock_base::stype type, sock_base::protocol prot )
      { sock_processor_base::open(INADDR_ANY, port, type, prot); }

    virtual void close();
#if 0
    virtual void stop() = 0;
#else
    virtual void stop()
      { /* abort(); */ }
    // void stop()
    //   { (this->*_real_stop)(); }
#endif

#if 0
    virtual sockbuf_t* operator ()( sock_base::socket_type fd, const sockaddr& ) = 0;
    virtual void operator ()( sock_base::socket_type fd, const adopt_close_t& ) = 0;
    virtual void operator ()( sock_base::socket_type fd ) = 0;
#else
    virtual sockbuf_t* operator ()( sock_base::socket_type fd, const sockaddr& )
      { abort(); return 0; }
    virtual void operator ()( sock_base::socket_type fd, const adopt_close_t& )
      { abort(); }
    virtual void operator ()( sock_base::socket_type fd )
      { abort(); }
#endif
  private:
    sock_processor_base( const sock_processor_base& );
    sock_processor_base& operator =( const sock_processor_base& ); 

  protected:
    void setoptions_unsafe( sock_base::so_t optname, bool on_off = true, int __v = 0 );
    sockstream_t* create_stream( int fd, const sockaddr& addr )
      {
        sockstream_t* s = new sockstream_t();
        if ( s != 0 ) {
          s->rdbuf()->_open_sockmgr( fd, addr );
        }
        return s;
      }

    void (sock_processor_base::*_real_stop)();

  public:
    bool is_open() const
      { std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck); return basic_socket_t::is_open_unsafe(); }
    bool good() const
      { return _state == ios_base::goodbit; }

    sock_base::socket_type fd() const
      { std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck); sock_base::socket_type tmp = basic_socket_t::fd_unsafe(); return tmp; }

    void shutdown( sock_base::shutdownflg dir );
    void setoptions( sock_base::so_t optname, bool on_off = true, int __v = 0 )
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

template <class Connect, class charT = char, class traits = std::char_traits<charT>, class _Alloc = std::allocator<charT>, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& ) = &Connect::connect >
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
         _in_work( true ),
         ploop( loop, this )
      { new( Init_buf ) Init(); /* base_t::_real_stop = &connect_processor::_xstop; */ }

    explicit connect_processor( int port ) :
        base_t( port, sock_base::sock_stream ),
        not_empty( *this ),
        _in_work( true ),
        ploop( loop, this )
      { new( Init_buf ) Init(); /* base_t::_real_stop = &connect_processor::_xstop; */ }

    virtual ~connect_processor()
      {
        connect_processor::close();

        if ( ploop.joinable() ) {
          ploop.join();
        }

        // basic_socket<charT,traits,_Alloc>::mgr->final( *this );

#if 0
        {
          std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
          std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
          if ( worker_pool.empty() && ready_pool.empty() ) {
            break;
          }

          for ( ; ; ) {
            
          }
        }
#endif


        {
          std::tr2::lock_guard<std::tr2::mutex> lk2( rdlock );
          cerr << __FILE__ << ":" << __LINE__ << " " << ready_pool.size() << endl; 
        }

        {
          std::tr2::lock_guard<std::tr2::mutex> lk2( wklock );
          cerr << __FILE__ << ":" << __LINE__ << " " << worker_pool.size() << endl;
#if 0
          for ( typename worker_pool_t::iterator i = worker_pool.begin(); i != worker_pool.end(); ++i ) {
            delete i->second.c;
            delete i->second.s;
          }
#endif
        }

        ((Init *)Init_buf)->~Init();
      }

    virtual void close();
    virtual void stop();

    void wait()
      { if ( ploop.joinable() ) { ploop.join(); } }

  private:
    virtual typename base_t::sockbuf_t* operator ()( sock_base::socket_type fd, const sockaddr& );
    virtual void operator ()( sock_base::socket_type fd, const typename base_t::adopt_close_t& );
    virtual void operator ()( sock_base::socket_type fd );

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
       bool operator ==( sock_base::socket_type fd ) const
         { return s == 0 ? (fd == -1) : (s->rdbuf()->fd() == fd); }

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
    typedef std::hash_map<sock_base::socket_type, processor> worker_pool_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<sock_base::socket_type, processor> worker_pool_t;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<sock_base::socket_type, processor> worker_pool_t;
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
    std::tr2::condition_variable cnd_inwk;
    std::tr2::thread ploop;

    friend struct _not_empty;
};

} // namesapce std

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

#include <sockios/socksrv.cc>

#endif /* __SOCKIOS_SOCKSRV_H */
