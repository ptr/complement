// -*- C++ -*- Time-stamp: <08/03/06 23:31:13 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios2_test.h"
#include <exam/suite.h>

#include <sockios/sockstream2>
#include <mt/mutex>

using namespace std;
using namespace std::tr2;

sockios2_test::sockios2_test()
{
}

sockios2_test::~sockios2_test()
{
}

/* ************************************************************ */

class worker
{
  public:
    worker( sockstream& )
      { lock_guard<mutex> lk(lock); ++cnt; ++visits; }

    ~worker()
      { lock_guard<mutex> lk(lock); --cnt; }

    void connect( sockstream& )
      { }

    void close()
      { }

    static int get_visits()
      { lock_guard<mutex> lk(lock); return visits; }

    static mutex lock;
    static int cnt;
    static int visits;
};

mutex worker::lock;
int worker::cnt = 0;
int worker::visits = 0;

class simple_mgr :
    public sock_basic_processor
{
  public:
    simple_mgr() :
        sock_basic_processor()
      { }

    simple_mgr( int port, sock_base2::stype t = sock_base2::sock_stream ) :
        sock_basic_processor( port, t )
      { }

  protected:
    virtual void operator ()( sockstream_t& s, const adopt_new_t& )
      { lock_guard<mutex> lk(lock); b.wait(); ++n_cnt; }
    virtual void operator ()( sockstream_t& s, const adopt_close_t& )
      { lock_guard<mutex> lk(lock); b.wait(); ++c_cnt; }
    virtual void operator ()( sockstream_t& s, const adopt_data_t& )
      { lock_guard<mutex> lk(lock); ++d_cnt; }

  public:
    static mutex lock;
    static int n_cnt;
    static int c_cnt;
    static int d_cnt;
    static barrier b;
};

mutex simple_mgr::lock;
int simple_mgr::n_cnt = 0;
int simple_mgr::c_cnt = 0;
int simple_mgr::d_cnt = 0;
barrier simple_mgr::b;

class simple_mgr2 :
    public sock_basic_processor
{
  public:
    simple_mgr2() :
        sock_basic_processor()
      { }

    simple_mgr2( int port, sock_base2::stype t = sock_base2::sock_stream ) :
        sock_basic_processor( port, t )
      { }

  protected:
    virtual void operator ()( sockstream_t& s, const adopt_new_t& )
      { lock_guard<mutex> lk(lock); b.wait(); ++n_cnt; }
    virtual void operator ()( sockstream_t& s, const adopt_close_t& )
      { lock_guard<mutex> lk(lock); b.wait(); ++c_cnt; }
    virtual void operator ()( sockstream_t& s, const adopt_data_t& )
      {
        lock_guard<mutex> lk(lock);
        b.wait();
        ++d_cnt;
        string str;
        getline( s, str );
        EXAM_CHECK_ASYNC( str == "Hello" );
      }

  public:
    static mutex lock;
    static int n_cnt;
    static int c_cnt;
    static int d_cnt;
    static barrier b;
};

mutex simple_mgr2::lock;
int simple_mgr2::n_cnt = 0;
int simple_mgr2::c_cnt = 0;
int simple_mgr2::d_cnt = 0;
barrier simple_mgr2::b;


int EXAM_IMPL(sockios2_test::srv_core)
{
  simple_mgr srv( 2008 );

  EXAM_CHECK( srv.is_open() );
  EXAM_CHECK( srv.good() );

  srv.close();

  EXAM_CHECK( !srv.is_open() );

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios2_test::ctor_dtor)
{
  {
    simple_mgr srv( 2008 );

    EXAM_CHECK( srv.is_open() );
    EXAM_CHECK( srv.good() );

    {
      sockstream2 s( "localhost", 2008 );

      EXAM_CHECK( s.is_open() );
      EXAM_CHECK( s.good() );

      {
        simple_mgr::b.wait();
        lock_guard<mutex> lk(simple_mgr::lock);
        EXAM_CHECK( simple_mgr::n_cnt == 1 );
      }
      {
        lock_guard<mutex> lk(simple_mgr::lock);
        EXAM_CHECK( simple_mgr::d_cnt == 0 );
      }
      {
        lock_guard<mutex> lk(simple_mgr::lock);
        EXAM_CHECK( simple_mgr::c_cnt == 0 );
      }
    }
    
    simple_mgr::b.wait();
    lock_guard<mutex> lk(simple_mgr::lock);
    EXAM_CHECK( simple_mgr::c_cnt == 1 );
  }

  {
    simple_mgr2 srv( 2008 );

    EXAM_CHECK( srv.is_open() );
    EXAM_CHECK( srv.good() );
    {
      sockstream2 s( "localhost", 2008 );

      EXAM_CHECK( s.is_open() );
      EXAM_CHECK( s.good() );

      {
        simple_mgr2::b.wait();
        lock_guard<mutex> lk(simple_mgr2::lock);
        EXAM_CHECK( simple_mgr2::n_cnt == 1 );
      }
      {
        s << "Hello" << endl;
        EXAM_CHECK( s.good() );
        simple_mgr2::b.wait();
        lock_guard<mutex> lk(simple_mgr2::lock);
        EXAM_CHECK( simple_mgr2::d_cnt == 1 );
      }
      s.close();
      {
        simple_mgr2::b.wait();
        lock_guard<mutex> lk(simple_mgr2::lock);
        EXAM_CHECK( simple_mgr2::c_cnt == 1 );
      }
    }
  }

#if 0
  // Check, that number of ctors of Cnt is the same as number of called dtors
  // i.e. all created Cnt was released.
  {
    sockmgr_stream_MP<Cnt> srv( port );

    {
      sockstream s1( "localhost", port );

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );

      s1 << "1234" << endl;

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );
      while ( Cnt::get_visits() == 0 ) {
        xmt::Thread::yield();
      }
      Cnt::lock.lock();
      EXAM_CHECK( Cnt::cnt == 1 );
      Cnt::lock.unlock();
    }

    srv.close();
    srv.wait();

    Cnt::lock.lock();
    EXAM_CHECK( Cnt::cnt == 0 );
    Cnt::visits = 0;
    Cnt::lock.unlock();
  }

  Cnt::lock.lock();
  EXAM_CHECK( Cnt::cnt == 0 );
  Cnt::lock.unlock();

  {
    sockmgr_stream_MP<Cnt> srv( port );

    {
      sockstream s1( "localhost", port );
      sockstream s2( "localhost", port );

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );
      EXAM_CHECK( s2.good() );
      EXAM_CHECK( s2.is_open() );

      s1 << "1234" << endl;
      s2 << "1234" << endl;

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );
      EXAM_CHECK( s2.good() );
      EXAM_CHECK( s2.is_open() );
      while ( Cnt::get_visits() < 2 ) {
        xmt::Thread::yield();
      }
      Cnt::lock.lock();
      EXAM_CHECK( Cnt::cnt == 2 );
      Cnt::lock.unlock();
    }

    srv.close();
    srv.wait();

    Cnt::lock.lock();
    EXAM_CHECK( Cnt::cnt == 0 );
    Cnt::lock.unlock();
  }

  Cnt::lock.lock();
  EXAM_CHECK( Cnt::cnt == 0 );
  Cnt::lock.unlock();
#endif

  return EXAM_RESULT;
}
