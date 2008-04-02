// -*- C++ -*- Time-stamp: <08/04/02 01:48:59 ptr>

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
#include <sys/wait.h>
#include <mt/shm.h>

using namespace std;
using namespace std::tr2;

sockios2_test::sockios2_test()
{
}

sockios2_test::~sockios2_test()
{
}

/* ************************************************************ */

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

    ~simple_mgr()
      { cerr << "In destructor\n"; }

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

int EXAM_IMPL(sockios2_test::connect_disconnect)
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

  return EXAM_RESULT;
}

class worker
{
  public:
    worker( sockstream2& )
      { lock_guard<mutex> lk(lock); ++cnt; ++visits; cnd.notify_one(); }

    ~worker()
      { lock_guard<mutex> lk(lock); --cnt; }

    void connect( sockstream2& s )
      { lock_guard<mutex> lk(lock); getline( s, line ); ++rd; line_cnd.notify_one(); }

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
    // static barrier b;
};

mutex worker::lock;
int worker::cnt = 0;
/* volatile */ int worker::visits = 0;
condition_variable worker::cnd;
string worker::line;
condition_variable worker::line_cnd;
int worker::rd = 0;

// barrier worker::b;

// void stopper( connect_processor<worker>* prss )
// {
//   b.wait();
//   prss->close();
// }

bool visits_counter1()
{
  return worker::visits == 1;
}

bool visits_counter2()
{
  return worker::visits == 2;
}

bool rd_counter1()
{
  return worker::rd == 1;
}

int EXAM_IMPL(sockios2_test::processor_core)
{
  {
    connect_processor<worker> prss( 2008 );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    {
      sockstream2 s( "localhost", 2008 );

      EXAM_CHECK( s.good() );
      EXAM_CHECK( s.is_open() );

//      for ( int i = 0; i < 64; ++i ) { // give chance to process it
//        std::tr2::this_thread::yield();
//      }

      unique_lock<mutex> lk( worker::lock );

      worker::cnd.timed_wait( lk, milliseconds( 100 ), visits_counter1 );
      
      EXAM_CHECK( worker::visits == 1 );
      worker::visits = 0;
    }
  }
  {
    lock_guard<mutex> lk( worker::lock );
    EXAM_CHECK( worker::cnt == 0 );
  }

  {
    connect_processor<worker> prss( 2008 );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    {
      sockstream2 s1( "localhost", 2008 );

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );

      sockstream2 s2( "localhost", 2008 );

      EXAM_CHECK( s2.good() );
      EXAM_CHECK( s2.is_open() );

//      for ( int i = 0; i < 1024; ++i ) { // give chance to process it
//        std::tr2::this_thread::yield();
//      }
      unique_lock<mutex> lk( worker::lock );

      worker::cnd.timed_wait( lk, milliseconds( 500 ), visits_counter2 );

      EXAM_CHECK( worker::visits == 2 );
      worker::visits = 0;
    }
  }
  {
    lock_guard<mutex> lk( worker::lock );
    EXAM_CHECK( worker::cnt == 0 );
  }


  // check before sockstream2 was closed
  {
    connect_processor<worker> prss( 2008 );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    {
      sockstream2 s1( "localhost", 2008 );

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );

      s1 << "Hello, world!" << endl;

      unique_lock<mutex> lk( worker::lock );
      worker::cnd.timed_wait( lk, milliseconds( 100 ), rd_counter1 );

      // cerr << worker::line << endl;
      EXAM_CHECK( worker::line == "Hello, world!" );
      worker::line = "";
      worker::rd = 0;
    }
  }


  EXAM_CHECK( worker::line == "" );

  // check after sockstream2 was closed, i.e. ensure, that all data available read before close
  {
    connect_processor<worker> prss( 2008 );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    {
      sockstream2 s1( "localhost", 2008 );

      EXAM_CHECK( s1.good() );
      EXAM_CHECK( s1.is_open() );

      s1 << "Hello, world!" << endl;
    }

    unique_lock<mutex> lk( worker::lock );
    worker::cnd.timed_wait( lk, milliseconds( 100 ), rd_counter1 );

    // cerr << worker::line << endl;
    EXAM_CHECK( worker::line == "Hello, world!" );
    worker::line = "";
    worker::rd = 0;
  }

  EXAM_CHECK( worker::line == "" );

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios2_test::fork)
{
  const char fname[] = "/tmp/sockios2_test.shm";

  /* You must work very carefully with sockets, theads and fork: it unsafe in principle
     and no way to make it safe. Never try to pass _opened_ connection via fork.
     Here I create sockstream, but without connection (it check that io processing
     loop in underlying sockmgr finish and restart smoothly in child process).
   */
  sockstream2 s;

  // worker::lock.lock();
  worker::visits = 0;
  // worker::lock.unlock();

  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( fname, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;
    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

    try {

      EXAM_CHECK( worker::visits == 0 );

      this_thread::fork();
      {
        connect_processor<worker> prss( 2008 );

        EXAM_CHECK_ASYNC( worker::visits == 0 );

        b.wait(); // -- align here

        EXAM_CHECK_ASYNC( prss.good() );
        EXAM_CHECK_ASYNC( prss.is_open() );

        unique_lock<mutex> lk( worker::lock );
        worker::cnd.timed_wait( lk, milliseconds( 100 ), visits_counter1 );

        EXAM_CHECK_ASYNC( worker::visits == 1 );
      }

      exit( 0 );
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait(); // -- align here

      s.open( "localhost", 2008 );

      EXAM_CHECK( s.good() );
      EXAM_CHECK( s.is_open() );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }

      EXAM_CHECK( worker::visits == 0 );
    }
    shm.deallocate( &b );
    seg.deallocate();
    unlink( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

class stream_reader
{
  public:
    stream_reader( sockstream2& )
      { }

    ~stream_reader()
      { }

    void connect( sockstream2& s )
      {
        char buf[1024];

        s.read( buf, 1024 );
        s.write( buf, 1024 );
        s.flush();
      }

    static void load_generator( barrier* b )
    {
      sockstream2 s( "localhost", 2008 );

      char buf[1024];

      fill( buf, buf + 1024, 0 );

      b->wait();

      while( true ) {
        s.write( buf, 1024 );
        s.flush();

        s.read( buf, 1024 );
        this_thread::yield();
      }
    }

};

int EXAM_IMPL(sockios2_test::srv_sigpipe)
{
  const char fname[] = "/tmp/sockios2_test.shm";
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( fname, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;
    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();
    try {
      this_thread::fork();

      b.wait();
      /*
       * This process will be killed,
       * so I don't care about safe termination.
       */

      const int b_count = 10;
      barrier bb( b_count );

      thread* th1 = new thread( stream_reader::load_generator, &bb );

      for ( int i = 0; i < (b_count - 1); ++i ) {
        new thread( stream_reader::load_generator, &bb );
      }

      this_thread::sleep( milliseconds( 100 ) );

      b.wait();

      th1->join(); // Will be interrupted!

      exit( 0 );
    }
    catch ( std::tr2::fork_in_parent& child ) {
      connect_processor<stream_reader> r( 2008 );

      EXAM_CHECK( r.good() );
      EXAM_CHECK( r.is_open() );

      b.wait();
      b.wait();

      kill( child.pid(), SIGTERM );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        // EXAM_CHECK( WEXITSTATUS(stat) == 0 );
        EXAM_ERROR( "child should be interrupted" );
      } else {
        EXAM_MESSAGE( "child interrupted" );
      }

      EXAM_CHECK( r.good() );
      EXAM_CHECK( r.is_open() );

    }
    shm.deallocate( &b );
    seg.deallocate();
    unlink( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}


class interrupted_writer
{
  public:
    interrupted_writer( sockstream2& s )
      {
        EXAM_CHECK_ASYNC( s.good() );

        int n = 1;

        cerr << "align 3\n";
        bb->wait();  // <-- align 3

        cerr << "align 3 pass\n";
        s.write( (const char *)&n, sizeof( int ) ).flush();
        EXAM_CHECK_ASYNC( s.good() );
      }

    ~interrupted_writer()
      { cerr << "~~\n"; }

    void connect( sockstream2& s )
      { }

    static void read_generator( barrier* b )
    {
      sockstream2 s( "localhost", 2008 );

      int buff = 0;
      cerr << "align 2" << endl;
      b->wait(); // <-- align 2
      cerr << "align pass" << endl;

      EXAM_CHECK_ASYNC( s.read( (char *)&buff, sizeof(int) ).good() ); // <---- key line
      EXAM_CHECK_ASYNC( buff == 1 );
    }

    static barrier_ip* bb;
};

barrier_ip* interrupted_writer::bb = 0;

int EXAM_IMPL(sockios2_test::read0)
{
  const char fname[] = "/tmp/sockios2_test.shm";
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( fname, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;
    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();
    barrier_ip& bnew = *new ( shm.allocate( 1 ) ) barrier_ip();
    interrupted_writer::bb = &bnew;

    try {
      this_thread::fork();

      b.wait();  // <-- align 1

      barrier bb;

      thread t( interrupted_writer::read_generator, &bb );

      bb.wait();  // <-- align 2

      cerr << "system" << endl;
      system( "echo > /dev/null" );  // <------ key line
      cerr << "after system" << endl;

      bnew.wait();  // <-- align 3
      cerr << "after align 3" << endl;

      t.join();

      exit( 0 );
    }
    catch ( std::tr2::fork_in_parent& child ) {
      connect_processor<interrupted_writer> r( 2008 );

      EXAM_CHECK( r.good() );
      EXAM_CHECK( r.is_open() );

      b.wait();  // <-- align 1

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }

      EXAM_CHECK( r.good() );
      EXAM_CHECK( r.is_open() );
    }
    shm.deallocate( &bnew );
    shm.deallocate( &b );
    seg.deallocate();
    unlink( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}
