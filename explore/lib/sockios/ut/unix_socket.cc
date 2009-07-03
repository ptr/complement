// -*- C++ -*- Time-stamp: <09/07/03 15:16:30 ptr>

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
