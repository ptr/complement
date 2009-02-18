// -*- C++ -*- Time-stamp: <09/02/18 15:29:15 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_test.h"
#include <exam/suite.h>

#include <sockios/sockstream>
#include <mt/mutex>
#include <sys/wait.h>
#include <mt/shm.h>


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

sockios_test::sockios_test()
{
}

sockios_test::~sockios_test()
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

    simple_mgr( int port, sock_base::stype t = sock_base::sock_stream ) :
        sock_basic_processor( port, t )
      { }

    ~simple_mgr()
      { /* cerr << "In destructor\n"; */ }

  protected:
    virtual sock_basic_processor::sockbuf_t* operator ()( sock_base::socket_type, const sockaddr& )
      { lock_guard<mutex> lk(lock); ++n_cnt; cnd.notify_one(); return 0; }
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

    static bool n_cnt_check()
      { return n_cnt == 1; }
    static bool c_cnt_check()
      { return c_cnt == 1; }
    static bool d_cnt_check()
      { return d_cnt == 1; }
};

mutex simple_mgr::lock;
int simple_mgr::n_cnt = 0;
int simple_mgr::c_cnt = 0;
int simple_mgr::d_cnt = 0;
condition_variable simple_mgr::cnd;

class simple_mgr2 :
    public sock_basic_processor
{
  public:
    simple_mgr2() :
        sock_basic_processor()
      { }

    simple_mgr2( int port, sock_base::stype t = sock_base::sock_stream ) :
        sock_basic_processor( port, t )
      { }

  protected:
    virtual sock_basic_processor::sockbuf_t* operator ()( sock_base::socket_type fd, const sockaddr& addr )
      { 
        lock_guard<mutex> lk(lock);
        ++n_cnt;
        cnd.notify_one();
        sockstream_t* s = sock_basic_processor::create_stream( fd, addr );

        cons[fd] = s;

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

    virtual void operator ()( sock_base::socket_type fd )
      {
        lock_guard<mutex> lk(lock);
        ++d_cnt;
        cnd.notify_one();
        string str;
        getline( *cons[fd], str ); // map fd -> s
        EXAM_CHECK_ASYNC( str == "Hello" );
      }

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

mutex simple_mgr2::lock;
int simple_mgr2::n_cnt = 0;
int simple_mgr2::c_cnt = 0;
int simple_mgr2::d_cnt = 0;
condition_variable simple_mgr2::cnd;


int EXAM_IMPL(sockios_test::srv_core)
{
  simple_mgr srv( 2008 );

  EXAM_CHECK( srv.is_open() );
  EXAM_CHECK( srv.good() );

  srv.close();

  EXAM_CHECK( !srv.is_open() );

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_test::connect_disconnect)
{
  {
    simple_mgr srv( 2008 );

    EXAM_CHECK( srv.is_open() );
    EXAM_CHECK( srv.good() );

    {
      sockstream s( "localhost", 2008 );

      EXAM_CHECK( s.is_open() );
      EXAM_CHECK( s.good() );

      {
        unique_lock<mutex> lk( simple_mgr::lock );

        EXAM_CHECK( simple_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_mgr::n_cnt_check ) );
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
    
    unique_lock<mutex> lk( simple_mgr::lock );

    EXAM_CHECK( simple_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_mgr::c_cnt_check ) );
    EXAM_CHECK( simple_mgr::d_cnt == 0 );
  }

  {
    simple_mgr2 srv( 2008 );

    EXAM_CHECK( srv.is_open() );
    EXAM_CHECK( srv.good() );
    {
      sockstream s( "localhost", 2008 );

      EXAM_CHECK( s.is_open() );
      EXAM_CHECK( s.good() );

      {
        unique_lock<mutex> lk( simple_mgr2::lock );
        EXAM_CHECK( simple_mgr2::cnd.timed_wait( lk, milliseconds( 500 ), simple_mgr2::n_cnt_check ) );
      }
      {
        s << "Hello" << endl;
        EXAM_CHECK( s.good() );
        unique_lock<mutex> lk( simple_mgr2::lock );
        EXAM_CHECK( simple_mgr2::cnd.timed_wait( lk, milliseconds( 500 ), simple_mgr2::d_cnt_check ) );
      }
      s.close();
      {
        unique_lock<mutex> lk( simple_mgr2::lock );
        EXAM_CHECK( simple_mgr2::cnd.timed_wait( lk, milliseconds( 500 ), simple_mgr2::c_cnt_check ) );
      }
    }
  }

  return EXAM_RESULT;
}

class worker
{
  public:
    worker( sockstream& )
      { lock_guard<mutex> lk(lock); ++cnt; ++visits; cnd.notify_one(); }

    ~worker()
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
      { return worker::visits == 1; }

    static bool visits_counter2()
      { return worker::visits == 2; }

    static bool rd_counter1()
      { return worker::rd == 1; }

    static bool counter0()
      { return worker::cnt == 0; }
};

mutex worker::lock;
int worker::cnt = 0;
int worker::visits = 0;
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

int EXAM_IMPL(sockios_test::processor_core_one_local)
{
  connect_processor<worker> prss( 2008 );

  EXAM_CHECK( prss.good() );
  EXAM_CHECK( prss.is_open() );

  {
    sockstream s( "localhost", 2008 );

    EXAM_CHECK( s.good() );
    EXAM_CHECK( s.is_open() );

//      for ( int i = 0; i < 64; ++i ) { // give chance to process it
//        std::tr2::this_thread::yield();
//      }

    unique_lock<mutex> lk( worker::lock );

    // worker's ctor visited once:
    EXAM_CHECK( worker::cnd.timed_wait( lk, milliseconds( 500 ), worker::visits_counter1 ) );      
    worker::visits = 0;
  }

  // for ( int i = 0; i < 64; ++i ) { // give chance for system
  //   std::tr2::this_thread::yield();
  // }

  unique_lock<mutex> lksrv( worker::lock );
  // worker's dtor pass, no worker's objects left
  EXAM_CHECK( worker::cnd.timed_wait( lksrv, milliseconds( 500 ), worker::counter0 ) );

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_test::processor_core_two_local)
{
  {
    // check precondition
    lock_guard<mutex> lk( worker::lock );
    EXAM_CHECK( worker::cnt == 0 );
  }

  connect_processor<worker> prss( 2008 );

  EXAM_CHECK( prss.good() );
  EXAM_CHECK( prss.is_open() );

  {
    sockstream s1( "localhost", 2008 );

    EXAM_CHECK( s1.good() );
    EXAM_CHECK( s1.is_open() );

    sockstream s2( "localhost", 2008 );

    EXAM_CHECK( s2.good() );
    EXAM_CHECK( s2.is_open() );

//      for ( int i = 0; i < 1024; ++i ) { // give chance to process it
//        std::tr2::this_thread::yield();
//      }

    unique_lock<mutex> lk( worker::lock );

    // two worker's ctors visited (two connects)
    EXAM_CHECK( worker::cnd.timed_wait( lk, milliseconds( 500 ), worker::visits_counter2 ) );
    worker::visits = 0;
  }

  // for ( int i = 0; i < 64; ++i ) { // give chance for system
  //   std::tr2::this_thread::yield();
  // }
  unique_lock<mutex> lksrv( worker::lock );
  // both worker's dtors pass, no worker's objects left
  EXAM_CHECK( worker::cnd.timed_wait( lksrv, milliseconds( 500 ), worker::counter0 ) );

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_test::processor_core_getline)
{
  {
    // check precondition
    lock_guard<mutex> lk( worker::lock );
    EXAM_CHECK( worker::cnt == 0 );
  }

  // check income data before sockstream was closed
  connect_processor<worker> prss( 2008 );

  EXAM_CHECK( prss.good() );
  EXAM_CHECK( prss.is_open() );

  {
    sockstream s1( "localhost", 2008 );

    EXAM_CHECK( s1.good() );
    EXAM_CHECK( s1.is_open() );

    s1 << "Hello, world!" << endl;

    unique_lock<mutex> lk( worker::lock );
    EXAM_CHECK( worker::line_cnd.timed_wait( lk, milliseconds( 500 ), worker::rd_counter1 ) );

    // cerr << worker::line << endl;
    EXAM_CHECK( worker::line == "Hello, world!" );
    worker::line = "";
    worker::rd = 0;
  }

  // for ( int i = 0; i < 64; ++i ) { // give chance for system
  //   std::tr2::this_thread::yield();
  // }

  unique_lock<mutex> lksrv( worker::lock );
  // worker's dtor pass, no worker's objects left
  EXAM_CHECK( worker::cnd.timed_wait( lksrv, milliseconds( 500 ), worker::counter0 ) );

  return EXAM_RESULT;
}


/*
 * If server and client both in the same process (sure?), server can
 * see signal about close client socket _before_ it receive the rest of data
 * (tcp stack implemenation dependent?).
 * So sockios2_test::processor_core_income_data may not fill line
 * with "Hello, world!".
 * 
 * But if server and client situated in different processes, server will see
 * FYN _after_ it read all data.
 *
 * Tests processor_core_income_data and income_data (below) are the same,
 * except first has client and server in the same process, while second
 * has server in child process. See commented line:
 *   // EXAM_CHECK( worker::line == "Hello, world!" ); // <-- may fail
 * in the first test.
 *
 * Good text above. But wrong.
 */

int EXAM_IMPL(sockios_test::processor_core_income_data)
{
  // check after sockstream was closed, i.e. ensure, that all data available
  connect_processor<worker> prss( 2008 );

  EXAM_CHECK( prss.good() );
  EXAM_CHECK( prss.is_open() );

  {
    sockstream s1( "localhost", 2008 );

    EXAM_CHECK( s1.good() );
    EXAM_CHECK( s1.is_open() );

    s1 << "Hello, world!" << endl;
  }

  {
    unique_lock<mutex> lk( worker::lock );
    EXAM_CHECK( worker::line_cnd.timed_wait( lk, milliseconds( 500 ), worker::rd_counter1 ) );
  }

  {
    unique_lock<mutex> lksrv( worker::lock );
    EXAM_CHECK( worker::cnd.timed_wait( lksrv, milliseconds( 500 ), worker::counter0 ) );
  }

  // EXAM_CHECK( worker::line == "Hello, world!" ); // <-- may fail
  worker::line = "";
  worker::rd = 0;

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_test::income_data)
{
  // worker::lock.lock();
  worker::visits = 0;
  worker::line = "";
  worker::rd = 0;
  // worker::lock.unlock();

  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;
    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

    try {

      EXAM_CHECK( worker::visits == 0 );

      this_thread::fork();

      int res = 0;
      {
        connect_processor<worker> prss( 2008 );

        EXAM_CHECK_ASYNC_F( worker::visits == 0, res );

        b.wait(); // -- align here

        EXAM_CHECK_ASYNC_F( prss.good(), res );
        EXAM_CHECK_ASYNC_F( prss.is_open(), res );

        {
          unique_lock<mutex> lk( worker::lock );
          EXAM_CHECK_ASYNC_F( worker::line_cnd.timed_wait( lk, milliseconds( 500 ), worker::rd_counter1 ), res );
        }

        {
          unique_lock<mutex> lksrv( worker::lock );
          EXAM_CHECK_ASYNC_F( worker::cnd.timed_wait( lksrv, milliseconds( 500 ), worker::counter0 ), res );
        }

        EXAM_CHECK_ASYNC_F( worker::line == "Hello, world!", res ); // <-- may fail
      }

      exit( res );
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait(); // -- align here

      {
        sockstream s1( "localhost", 2008 );

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

      EXAM_CHECK( worker::visits == 0 );
    }
    shm.deallocate( &b );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

class srv_reader
{
  public:
    srv_reader( sockstream& )
      { }
    ~srv_reader()
      { }
    void connect( sockstream& s )
      {
        char buf[64];

        while ( s.read( buf, 4 ).good() ) {
          continue;
        }

        cnd.notify_one();
      }

    static std::tr2::condition_event cnd;
};

std::tr2::condition_event srv_reader::cnd;

int EXAM_IMPL(sockios_test::disconnect_rawclnt)
{
  // throw exam::skip_exception();
  xmt::shm_alloc<0> seg;

  try {
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
    try {
      seg.allocate( 70000, 4096, 0, 0660 );
    }
    catch ( xmt::shm_bad_alloc& err2 ) {
      EXAM_ERROR( err.what() );
      return EXAM_RESULT;
    }
  }

  xmt::allocator_shm<barrier_ip,0> shm;
  barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

  try {
    this_thread::fork();

    int res = 0;

    {
      connect_processor<srv_reader> prss( 2008 );

      EXAM_CHECK_ASYNC_F( prss.good(), res );

      b.wait();

      EXAM_CHECK_ASYNC_F( srv_reader::cnd.timed_wait( milliseconds( 800 ) ), res );
      // srv_reader::cnd.wait();
    }

    exit( res );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    b.wait();

    char buf[] = "1234";

    int fd = socket( PF_INET, SOCK_STREAM, 0 );

    EXAM_CHECK( fd != 0 );
    
    union sockaddr_t {
        sockaddr_in inet;
        sockaddr    any;
    } address;

    int port = 2008;

    address.inet.sin_family = AF_INET;
    address.inet.sin_port = ((((port) >> 8) & 0xff) | (((port) & 0xff) << 8));
    address.inet.sin_addr = std::findhost( "localhost" );

    EXAM_CHECK( connect( fd, &address.any, sizeof( address ) ) != -1 );

    EXAM_CHECK( ::write( fd, buf, 4 ) == 4 );

    ::close( fd );

    int stat = -1;
    EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    if ( WIFEXITED(stat) ) {
      EXAM_CHECK( WEXITSTATUS(stat) == 0 );
    } else {
      EXAM_ERROR( "child fail" );
    }
  }

  shm.deallocate( &b );
  seg.deallocate();

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_test::disconnect)
{
  // throw exam::skip_exception();

  xmt::shm_alloc<0> seg;

  try {
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
    try {
      seg.allocate( 70000, 4096, 0, 0660 );
    }
    catch ( xmt::shm_bad_alloc& err2 ) {
      EXAM_ERROR( err.what() );
      return EXAM_RESULT;
    }
  }

  xmt::allocator_shm<barrier_ip,0> shm;
  barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

  try {
    this_thread::fork();

    int res = 0;

    {
      connect_processor<srv_reader> prss( 2008 );

      EXAM_CHECK_ASYNC_F( prss.good(), res );

      b.wait();

      EXAM_CHECK_ASYNC_F( srv_reader::cnd.timed_wait( milliseconds( 800 ) ), res );
      // srv_reader::cnd.wait();
    }

    exit( res );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    b.wait();

    sockstream s( "localhost", 2008 );

    char buf[] = "1234";

    EXAM_CHECK( s.write( buf, 4 ).flush().good() );

    s.rdbuf()->shutdown( /* sock_base::stop_in | */ sock_base::stop_out );

    s.close(); // should work with this line commented, but sorry

    int stat = -1;
    EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    if ( WIFEXITED(stat) ) {
      EXAM_CHECK( WEXITSTATUS(stat) == 0 );
    } else {
      EXAM_ERROR( "child fail" );
    }
  }

  shm.deallocate( &b );
  seg.deallocate();

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_test::fork)
{
  /* You must work very carefully with sockets, theads and fork: it unsafe in principle
     and no way to make it safe. Never try to pass _opened_ connection via fork.
     Here I create sockstream, but without connection (it check that io processing
     loop in underlying sockmgr finish and restart smoothly in child process).
   */
  sockstream s;

  // worker::lock.lock();
  worker::visits = 0;
  // worker::lock.unlock();

  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;
    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

    try {

      EXAM_CHECK( worker::visits == 0 );

      this_thread::fork();

      int res = 0;
      {
        connect_processor<worker> prss( 2008 );

        EXAM_CHECK_ASYNC_F( worker::visits == 0, res );

        b.wait(); // -- align here

        EXAM_CHECK_ASYNC_F( prss.good(), res );
        EXAM_CHECK_ASYNC_F( prss.is_open(), res );

        {
          unique_lock<mutex> lk( worker::lock );
          EXAM_CHECK_ASYNC_F( worker::cnd.timed_wait( lk, milliseconds( 500 ), worker::visits_counter1 ) , res );
        }

        // for ( int i = 0; i < 64; ++i ) { // give chance for system
        //   std::tr2::this_thread::yield();
        // }

        {
          unique_lock<mutex> lksrv( worker::lock );
          EXAM_CHECK_ASYNC_F( worker::cnd.timed_wait( lksrv, milliseconds( 500 ), worker::counter0 ), res );
        }
      }

      exit( res );
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait(); // -- align here

      s.open( "localhost", 2008 );

      EXAM_CHECK( s.good() );
      EXAM_CHECK( s.is_open() );

      s.close();

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
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

class stream_reader
{
  public:
    stream_reader( sockstream& s )
      {
        try {
          // s.rdbuf()->setoptions( sock_base::so_keepalive, true );
          // s.rdbuf()->setoptions( sock_base::so_tcp_keepidle, 60 );
          // s.rdbuf()->setoptions( sock_base::so_tcp_keepintvl, 1 );
          // s.rdbuf()->setoptions( sock_base::so_tcp_keepcnt, 3 );
        }
        catch ( ... ) {
          EXAM_ERROR_ASYNC( "bad option" );
        }
      }

    ~stream_reader()
      { }

    void connect( sockstream& s )
      {
        char buf[1024];

        s.read( buf, 1024 );
        s.write( buf, 1024 );
        s.flush();
      }

    static void load_generator( barrier* b )
    {
      sockstream s( "localhost", 2008 );

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

int EXAM_IMPL(sockios_test::srv_sigpipe)
{
  // throw exam::skip_exception();

  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

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

      // for ( int i = 0; i < 64; ++i ) { // give chance for system
      //   std::tr2::this_thread::yield();
      // }
    }
    shm.deallocate( &b );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}


class interrupted_writer
{
  public:
    interrupted_writer( sockstream& s )
      {
        EXAM_CHECK_ASYNC( s.good() );

        int n = 1;

        // cerr << "align 3\n";
        bb->wait();  // <-- align 3

        // cerr << "align 3 pass\n";
        s.write( (const char *)&n, sizeof( int ) ).flush();
        EXAM_CHECK_ASYNC( s.good() );
      }

    ~interrupted_writer()
      { /* cerr << "~~\n"; */ }

    void connect( sockstream& s )
      { }

    static void read_generator( barrier* b )
    {      
      sockstream s( "localhost", 2008 );

      int buff = 0;
      // cerr << "align 2" << endl;

      EXAM_CHECK_ASYNC( s.good() );
      EXAM_CHECK_ASYNC( s.is_open() );

      b->wait(); // <-- align 2
      // cerr << "align 2 pass" << endl;

      EXAM_CHECK_ASYNC( s.read( (char *)&buff, sizeof(int) ).good() ); // <---- key line
      // cerr << "read pass" << endl;
      EXAM_CHECK_ASYNC( buff == 1 );
    }

    static barrier_ip* bb;
};

barrier_ip* interrupted_writer::bb = 0;

int EXAM_IMPL(sockios_test::read0)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

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

      // cerr << "system" << endl;
      system( "echo > /dev/null" );  // <------ key line
      // cerr << "after system" << endl;

      bnew.wait();  // <-- align 3
      // cerr << "after align 3" << endl;

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

      // for ( int i = 0; i < 64; ++i ) { // give chance for system
      //   std::tr2::this_thread::yield();
      // }
    }
    shm.deallocate( &bnew );
    shm.deallocate( &b );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

class byte_cnt
{
  public:
    static const int sz;
    static const int bsz;
    static int r;

    byte_cnt( sockstream& s )
      {
        EXAM_CHECK_ASYNC( s.good() );
      }

    ~byte_cnt()
      { }

    void connect( sockstream& s )
      {
        char buf[bsz];

        fill( buf, buf + sizeof(buf), 0x0 );

        s.read( buf, sizeof(buf) );

        EXAM_CHECK_ASYNC( /* s.good() */ !s.fail() );
        // static int j = 0;
        // ++j;
        // for ( int i = 0; i < bsz; ++i ) {
        //   if ( buf[i] == 0 ) {
        //     cerr << "***** " << i << ' ' << j << endl;
        //   }
        // }
        // if ( s.fail() ) {
        //   cerr << count( buf, buf + sizeof(buf), 1 ) << endl;
        //   xmt::callstack( cerr );
        // }

        lock_guard<mutex> lk( lock );
        r += count( buf, buf + sizeof(buf), 1 );
        if ( r == sz * bsz ) {
          EXAM_MESSAGE_ASYNC( "Looks all here" );
          cnd.notify_one();
        }
      }

    static mutex lock;
    static condition_variable cnd;
};

const int byte_cnt::sz = 64 * 5;
const int byte_cnt::bsz = 1024;  // bsz * sz > MTU of lo
int byte_cnt::r = 0;
mutex  byte_cnt::lock;
condition_variable byte_cnt::cnd;

struct byte_cnt_predicate
{
    bool operator ()()
      { return byte_cnt::r == (byte_cnt::sz * byte_cnt::bsz); }
};

int EXAM_IMPL(sockios_test::few_packets)
{
  char buf[byte_cnt::bsz * byte_cnt::sz];

  fill( buf, buf + sizeof(buf), 0x1 );

  {
    connect_processor<byte_cnt> prss( 2008 );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    sockstream s( "localhost", 2008 );

    EXAM_CHECK( s.is_open() );
    EXAM_CHECK( s.good() );

    s.write( buf, sizeof(buf) );

    EXAM_CHECK( s.good() );

    s.flush();

    unique_lock<mutex> lk( byte_cnt::lock );
    EXAM_CHECK( byte_cnt::cnd.timed_wait( lk, milliseconds(500), byte_cnt_predicate() ) );
  }

  EXAM_CHECK( byte_cnt::r == (byte_cnt::bsz * byte_cnt::sz) );
  
  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_test::few_packets_loop)
{
  char buf[byte_cnt::bsz];

  fill( buf, buf + sizeof(buf), 0x1 );

  byte_cnt::r = 0;

  {
    connect_processor<byte_cnt> prss( 2008 );

    EXAM_CHECK( prss.good() );
    EXAM_CHECK( prss.is_open() );

    sockstream s( "localhost", 2008 );

    EXAM_CHECK( s.is_open() );
    EXAM_CHECK( s.good() );

    for ( int i = 0; (i < byte_cnt::sz) && s.good(); ++i ) {
      s.write( buf, sizeof(buf) );
    }

    EXAM_CHECK( s.good() );

    s.flush();

    unique_lock<mutex> lk( byte_cnt::lock );
    EXAM_CHECK( byte_cnt::cnd.timed_wait( lk, milliseconds(500), byte_cnt_predicate() ) );
  }

  EXAM_CHECK( byte_cnt::r == (byte_cnt::bsz * byte_cnt::sz) );
  
  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_test::two_ports)
{
  {
    connect_processor<worker> prss1( 2008 );
    connect_processor<worker> prss2( 2009 );

    EXAM_CHECK( prss1.good() );
    EXAM_CHECK( prss2.good() );

    prss1.close();
    prss1.wait();
    prss2.close();
    prss2.wait();
  }
  
  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_test::service_stop)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;
    // xmt::allocator_shm<mutex_ip,0> mtx;
    // xmt::allocator_shm<condition_event_ip,0> cnd;
    // xmt::allocator_shm<int,0> ishm;

    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();
    barrier_ip& b2 = *new ( shm.allocate( 1 ) ) barrier_ip();
    try {
      this_thread::fork();

      std::tr2::this_thread::block_signal( SIGINT );

      int ret = 0;

      try {
        connect_processor<worker> prss( 2008 );

        EXAM_CHECK_ASYNC_F( prss.good(), ret );
        EXAM_CHECK_ASYNC_F( prss.is_open(), ret );

        b.wait();

        sigset_t signal_mask;

        sigemptyset( &signal_mask );
        sigaddset( &signal_mask, SIGINT );

        b2.wait();

        int sig_caught;
        sigwait( &signal_mask, &sig_caught );

        if ( sig_caught == SIGINT ) {
          EXAM_MESSAGE_ASYNC( "catch INT signal" );
          prss.close();
        } else {
          EXAM_ERROR_ASYNC_F( "catch of INT signal expected", ret );
        }

        prss.wait();
      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unexpected exception", ret );
      }

      exit(ret);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait();

      sockstream s( "localhost", 2008 );

      EXAM_CHECK( s.good() );
      EXAM_CHECK( s.is_open() );

      b2.wait();

      this_thread::sleep( milliseconds( 200 ) ); // chance for sigwait

      kill( child.pid(), SIGINT );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    shm.deallocate( &b2 );
    shm.deallocate( &b );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }
  
  return EXAM_RESULT;
}

class reader
{
  public:
    reader( sockstream& )
      { fill( buf, buf + sizeof(buf), 'b' ); }

    ~reader()
      { }

    void connect( sockstream& s )
      {
        static int i = 0;

        EXAM_CHECK_ASYNC( s.good() );
        ++i;
        fill( buf, buf + sizeof(buf), 'b' );
        s.read( buf, sizeof(buf) );

        /*
        if ( s.fail() ) {
          int j = 0;
          while ( buf[j] == ' ' && j < sizeof(buf) ) {
            ++j;
          }
          cerr << sizeof(buf) << ' ' << i << ' ' << j << endl;
        }
        */
        EXAM_CHECK_ASYNC( count( buf, buf + sizeof(buf), ' ' ) == sizeof(buf) );
        EXAM_CHECK_ASYNC( !s.fail() );
        // if ( s.fail() ) {
        //   cerr << count( buf, buf + sizeof(buf), ' ' ) << endl;
        // }

        // lock_guard<mutex> lk(lock);
        // if ( ++visits == N ) {
        //   cnd.notify_one();
        // }
      }

    // static bool n_cnt()
    //   { return visits == N; }

  private:
    char buf[64];

  public:
    // static mutex lock;
    // static condition_variable cnd;

  private:
    // static int visits;
};

// mutex reader::lock;

// condition_variable reader::cnd;

// int reader::visits = 0;

int EXAM_IMPL(sockios_test::quants_reader)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;

    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

    try {
      this_thread::fork();
      int ret = 0;

      try {
        std::tr2::this_thread::block_signal( SIGINT );

        connect_processor<reader> srv( 2008 );

        EXAM_CHECK_ASYNC_F( srv.good(), ret );
        EXAM_CHECK_ASYNC_F( srv.is_open(), ret );

        sigset_t signal_mask;

        sigemptyset( &signal_mask );
        sigaddset( &signal_mask, SIGINT );

        b.wait();

        int sig_caught;
        sigwait( &signal_mask, &sig_caught );

        if ( sig_caught == SIGINT ) {
          EXAM_MESSAGE_ASYNC( "catch INT signal" );
          this_thread::sleep( milliseconds( 200 ) ); // chance to process the rest
          srv.close();
        } else {
          EXAM_ERROR_ASYNC_F( "catch of INT signal expected", ret );
        }

      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unexpected exception", ret );
      }

      exit(ret);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait();
      {
        sockstream s( "localhost", 2008 );

        char buf[64];
        fill( buf, buf + sizeof(buf), ' ' );
        for ( int i = 0; i < /* 819200 */ 1024; ++i ) {
          s.write( buf, sizeof(buf) ).flush();
          EXAM_CHECK( s.good() );
        }
      }

      // this_thread::sleep( milliseconds( 500 ) );

      kill( child.pid(), SIGINT );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    shm.deallocate( &b );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }
  
  return EXAM_RESULT;
}
