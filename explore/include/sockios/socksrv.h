// -*- C++ -*- Time-stamp: <09/03/04 14:09:01 ptr>

/*
 * Copyright (c) 2008, 2009
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
#include <list>
#include <functional>
#include <exception>

// #include <boost/shared_ptr.hpp>

#include <mt/callstack.h>

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
        _state( ios_base::goodbit ),
        _chk( *this ),
        _rcount( 0 )
      { }

    explicit sock_processor_base( int port, sock_base::stype t = sock_base::sock_stream ) :
        _chk( *this ),
        _rcount( 0 )
      { sock_processor_base::open( port, t, sock_base::inet ); }

    virtual ~sock_processor_base()
      {        
        sock_processor_base::_close();

        std::tr2::unique_lock<std::tr2::mutex> lk(_cnt_lck);
        _cnt_cnd.wait( lk, _chk );
        // if ( !_cnt_cnd.timed_wait( lk, std::tr2::seconds(1), _chk ) ) { // <-- debug
        //   std::cerr << __FILE__ << ":" << __LINE__ << " " << _rcount << std::endl;
        // }
      }

    void addref()
      {
        std::tr2::lock_guard<std::tr2::mutex> lk(_cnt_lck);
        ++_rcount;
      }

    void release()
      {
        std::tr2::lock_guard<std::tr2::mutex> lk(_cnt_lck);
        if ( --_rcount == 0 ) {
          _cnt_cnd.notify_one();
        }
        if ( _rcount < 0 ) { // <-- debug
          xmt::callstack( std::cerr );
        }
      }

    int count()
      {
        std::tr2::lock_guard<std::tr2::mutex> lk(_cnt_lck);
        int tmp = _rcount;
        return tmp;
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

    virtual void close()
      { _close(); }
    virtual void stop()
      { }

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

    void _close();

    // void (sock_processor_base::*_real_stop)();

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

    class _cnt_checker
    {
      public:
        _cnt_checker( sock_processor_base<charT,traits,_Alloc>& _p ) :
            p( _p )
          { }

      private:
        sock_processor_base<charT,traits,_Alloc>& p;

      public:
        bool operator ()() const
          { return p._rcount == 0; }
    };

    _cnt_checker _chk;
    std::tr2::mutex _cnt_lck;
    std::tr2::condition_variable _cnt_cnd;
    int _rcount;

    friend class _cnt_checker;
};

typedef sock_processor_base<char,char_traits<char>,allocator<char> > sock_basic_processor;

template <class Connect, class charT = char, class traits = std::char_traits<charT>, class _Alloc = std::allocator<charT>, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& ) = &Connect::connect >
class connect_processor :
        public sock_processor_base<charT,traits,_Alloc>
{
  private:
    typedef sock_processor_base<charT,traits,_Alloc> base_t;
    // typedef typename std::detail::processor<Connect,charT,traits,_Alloc> processor;

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
        connect_processor::_close();

        // _stop();

        if ( ploop.joinable() ) {
          ploop.join();
        }

        // {
        //   std::tr2::lock_guard<std::tr2::mutex> lk2( rdlock );
        //   cerr << __FILE__ << ":" << __LINE__ << " " << ready_pool.size() << endl; 
        // }

        // {
        //   std::tr2::lock_guard<std::tr2::mutex> lk2( wklock );
        //   cerr << __FILE__ << ":" << __LINE__ << " " << worker_pool.size() << endl;
        // }

        ((Init *)Init_buf)->~Init();
      }

    virtual void close()
      { connect_processor::_close(); }
    virtual void stop()
      { connect_processor::_stop(); }

    void wait()
      { if ( ploop.joinable() ) { ploop.join(); } }

  private:
    virtual typename base_t::sockbuf_t* operator ()( sock_base::socket_type fd, const sockaddr& );
    virtual void operator ()( sock_base::socket_type fd, const typename base_t::adopt_close_t& );
    virtual void operator ()( sock_base::socket_type fd );

    static void loop( connect_processor* me )
      { me->worker(); }

    void worker();

    class finish :
        public std::exception
    {
    };

  private:
    connect_processor( const connect_processor& )
      { }

    connect_processor& operator =( const connect_processor& )
      { return *this; }

    class processor
    {
      public:
        processor() :
            c(0),
            s(0)
          { }
        processor( Connect* __c, typename sock_processor_base<charT,traits,_Alloc>::sockstream_t* __s ) :
            c(__c),
            s(__s)
          { }

      private:
        processor( /* const */ processor& p ) // :
        // c( p.c ),
        // s( p.s )
          { this->swap( p ); }

      public:
        processor( const processor& ) :
            c( 0 ),
            s( 0 )
          { }

        ~processor()
          {
            delete c;
            delete s;
          }

      public:
        void swap( processor& p )
          { std::swap(c, p.c); std::swap(s, p.s); }

      private:
        processor& operator =( const processor& p )
          { c = p.c; s = p.s; return *this; }

      public:
        Connect* c;
        typename sock_processor_base<charT,traits,_Alloc>::sockstream_t* s;

        // bool operator ==( const processor& p ) const
        //   { return s == p.s; }
        // bool operator ==( const typename sock_processor_base<charT,traits,_Alloc>::sockstream_t* st ) const
        //   { return const_cast<const typename sock_processor_base<charT,traits,_Alloc>::sockstream_t*>(s) == st; }
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

    void _close()
      { base_t::_close(); }
    void _stop();

#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<sock_base::socket_type, processor> worker_pool_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<sock_base::socket_type, processor> worker_pool_t;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<sock_base::socket_type, processor> worker_pool_t;
#endif
    typedef std::list<processor> ready_pool_t;

    struct _not_empty
    {
      _not_empty( connect_processor& p ) :
          me( p )
        { }

      bool operator()() const
        { return !me.ready_pool.empty() || !me._in_work; }

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
