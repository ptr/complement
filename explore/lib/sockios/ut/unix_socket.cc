// -*- C++ -*- Time-stamp: <09/07/03 20:39:24 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "unix_socket.h"

#include <exam/suite.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/shm.h>
#include <sys/wait.h>

// #include <sockios/syslog.h>
// #include <locale>

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

using namespace std;
using namespace std::tr2;

class simple_us_mgr :
    public sock_basic_processor
{
  public:
    simple_us_mgr() :
        sock_basic_processor()
      { fill( buf, buf + 1024, 0 ); }

    simple_us_mgr( const char* path, sock_base::stype t = sock_base::sock_dgram ) :
        sock_basic_processor( path, t )
      { fill( buf, buf + 1024, 0 ); }

    ~simple_us_mgr()
      { }

  protected:
    virtual sock_basic_processor::sockbuf_t* operator ()( sock_base::socket_type fd, const sockaddr& )
      {
        lock_guard<mutex> lk(lock);
        ++n_cnt;
        size_t n = ::read( fd, buf, 1024 );
        cnd.notify_one();
        return 0;
      }
    virtual void operator ()( sock_base::socket_type, const adopt_close_t& )
      { lock_guard<mutex> lk(lock); ++c_cnt; cnd.notify_one(); }
    virtual void operator ()( sock_base::socket_type )
      { lock_guard<mutex> lk(lock); ++d_cnt; cnd.notify_one(); }

  public:
    static mutex lock;
    static int n_cnt;
    static int c_cnt;
    static int d_cnt;
    static condition_variable cnd;
    static char buf[];

    static bool n_cnt_check()
      { return n_cnt == 1; }
    static bool c_cnt_check()
      { return c_cnt == 1; }
    static bool d_cnt_check()
      { return d_cnt == 1; }
};

mutex simple_us_mgr::lock;
int simple_us_mgr::n_cnt = 0;
int simple_us_mgr::c_cnt = 0;
int simple_us_mgr::d_cnt = 0;
condition_variable simple_us_mgr::cnd;
char simple_us_mgr::buf[1024];

int EXAM_IMPL(unix_sockios_test::core_test)
{
  const char* f = "/tmp/sock_test";
  // const char* f = "/dev/log";
  {
    simple_us_mgr s( f );

    EXAM_CHECK( s.is_open() );
    EXAM_CHECK( s.good() );

    {
      sockstream str( f, sock_base::sock_dgram );

      EXAM_CHECK( str.is_open() );
      EXAM_CHECK( str.good() );

      {
        unique_lock<mutex> lk( simple_us_mgr::lock );

        // EXAM_CHECK( simple_us_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr::n_cnt_check ) );
      }
      {
        lock_guard<mutex> lk(simple_us_mgr::lock);
        EXAM_CHECK( simple_us_mgr::d_cnt == 0 );
      }
      {
        lock_guard<mutex> lk(simple_us_mgr::lock);
        EXAM_CHECK( simple_us_mgr::c_cnt == 0 );
      }

      str.close();
    }

    unique_lock<mutex> lk( simple_us_mgr::lock );

    // EXAM_CHECK( simple_us_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr::c_cnt_check ) );
    EXAM_CHECK( simple_us_mgr::d_cnt == 0 );

    s.close();
  }

  unlink( f );

  unique_lock<mutex> lk( simple_us_mgr::lock );
  simple_us_mgr::d_cnt = 0;
  simple_us_mgr::c_cnt = 0;
  simple_us_mgr::n_cnt = 0;
  simple_us_mgr::buf[0] = 0;

#if 0
  {
    misc::open_syslog();
    int i = 20;
    misc::use_syslog<LOG_INFO,LOG_USER>() << "hello " << i << ", ok" << endl;
    misc::close_syslog();
  }
#endif

  return EXAM_RESULT;
}

int EXAM_IMPL(unix_sockios_test::core_write_test)
{
  const char* f = "/tmp/sock_test";

  {
    simple_us_mgr s( f );

    EXAM_CHECK( s.is_open() );
    EXAM_CHECK( s.good() );

    {
      sockstream str( f, sock_base::sock_dgram );

      EXAM_CHECK( str.is_open() );
      EXAM_CHECK( str.good() );

      str << "Hello, world!" << endl;

      {
        unique_lock<mutex> lk( simple_us_mgr::lock );

        EXAM_CHECK( simple_us_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr::n_cnt_check ) );
        EXAM_CHECK( string(simple_us_mgr::buf) == "Hello, world!\n" );
      }
      {
        lock_guard<mutex> lk(simple_us_mgr::lock);
        EXAM_CHECK( simple_us_mgr::d_cnt == 0 );
      }
      {
        lock_guard<mutex> lk(simple_us_mgr::lock);
        EXAM_CHECK( simple_us_mgr::c_cnt == 0 );
      }

      str.close();
    }

    unique_lock<mutex> lk( simple_us_mgr::lock );

    // EXAM_CHECK( simple_us_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr::c_cnt_check ) );
    EXAM_CHECK( simple_us_mgr::d_cnt == 0 );

    s.close();
  }

  unlink( f );

  unique_lock<mutex> lk( simple_us_mgr::lock );
  simple_us_mgr::d_cnt = 0;
  simple_us_mgr::c_cnt = 0;
  simple_us_mgr::n_cnt = 0;
  simple_us_mgr::buf[0] = 0;

  return EXAM_RESULT;
}

int EXAM_IMPL(unix_sockios_test::stream_core_test)
{
  const char* f = "/tmp/sock_test";

  {
    simple_us_mgr s( f, sock_base::sock_stream );

    EXAM_CHECK( s.is_open() );
    EXAM_CHECK( s.good() );

    {
      sockstream str( f, sock_base::sock_stream );

      EXAM_CHECK( str.is_open() );
      EXAM_CHECK( str.good() );

      {
        unique_lock<mutex> lk( simple_us_mgr::lock );

        EXAM_CHECK( simple_us_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr::n_cnt_check ) );
      }
      {
        lock_guard<mutex> lk(simple_us_mgr::lock);
        EXAM_CHECK( simple_us_mgr::d_cnt == 0 );
      }
      {
        lock_guard<mutex> lk(simple_us_mgr::lock);
        EXAM_CHECK( simple_us_mgr::c_cnt == 0 );
      }

      str.close();
    }

    unique_lock<mutex> lk( simple_us_mgr::lock );

    EXAM_CHECK( simple_us_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr::c_cnt_check ) );
    EXAM_CHECK( simple_us_mgr::d_cnt == 0 );

    s.close();
  }

  unlink( f );

  unique_lock<mutex> lk( simple_us_mgr::lock );
  simple_us_mgr::d_cnt = 0;
  simple_us_mgr::c_cnt = 0;
  simple_us_mgr::n_cnt = 0;
  simple_us_mgr::buf[0] = 0;

  return EXAM_RESULT;
}

class simple_us_mgr2 :
    public sock_basic_processor
{
  public:
    simple_us_mgr2() :
        sock_basic_processor()
      { }

    simple_us_mgr2( const char* path, sock_base::stype t = sock_base::sock_stream ) :
        sock_basic_processor( path, t )
      { }

    ~simple_us_mgr2()
      { }

  protected:
    virtual sock_basic_processor::sockbuf_t* operator ()( sock_base::socket_type fd, const sockaddr& addr )
      {
        lock_guard<mutex> lk(lock);
        ++n_cnt;
        sockstream_t* s = sock_basic_processor::create_stream( fd, addr );

        cons[fd] = s;

        cnd.notify_one();
        return s->rdbuf();
      }
    virtual void operator ()( sock_base::socket_type fd, const adopt_close_t& )
      {
        lock_guard<mutex> lk(lock);
        ++c_cnt;
        cnd.notify_one();
        delete cons[fd];
        cons.erase( fd );
      }
    virtual void operator ()( sock_base::socket_type )
      { lock_guard<mutex> lk(lock); ++d_cnt; cnd.notify_one(); }

  public:
    static mutex lock;
    static int n_cnt;
    static int c_cnt;
    static int d_cnt;
    static condition_variable cnd;

    static bool n_cnt_check()
      { return n_cnt == 1; }
    static bool c_cnt_check()
      { return c_cnt == 1; }
    static bool d_cnt_check()
      { return d_cnt == 1; }

  private:
#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<sock_base::socket_type,sockstream_t*> fd_container_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<sock_base::socket_type, sockstream_t*> fd_container_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<sock_base::socket_type, sockstream_t*> fd_container_type;
#endif

    fd_container_type cons;
};

mutex simple_us_mgr2::lock;
int simple_us_mgr2::n_cnt = 0;
int simple_us_mgr2::c_cnt = 0;
int simple_us_mgr2::d_cnt = 0;
condition_variable simple_us_mgr2::cnd;

int EXAM_IMPL(unix_sockios_test::stream_core_write_test)
{
  const char* f = "/tmp/sock_test";

  {
    simple_us_mgr2 s( f, sock_base::sock_stream );

    EXAM_CHECK( s.is_open() );
    EXAM_CHECK( s.good() );

    {
      sockstream str( f, sock_base::sock_stream );

      EXAM_CHECK( str.is_open() );
      EXAM_CHECK( str.good() );

      {
        unique_lock<mutex> lk( simple_us_mgr2::lock );

        EXAM_CHECK( simple_us_mgr2::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr2::n_cnt_check ) );
      }

      str << "Hello, world!" << endl;

      EXAM_CHECK( str.good() );

      {
        unique_lock<mutex> lk( simple_us_mgr2::lock );

        EXAM_CHECK( simple_us_mgr2::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr2::d_cnt_check ) );
      }
      str.close();
      {
        unique_lock<mutex> lk( simple_us_mgr2::lock );

        EXAM_CHECK( simple_us_mgr2::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr2::c_cnt_check ) );
      }
    }
  }

  unlink( f );

  unique_lock<mutex> lk( simple_us_mgr2::lock );
  simple_us_mgr2::d_cnt = 0;
  simple_us_mgr2::c_cnt = 0;
  simple_us_mgr2::n_cnt = 0;

  return EXAM_RESULT;
}

class worker_us
{
  public:
    worker_us( sockstream& )
      { lock_guard<mutex> lk(lock); ++cnt; ++visits; cnd.notify_one(); }

    ~worker_us()
      { lock_guard<mutex> lk(lock); --cnt; cnd.notify_one(); }

    void connect( sockstream& s )
      {
        lock_guard<mutex> lk(lock);
        getline( s, line );
        // cerr << __FILE__ << ":" << __LINE__ << " " << s.good() << " "
        //      << s.rdbuf()->in_avail() << endl;
        ++rd;
        line_cnd.notify_one();
      }

//     void close()
//      { }

    static int get_visits()
      { lock_guard<mutex> lk(lock); return visits; }

    static mutex lock;
    static int cnt;
    static /* volatile */ int visits;
    static condition_variable cnd;
    static string line;
    static condition_variable line_cnd;
    static int rd;

    static bool visits_counter1()
      { return worker_us::visits == 1; }

    static bool visits_counter2()
      { return worker_us::visits == 2; }

    static bool rd_counter1()
      { return worker_us::rd == 1; }

    static bool counter0()
      { return worker_us::cnt == 0; }
};

mutex worker_us::lock;
int worker_us::cnt = 0;
int worker_us::visits = 0;
condition_variable worker_us::cnd;
string worker_us::line;
condition_variable worker_us::line_cnd;
int worker_us::rd = 0;

int EXAM_IMPL(unix_sockios_test::processor_core_one_local)
{
  const char f[] = "/tmp/sockios_test";

  {
    connect_processor<worker_us> prss( f );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    {
      sockstream s( f, sock_base::sock_stream );

      EXAM_CHECK( s.good() );
      EXAM_CHECK( s.is_open() );

      unique_lock<mutex> lk( worker_us::lock );

      // worker's ctor visited once:
      EXAM_CHECK( worker_us::cnd.timed_wait( lk, milliseconds( 500 ), worker_us::visits_counter1 ) );      
      worker_us::visits = 0;
    }

    unique_lock<mutex> lksrv( worker_us::lock );
    // worker's dtor pass, no worker's objects left
    EXAM_CHECK( worker_us::cnd.timed_wait( lksrv, milliseconds( 500 ), worker_us::counter0 ) );
  }

  unlink( f );

  return EXAM_RESULT;
}

int EXAM_IMPL(unix_sockios_test::processor_core_two_local)
{
  const char f[] = "/tmp/sockios_test";

  {
    // check precondition
    lock_guard<mutex> lk( worker_us::lock );
    EXAM_CHECK( worker_us::cnt == 0 );
  }

  {
    connect_processor<worker_us> prss( f );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    {
      sockstream s1( f, sock_base::sock_stream );

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );

      sockstream s2( f, sock_base::sock_stream );

      EXAM_CHECK( s2.good() );
      EXAM_CHECK( s2.is_open() );

      unique_lock<mutex> lk( worker_us::lock );

      // two worker's ctors visited (two connects)
      EXAM_CHECK( worker_us::cnd.timed_wait( lk, milliseconds( 500 ), worker_us::visits_counter2 ) );
      worker_us::visits = 0;
    }

    unique_lock<mutex> lksrv( worker_us::lock );
    // both worker's dtors pass, no worker's objects left
    EXAM_CHECK( worker_us::cnd.timed_wait( lksrv, milliseconds( 500 ), worker_us::counter0 ) );
  }

  unlink( f );

  return EXAM_RESULT;
}

int EXAM_IMPL(unix_sockios_test::processor_core_getline)
{
  const char f[] = "/tmp/sockios_test";

  {
    // check precondition
    lock_guard<mutex> lk( worker_us::lock );
    EXAM_CHECK( worker_us::cnt == 0 );
  }

  {
    // check income data before sockstream was closed
    connect_processor<worker_us> prss( f );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    {
      sockstream s1( f, sock_base::sock_stream );

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );

      s1 << "Hello, world!" << endl;

      unique_lock<mutex> lk( worker_us::lock );
      EXAM_CHECK( worker_us::line_cnd.timed_wait( lk, milliseconds( 500 ), worker_us::rd_counter1 ) );

      // cerr << worker::line << endl;
      EXAM_CHECK( worker_us::line == "Hello, world!" );
      worker_us::line = "";
      worker_us::rd = 0;
    }

    unique_lock<mutex> lksrv( worker_us::lock );
    // worker's dtor pass, no worker's objects left
    EXAM_CHECK( worker_us::cnd.timed_wait( lksrv, milliseconds( 500 ), worker_us::counter0 ) );
  }

  unlink( f );

  return EXAM_RESULT;
}

int EXAM_IMPL(unix_sockios_test::processor_core_income_data)
{
  const char f[] = "/tmp/sockios_test";

  {
    // check after sockstream was closed, i.e. ensure, that all data available
    connect_processor<worker_us> prss( f );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    {
      sockstream s1( f, sock_base::sock_stream );

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );

      s1 << "Hello, world!" << endl;
    }

    {
      unique_lock<mutex> lk( worker_us::lock );
      EXAM_CHECK( worker_us::line_cnd.timed_wait( lk, milliseconds( 500 ), worker_us::rd_counter1 ) );
    }

    {
      unique_lock<mutex> lksrv( worker_us::lock );
      EXAM_CHECK( worker_us::cnd.timed_wait( lksrv, milliseconds( 500 ), worker_us::counter0 ) );
    }

    // EXAM_CHECK( worker_us::line == "Hello, world!" ); // <-- may fail
    worker_us::line = "";
    worker_us::rd = 0;
  }

  unlink( f );

  return EXAM_RESULT;
}

int EXAM_IMPL(unix_sockios_test::income_data)
{
  const char f[] = "/tmp/sockios_test";

  // worker::lock.lock();
  worker_us::visits = 0;
  worker_us::line = "";
  worker_us::rd = 0;
  // worker::lock.unlock();

  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;
    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

    try {

      EXAM_CHECK( worker_us::visits == 0 );

      this_thread::fork();

      int res = 0;
      {
        connect_processor<worker_us> prss( f );

        EXAM_CHECK_ASYNC_F( worker_us::visits == 0, res );

        b.wait(); // -- align here

        EXAM_CHECK_ASYNC_F( prss.good(), res );
        EXAM_CHECK_ASYNC_F( prss.is_open(), res );

        {
          unique_lock<mutex> lk( worker_us::lock );
          EXAM_CHECK_ASYNC_F( worker_us::line_cnd.timed_wait( lk, milliseconds( 500 ), worker_us::rd_counter1 ), res );
        }

        {
          unique_lock<mutex> lksrv( worker_us::lock );
          EXAM_CHECK_ASYNC_F( worker_us::cnd.timed_wait( lksrv, milliseconds( 500 ), worker_us::counter0 ), res );
        }

        EXAM_CHECK_ASYNC_F( worker_us::line == "Hello, world!", res ); // <-- may fail
      }

      unlink( f );

      exit( res );
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait(); // -- align here

      {
        sockstream s1( f, sock_base::sock_stream );

        EXAM_CHECK( s1.good() );
        EXAM_CHECK( s1.is_open() );

        s1 << "Hello, world!" << endl;
      }

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }

      EXAM_CHECK( worker_us::visits == 0 );
    }
    shm.deallocate( &b );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}
