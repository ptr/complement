// -*- C++ -*- Time-stamp: <08/04/02 10:25:57 ptr>

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
      { sock_processor_base::close(); }

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

    virtual void operator ()( sockstream_t& s, const adopt_new_t& ) = 0;
    virtual void operator ()( sockstream_t& s, const adopt_close_t& ) = 0;
    virtual void operator ()( sockstream_t& s, const adopt_data_t& ) = 0;

  private:
    sock_processor_base( const sock_processor_base& );
    sock_processor_base& operator =( const sock_processor_base& ); 

  protected:
    void setoptions_unsafe( sock_base2::so_t optname, bool on_off = true, int __v = 0 );

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

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( const in_addr& addr, int port, sock_base2::stype type, sock_base2::protocol prot )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
#ifdef WIN32
  ::WSASetLastError( 0 );
#endif
  if ( prot == sock_base2::inet ) {
    basic_socket_t::_fd = socket( PF_INET, type, 0 );
    if ( basic_socket_t::_fd == -1 ) {
      _state |= ios_base::failbit | ios_base::badbit;
      return;
    }
    // _open = true;
    basic_socket_t::_address.inet.sin_family = AF_INET;
    basic_socket_t::_address.inet.sin_port = htons( port );
    basic_socket_t::_address.inet.sin_addr.s_addr = addr.s_addr;

    if ( type == sock_base2::sock_stream || type == sock_base2::sock_seqpacket ) {
      // let's try reuse local address
      setoptions_unsafe( sock_base2::so_reuseaddr, true );
    }

    if ( ::bind( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof(basic_socket_t::_address) ) == -1 ) {
      _state |= ios_base::failbit;
#ifdef WIN32
      ::closesocket( basic_socket_t::_fd );
#else
      ::close( basic_socket_t::_fd );
#endif
      basic_socket_t::_fd = -1;
      return;
    }

    if ( type == sock_base2::sock_stream || type == sock_base2::sock_seqpacket ) {
      // I am shure, this is socket of type SOCK_STREAM | SOCK_SEQPACKET,
      // so don't check return code from listen
      ::listen( basic_socket_t::_fd, SOMAXCONN );
      basic_socket_t::mgr->push( *this );
    }
  } else if ( prot == sock_base2::local ) {
    return;
  } else {
    return;
  }
  _state = ios_base::goodbit;

  return;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::close()
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( !basic_socket_t::is_open_unsafe() ) {
    return;
  }
  basic_socket<charT,traits,_Alloc>::mgr->pop( *this, basic_socket_t::_fd );
#ifdef WIN32
  ::closesocket( basic_socket_t::_fd );
#else
  ::shutdown( basic_socket_t::_fd, 2 );
  ::close( basic_socket_t::_fd );
#endif
  basic_socket_t::_fd = -1;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::shutdown( sock_base2::shutdownflg dir )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    if ( (dir & (sock_base2::stop_in | sock_base2::stop_out)) ==
         (sock_base2::stop_in | sock_base2::stop_out) ) {
      ::shutdown( basic_socket_t::_fd, 2 );
    } else if ( dir & sock_base2::stop_in ) {
      ::shutdown( basic_socket_t::_fd, 0 );
    } else if ( dir & sock_base2::stop_out ) {
      ::shutdown( basic_socket_t::_fd, 1 );
    }
  }
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::setoptions_unsafe( sock_base2::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  if ( basic_socket_t::is_open_unsafe() ) {
    if ( optname != sock_base2::so_linger ) {
      int turn = on_off ? 1 : 0;
      if ( setsockopt( basic_socket_t::_fd, SOL_SOCKET, (int)optname, (const void *)&turn,
                       (socklen_t)sizeof(int) ) != 0 ) {
        _state |= ios_base::failbit;
      }
    } else {
      linger l;
      l.l_onoff = on_off ? 1 : 0;
      l.l_linger = __v;
      if ( setsockopt( basic_socket_t::_fd, SOL_SOCKET, (int)optname, (const void *)&l,
                       (socklen_t)sizeof(linger) ) != 0 ) {
        _state |= ios_base::failbit;
      }
      
    }
  } else {
    _state |= ios_base::failbit;
  }
#endif // __unix
}

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

        ((Init *)Init_buf)->~Init();
      }

    virtual void close();

    void wait()
      { if ( ploop.joinable() ) { ploop.join(); } }

  private:
    virtual void operator ()( typename base_t::sockstream_t& s, const typename base_t::adopt_new_t& );
    virtual void operator ()( typename base_t::sockstream_t& s, const typename base_t::adopt_close_t& );
    virtual void operator ()( typename base_t::sockstream_t& s, const typename base_t::adopt_data_t& );


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
    typedef std::hash_map<typename base_t::sockstream_t*,Connect*> worker_pool_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<typename base_t::sockstream_t*,Connect*> worker_pool_t;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<typename base_t::sockstream_t*,Connect*> worker_pool_t;
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

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
int connect_processor<Connect, charT, traits, _Alloc, C>::Init::_count = 0;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
bool connect_processor<Connect, charT, traits, _Alloc, C>::Init::_at_fork = false;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
std::tr2::mutex connect_processor<Connect, charT, traits, _Alloc, C>::Init::_init_lock;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::_guard( int direction )
{
  if ( direction ) {
    std::tr2::lock_guard<std::tr2::mutex> lk( _init_lock );
    if ( _count++ == 0 ) {
#ifdef __FIT_PTHREADS
      if ( !_at_fork ) { // call only once
        if ( pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child ) ) {
          // throw system_error
        }
        _at_fork = true;
      }
#endif
//       _sock_processor_base::_idx = std::tr2::this_thread::xalloc();
    }
  } else {
    std::tr2::lock_guard<std::tr2::mutex> lk( _init_lock );
    if ( --_count == 0 ) {

    }
  }
}

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_prepare()
{ _init_lock.lock(); }

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_child()
{
  _init_lock.unlock();

  if ( _count != 0 ) {
    // std::cerr << "SHOULD NEVER HAPPEN!!!!\n";
    throw std::logic_error( "Fork while connect_processor working may has unexpected behaviour in child process" );
  }
  // _sock_processor_base::_idx =  std::tr2::this_thread::xalloc();
}

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_parent()
{ _init_lock.unlock(); }

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
char connect_processor<Connect, charT, traits, _Alloc, C>::Init_buf[128];

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>                                                             
void connect_processor<Connect, charT, traits, _Alloc, C>::close()
{
  base_t::close();

  { 
    std::tr2::lock_guard<std::tr2::mutex> lk(inwlock);
    _in_work = false; // <--- set before cnd.notify_one(); (below in this func)
  }

  std::tr2::lock_guard<std::tr2::mutex> lk2( rdlock );
  ready_pool.push_back( processor() ); // make ready_pool not empty
  // std::cerr << "=== " << ready_pool.size() << std::endl;
  cnd.notify_one();
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::sockstream_t& s, const typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::adopt_new_t& )
{
  Connect* c = new Connect( s );
  if ( s.rdbuf()->in_avail() ) {
    std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
    ready_pool.push_back( processor( c, &s ) );
    cnd.notify_one();
  } else {
    std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
    worker_pool.insert( std::make_pair( &s, c ) );
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::sockstream_t& s, const typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::adopt_close_t& )
{
  {
    std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
    typename worker_pool_t::iterator i = worker_pool.find( &s );
    if ( i != worker_pool.end() ) {
      delete i->second;
      // std::cerr << "oops\n";
      worker_pool.erase( i );
      return;
    }
  }

  Connect* c = 0;
  {
    std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
    typename ready_pool_t::iterator j = std::find( ready_pool.begin(), ready_pool.end(), /* std::bind2nd( typename processor::equal_to(), &s ) */ &s );
    if ( j != ready_pool.end() ) {
      // std::cerr << "oops 2\n";
      c = j->c;
      ready_pool.erase( j );
    }
  }
  if ( c != 0 ) {
    (c->*C)( s );
    delete c;
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::sockstream_t& s, const typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::adopt_data_t& )
{
  Connect* c;

  {
    std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
    typename worker_pool_t::const_iterator i = worker_pool.find( &s );
    if ( i == worker_pool.end() ) {
      return;
    }
    c = i->second;
    worker_pool.erase( i );
  }

  std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
  ready_pool.push_back( processor( c, &s ) );
  cnd.notify_one();
  // std::cerr << "notify data " << (void *)c << " " << ready_pool.size() << std::endl;
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
bool connect_processor<Connect, charT, traits, _Alloc, C>::pop_ready( processor& p )
{
  {
    std::tr2::unique_lock<std::tr2::mutex> lk( rdlock );

    cnd.wait( lk, not_empty );
    p = ready_pool.front(); // it may contain p.c == 0,  p.s == 0, if !in_work()
    ready_pool.pop_front();
    // std::cerr << "pop 1\n";
    if ( p.c == 0 ) { // wake up, but _in_work may be still true here (in processor pipe?),
      return false;   // even I know that _in_work <- false before notification...
    }                 // so, check twice
  }

  // std::cerr << "pop 2\n";

  std::tr2::lock_guard<std::tr2::mutex> lk(inwlock);
  return _in_work ? true : false;
}


template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream2<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::worker()
{
  _in_work = true;

  processor p;

  while ( pop_ready( p ) ) {
    std::cerr << "worker 1\n";
    (p.c->*C)( *p.s );
    std::cerr << "worker 2\n";
    if ( p.s->rdbuf()->in_avail() ) {
      std::tr2::lock_guard<std::tr2::mutex> lk( rdlock );
      ready_pool.push_back( p );
    } else {
      std::tr2::lock_guard<std::tr2::mutex> lk( wklock );
      worker_pool[p.s] = p.c;
    }
    std::cerr << "worker 3\n";
  }
}

namespace detail {

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
      }

    void exit_notify( sockbuf_t* b, int fd )
      {
        fd_info info = { 0, reinterpret_cast<sockstream_t*>(b), 0 };
        std::tr2::lock_guard<std::tr2::mutex> lk( cll );
	closed_queue[fd] = info;
      }

  private:
    sockmgr( const sockmgr& )
      { }
    sockmgr& operator =( const sockmgr& )
      { return *this; }

    void io_worker();

    int efd;
    int pipefd[2];
    std::tr2::thread *_worker;
    const int n_ret;

#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<int,fd_info> fd_container_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<int, fd_info> fd_container_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<int, fd_info> fd_container_type;
#endif

    fd_container_type descr;
    fd_container_type closed_queue;
    std::tr2::mutex cll;
};

template<class charT, class traits, class _Alloc>
void sockmgr<charT,traits,_Alloc>::io_worker()
{
  epoll_event ev[/*n_ret*/ 512 ];

/*
  ctl _xctl;
  int r = read( pipefd[0], &_xctl, sizeof(ctl) );

  if ( _xctl.cmd == rqstart ) {
    std::cerr << "io_worker fine" << std::endl;
  } else {
    std::cerr << "io_worker not fine, " << r << ", " << errno << std::endl;
  }
*/

  try {
  for ( ; ; ) {
    int n = epoll_wait( efd, &ev[0], /* n_ret */ 512, -1 );
    if ( n < 0 ) {
      if ( errno == EINTR ) {
        continue;
      }
      // throw system_error
    }
    // std::cerr << "epoll see " << n << std::endl;
    for ( int i = 0; i < n; ++i ) {
      // std::cerr << "epoll i = " << i << std::endl;
      if ( ev[i].data.fd == pipefd[0] ) {
        // std::cerr << "on pipe\n";
        epoll_event ev_add;
        ctl _ctl;
        int r = read( pipefd[0], &_ctl, sizeof(ctl) );
        if ( r < 0 ) {
          // throw system_error
          // std::cerr << "Read pipe\n";
        } else if ( r == 0 ) {
          std::cerr << "Read pipe 0\n";
          return;
        }

        switch ( _ctl.cmd ) {
          case listener:
            ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
            ev_add.data.fd = static_cast<socks_processor_t*>(_ctl.data.ptr)->fd();
            if ( ev_add.data.fd >= 0 ) {
              if ( fcntl( ev_add.data.fd, F_SETFL, fcntl( ev_add.data.fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
                // std::cerr << "xxx " << errno << " " << std::tr2::getpid() << std::endl;
                throw std::runtime_error( "can't establish nonblock mode on listener" );
              }
              fd_info new_info = { fd_info::listener, 0, static_cast<socks_processor_t*>(_ctl.data.ptr) };
              descr[ev_add.data.fd] = new_info;
              if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
                descr.erase( ev_add.data.fd );
                // throw system_error
              }
            }
            break;
#if 0
          case tcp_stream:
            ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
            ev_add.data.fd = static_cast<sockstream_t*>(_ctl.data.ptr)->rdbuf()->fd();
            if ( ev_add.data.fd >= 0 ) {
              fd_info new_info = { 0, static_cast<sockstream_t*>(_ctl.data.ptr), 0 };
              descr[ev_add.data.fd] = new_info;
              if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
                descr.erase( ev_add.data.fd );
                // throw system_error
              }
            }
            break;
#endif
          case tcp_buffer:
            ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
            ev_add.data.fd = static_cast<sockbuf_t*>(_ctl.data.ptr)->fd();
            if ( ev_add.data.fd >= 0 ) {
              fd_info new_info = { fd_info::buffer, static_cast<sockstream_t* /* sockbuf_t* */ >(_ctl.data.ptr), 0 };
              descr[ev_add.data.fd] = new_info;
              if ( epoll_ctl( efd, EPOLL_CTL_ADD, ev_add.data.fd, &ev_add ) < 0 ) {
                descr.erase( ev_add.data.fd );
                // throw system_error
              }
            }
            break;
          case rqstop:
            // std::cerr << "Stop request\n";
            return;
            break;
        }

        continue;
      }
      // std::cerr << "#\n";

      typename fd_container_type::iterator ifd = descr.find( ev[i].data.fd );
      if ( ifd == descr.end() ) {
        throw std::logic_error( "file descriptor in epoll, but not in descr[]" );
      }

      fd_info& info = ifd->second;
      if ( info.flags & fd_info::listener ) {
        // std::cerr << "%\n";
        if ( ev[i].events & EPOLLRDHUP ) {
          epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 );
          // walk through descr and detach every .p ?
          descr.erase( ifd );
          std::cerr << "Remove listener EPOLLRDHUP\n";
        } else if ( ev[i].events & EPOLLIN ) {
          sockaddr addr;
          socklen_t sz = sizeof( sockaddr_in );

          for ( ; ; ) {
            int fd = accept( ev[i].data.fd, &addr, &sz );
            if ( fd < 0 ) {
              std::cerr << "Accept, listener # " << ev[i].data.fd << ", errno " << errno << std::endl;
              if ( (errno == EINTR) || (errno == ECONNABORTED) /* || (errno == ERESTARTSYS) */ ) {
                continue;
              }
              if ( !(errno == EAGAIN || errno == EWOULDBLOCK) ) {
                // std::cerr << "Accept, listener " << ev[i].data.fd << ", errno " << errno << std::endl;
                // throw system_error ?
              }
#if 0
            {
              std::tr2::lock_guard<std::tr2::mutex> lck( cll );
              typename fd_container_type::iterator closed_ifd = closed_queue.find( ev[i].data.fd );
              if ( closed_ifd != closed_queue.end() ) {
                typename fd_container_type::iterator ifd = descr.begin();
                for ( ; ifd != descr.end(); ) {
                  if ( ifd->second.p == closed_ifd->second.p ) {
                    descr.erase( ifd++ );
                  } else {
                    ++ifd
                  }
                }
                closed_queue.erase( closed_ifd );
              }
            }

#endif
              break;
            }
            // std::cerr << "listener accept " << fd << std::endl;
            if ( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
              throw std::runtime_error( "can't establish nonblock mode" );
            }
            sockstream_t* s;

            try {
              s = new sockstream_t();
              if ( s->rdbuf()->_open_sockmgr( fd, addr ) ) {
                epoll_event ev_add;
                ev_add.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
                ev_add.data.fd = fd;
                fd_info new_info = { fd_info::owner, s, info.p };
                descr[fd] = new_info;

                if ( epoll_ctl( efd, EPOLL_CTL_ADD, fd, &ev_add ) < 0 ) {
                  std::cerr << "Accept, add " << fd << ", errno " << errno << std::endl;
                  descr.erase( fd );
                  // throw system_error
                }
                std::cerr << "adopt_new_t()\n";
                std::tr2::lock_guard<std::tr2::mutex> lk( cll );
                typename fd_container_type::iterator closed_ifd = closed_queue.begin();
                for ( ; closed_ifd != closed_queue.end(); ++closed_ifd ) {
                  if ( closed_ifd->second.p == info.p ) {
                    break;
                  }
                }
                if ( closed_ifd == closed_queue.end() ) {
                  (*info.p)( *s, typename socks_processor_t::adopt_new_t() );
                } else {
                  std::cerr << "@@@ 1\n" << std::endl;
                }
              } else {
                std::cerr << "Accept, delete " << fd << std::endl;
                delete s;
              }
            }
            catch ( const std::bad_alloc& ) {
              // nothing
            }
            catch ( ... ) {
              descr.erase( fd );
              delete s;
            }
          }
        } else {
          // std::cerr << "listener: " << std::hex << ev[i].events << std::dec << std::endl;
        }
      } else {
        // std::cerr << "not listener\n";
        if ( ev[i].events & EPOLLIN ) {
          if ( (info.flags & fd_info::owner) == 0 ) {
            // marginal case: me not owner (registerd via push(),
            // when I owner, I know destroy point),
            // already closed, but I not see closed event yet;
            // object may be deleted already, so I can't
            // call b->egptr() etc. here
            std::tr2::lock_guard<std::tr2::mutex> lck( cll );
            typename fd_container_type::iterator closed_ifd = closed_queue.find( ev[i].data.fd );
            if ( closed_ifd != closed_queue.end() ) {
              closed_queue.erase( closed_ifd );
              if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
                // throw system_error
              }
              descr.erase( ifd );
              continue;
            }
          }
          sockbuf_t* b = (info.flags & fd_info::buffer != 0) ? info.s.b : info.s.s->rdbuf();
          errno = 0;
          for ( ; ; ) {
            if ( b->_ebuf == b->egptr() ) {
              // process extract data from buffer too slow for us!
              if ( (info.flags & fd_info::level_triggered) == 0 ) {
                epoll_event xev;
                xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
                xev.data.fd = ev[i].data.fd;
                info.flags |= fd_info::level_triggered;
                if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev[i].data.fd, &xev ) < 0 ) {
                  std::cerr << "X " << ev[i].data.fd << ", " << errno << std::endl;
                }
              }
              std::cerr << "Z " << ev[i].data.fd << ", " << errno << std::endl;
              if ( info.p != 0 ) {
                std::tr2::lock_guard<std::tr2::mutex> lk( cll );
                typename fd_container_type::iterator closed_ifd = closed_queue.begin();
                for ( ; closed_ifd != closed_queue.end(); ++closed_ifd ) {
                  if ( closed_ifd->second.p == info.p ) {
                    break;
                  }
                }
                if ( closed_ifd == closed_queue.end() ) {
                  (*info.p)( *info.s.s, typename socks_processor_t::adopt_data_t() );
                } else {
                  std::cerr << "@@@ 2\n" << std::endl;
                }
              }
              break;
            }
            std::cerr << "ptr " <<  (void *)b->egptr() << ", " << errno << std::endl;
            long offset = read( ev[i].data.fd, b->egptr(), sizeof(charT) * (b->_ebuf - b->egptr()) );
            std::cerr << "offset " << offset << ", " << errno << std::endl;
            if ( offset < 0 ) {
              if ( (errno == EAGAIN) || (errno == EINTR) ) {
                errno = 0;
                epoll_event xev;
                xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
                xev.data.fd = ev[i].data.fd;
                epoll_ctl( efd, EPOLL_CTL_MOD, ev[i].data.fd, &xev );
                break;
              } else {
                switch ( errno ) {
                  // case EINTR:      // read was interrupted
                    // continue;
                  //  break;
                  case EFAULT:     // Bad address
                  case ECONNRESET: // Connection reset by peer
                    ev[i].events |= EPOLLRDHUP; // will be processed below
                    break;
                  default:
                    std::cerr << "not listener, other " << ev[i].data.fd << std::hex << ev[i].events << std::dec << " : " << errno << std::endl;
                    break;
                }
                break;
              }
            } else if ( offset > 0 ) {
              offset /= sizeof(charT); // if offset % sizeof(charT) != 0, rest will be lost!
            
              if ( info.flags & fd_info::level_triggered ) {
                epoll_event xev;
                xev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT;
                xev.data.fd = ev[i].data.fd;
                info.flags &= ~static_cast<unsigned>(fd_info::level_triggered);
                if ( epoll_ctl( efd, EPOLL_CTL_MOD, ev[i].data.fd, &xev ) < 0 ) {
                  std::cerr << "Y " << ev[i].data.fd << ", " << errno << std::endl;
                }
              }
              std::tr2::lock_guard<std::tr2::mutex> lk( b->ulck );
              b->setg( b->eback(), b->gptr(), b->egptr() + offset );
              b->ucnd.notify_one();
              if ( info.p != 0 ) {
                // std::cerr << "data here" << std::endl;
                std::tr2::lock_guard<std::tr2::mutex> lk( cll );
                typename fd_container_type::iterator closed_ifd = closed_queue.begin();
                for ( ; closed_ifd != closed_queue.end(); ++closed_ifd ) {
                  if ( closed_ifd->second.p == info.p ) {
                    break;
                  }
                }
                if ( closed_ifd == closed_queue.end() ) {
                  (*info.p)( *info.s.s, typename socks_processor_t::adopt_data_t() );
                } else {
                  std::cerr << "@@@ 3\n" << std::endl;
                }
              }
            } else {
              std::cerr << "K " << ev[i].data.fd << ", " << errno << std::endl;
              // EPOLLRDHUP may be missed in kernel, but offset 0 is the same
              ev[i].events |= EPOLLRDHUP; // will be processed below
              break;
            }
          }
        } else {
          std::cerr << "Q\n";
        }
        if ( (ev[i].events & EPOLLRDHUP) || (ev[i].events & EPOLLHUP) || (ev[i].events & EPOLLERR) ) {
          // std::cerr << "Poll EPOLLRDHUP " << ev[i].data.fd << ", " << errno << std::endl;
          if ( epoll_ctl( efd, EPOLL_CTL_DEL, ifd->first, 0 ) < 0 ) {
            // throw system_error
          }
          if ( info.p != 0 ) {
            std::tr2::lock_guard<std::tr2::mutex> lk( cll );
            typename fd_container_type::iterator closed_ifd = closed_queue.begin();
            for ( ; closed_ifd != closed_queue.end(); ++closed_ifd ) {
              if ( closed_ifd->second.p == info.p ) {
                break;
              }
            }
            if ( closed_ifd == closed_queue.end() ) {
              (*info.p)( *info.s.s, typename socks_processor_t::adopt_close_t() );
            } else {
              std::cerr << "@@@ 4\n" << std::endl;
            }
          }
          if ( (info.flags & fd_info::owner) != 0 ) {
            delete info.s.s;
          } else {
            if ( (info.flags & fd_info::buffer) != 0 ) {
              info.s.b->close();
            } else {
              info.s.s->close();
            }
            std::tr2::lock_guard<std::tr2::mutex> lck( cll );
            closed_queue.erase( ev[i].data.fd );
          }
          descr.erase( ifd );
        }
        // if ( ev[i].events & EPOLLHUP ) {
        //   std::cerr << "Poll HUP" << std::endl;
        // }
        // if ( ev[i].events & EPOLLERR ) {
        //   std::cerr << "Poll ERR" << std::endl;
        // }
        if ( ev[i].events & EPOLLPRI ) {
          std::cerr << "Poll PRI" << std::endl;
        }
        if ( ev[i].events & EPOLLRDNORM ) {
          std::cerr << "Poll RDNORM" << std::endl;
        }
        if ( ev[i].events & EPOLLRDBAND ) {
          std::cerr << "Poll RDBAND" << std::endl;
        }
        if ( ev[i].events & EPOLLMSG ) {
          std::cerr << "Poll MSG" << std::endl;
        }
      }
    }
  }
  }
  catch ( std::exception& e ) {
    std::cerr << e.what() << std::endl;
  }
}

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

#endif /* __SOCKIOS_SP_H */
