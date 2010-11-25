// -*- C++ -*- Time-stamp: <2010-11-09 15:27:02 ptr>

/*
 * Copyright (c) 2008-2010
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
#include <vector>
#include <queue>
#include <functional>
#include <exception>

namespace std {

template <class charT, class traits, class _Alloc> class basic_sockbuf;
template <class charT, class traits, class _Alloc> class basic_sockstream;
template <class charT, class traits, class _Alloc> class sock_processor_base;

namespace detail {
extern std::tr2::mutex _se_lock;
extern std::ostream* _se_stream;
}

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

    explicit sock_processor_base( const char* path, sock_base::stype t = sock_base::sock_dgram ) :
        _chk( *this ),
        _rcount( 0 )
      {
        sock_processor_base::open( path, t );
      }

    virtual ~sock_processor_base()
      {        
        sock_processor_base::_close();
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
      }

    int count()
      {
        std::tr2::lock_guard<std::tr2::mutex> lk(_cnt_lck);
        int tmp = _rcount;
        return tmp;
      }

    void open( const in_addr& addr, int port, sock_base::stype type, sock_base::protocol prot );
    void open( const char* path, sock_base::stype type );

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

    virtual sockbuf_t* operator ()( sock_base::socket_type, const sockaddr& )
      { abort(); return 0; }
    virtual void operator ()( sock_base::socket_type, const adopt_close_t& )
      { abort(); }
    virtual void operator ()( sock_base::socket_type )
      { abort(); }

    enum traceflags {
      notrace = 0,
      tracefault = 1
    };

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
    std::ios_base::openmode _mode;  // open mode
    std::ios_base::iostate  _state; // state flags

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
        _in_work( true )
      { 
        new( Init_buf ) Init();
        ploop = new std::tr2::thread( loop, this );
      }

    explicit connect_processor( int port ) :
        base_t( port, sock_base::sock_stream ),
        not_empty( *this ),
        _in_work( true )
      { 
        new( Init_buf ) Init();   
        ploop = new std::tr2::thread( loop, this );
      }

    explicit connect_processor( const char* path ) :
        base_t( path, sock_base::sock_stream ),
        not_empty( *this ),
        _in_work( true )
      { 
        new( Init_buf ) Init();
        ploop = new std::tr2::thread( loop, this );
      }

    virtual ~connect_processor()
      {
        connect_processor::_close();

        connect_processor::wait();

        delete ploop;

        Init* tmp = reinterpret_cast<Init*>(Init_buf);
        tmp->~Init();
      }

    virtual void close()
      { connect_processor::_close(); }
    virtual void stop()
      { connect_processor::_stop(); }

    void wait()
      { if ( ploop->joinable() ) { ploop->join(); } }

    typedef void (*at_func_type)( std::basic_sockstream<charT,traits,_Alloc>& );

    void at_connect( at_func_type f )
      { _at_connect.push_back( f ); }
    void at_data( at_func_type f )
      { _at_data.push_back( f ); }
    void at_disconnect( at_func_type f )
      { _at_disconnect.push_back( f ); }

    static std::ostream* settrs( std::ostream* );

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

    connect_processor& operator=( const connect_processor& )
      { return *this; }

    struct processor
    {
      processor( Connect* __c = 0, typename base_t::sockstream_t* __s = 0 ) :
          c(__c),
          s(__s)
        { }

      Connect* c;
      typename base_t::sockstream_t* s;
    };

    enum socket_operation_type {
      socket_open = 0,
      socket_read,
      socket_close
    };

    struct request_t
    {
      socket_operation_type operation_type;
      sock_base::socket_type fd;
      processor p;

      request_t() :
          fd( -1 ),
          p()
        { }

      request_t( socket_operation_type _operation_type,
                 sock_base::socket_type _fd ) :
          operation_type( _operation_type ),
          fd( _fd ),
          p()
        { }

      request_t( socket_operation_type _operation_type,
                 sock_base::socket_type _fd,
                 const processor& _p ) :
          operation_type( _operation_type ),
          fd( _fd ),
          p( _p )
        { }
    };

    void _close()
      { base_t::_close(); }
    void _stop();

#ifdef __USE_STLPORT_HASH
    typedef std::hash_map< sock_base::socket_type, processor > opened_pool_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map< sock_base::socket_type, processor > opened_pool_t;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map< sock_base::socket_type, processor > opened_pool_t;
#endif
    
    typedef std::queue< request_t > ready_queue_t;

    void process_request( const request_t& request );
    void feed_data_to_processor( processor& p );
    void clear_opened_pool();

    struct _not_empty
    {
      _not_empty( connect_processor& p ) :
          me( p )
        { }

      bool operator()() const
        { return !me.ready_queue.empty() || !me._in_work; }

      connect_processor& me;
    } not_empty;


    opened_pool_t opened_pool;

    ready_queue_t ready_queue;
    std::tr2::mutex ready_lock;
    std::tr2::condition_variable ready_cnd;

    typedef std::vector<at_func_type> at_container_type;

    at_container_type _at_connect;
    at_container_type _at_data;
    at_container_type _at_disconnect;

    bool _in_work;
    std::tr2::thread* ploop;

    static std::tr2::mutex _lock_tr;
    static unsigned _trflags;
    static std::ostream* _trs;
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
