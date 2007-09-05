// -*- C++ -*- Time-stamp: <07/09/05 00:44:02 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_test.h"
#include "message.h"

#include <exam/suite.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>

using namespace std;

sockios_test::sockios_test() :
    fname( "/tmp/sockios_test.shm" )
{
  try {
    seg.allocate( fname.c_str(), 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
  }
  catch ( const xmt::shm_bad_alloc& err ) {
    EXAM_ERROR_ASYNC( err.what() );
  }
}

sockios_test::~sockios_test()
{
  seg.deallocate();
  unlink( fname.c_str() );
}

/* ************************************************************ */

class Cnt
{
  public:
    Cnt( sockstream& )
      { xmt::scoped_lock lk(lock); ++cnt; ++visits; }

    ~Cnt()
      { xmt::scoped_lock lk(lock); --cnt; }

    void connect( sockstream& )
      { }

    void close()
      { }

    static int get_visits()
      { xmt::scoped_lock lk(lock); return visits; }

    static xmt::mutex lock;
    static int cnt;
    static int visits;
};

xmt::mutex Cnt::lock;
int Cnt::cnt = 0;
int Cnt::visits = 0;

int EXAM_IMPL(sockios_test::ctor_dtor)
{
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

  return EXAM_RESULT;
}

/* ************************************************************ */

class loader
{
  public:
    loader( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

loader::loader( std::sockstream& )
{
}

void loader::connect( std::sockstream& s )
{
  char buf[1024];

  s.read( buf, 1024 );
  s.write( buf, 1024 );
  s.flush();
}

void loader::close()
{
}

xmt::Thread::ret_t client_thr( void *p )
{
  sockstream s( "localhost", port );

  char buf[1024];

  fill( buf, buf + 1024, 0 );

  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  while( true ) {
    s.write( buf, 1024 );
    s.flush();

    s.read( buf, 1024 );
    xmt::Thread::yield();
  }

  return 0;
}

void sigpipe_handler( int sig, siginfo_t *si, void * )
{
#if 0
  cerr << "-----------------------------------------------\n"
       << "my pid: " << xmt::getpid() << ", ppid: " << xmt::getppid() << "\n"
       << "signal: " << sig << ", number " << si->si_signo
       << " errno " << si->si_errno
       << " code " << si->si_code << endl;
#endif
}
 
int EXAM_IMPL(sockios_test::sigpipe)
{
  try {
    xmt::__condition<true>& fcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
    fcnd.set( false );

    xmt::__condition<true>& tcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
    tcnd.set( false );

    try {
      xmt::fork();

      fcnd.try_wait();

      try {
        /*
         * This process will be killed,
         * so I don't care about safe termination.
         */
        const int b_count = 10;
        xmt::barrier b( b_count );
        xmt::Thread *th1 = new xmt::Thread( client_thr, &b );

        for ( int i = 0; i < (b_count - 1); ++i ) {
          new xmt::Thread( client_thr, &b );
        }

        xmt::delay( xmt::timespec(0,500000000) );

        tcnd.set( true );

        th1->join();

        exit( 0 );
      }
      catch ( ... ) {
      }
    }
    catch ( xmt::fork_in_parent& child ) {
      try {
        xmt::signal_handler( SIGPIPE, &sigpipe_handler );
        sockmgr_stream_MP<loader> srv( port );

        fcnd.set( true );

        tcnd.try_wait();

        kill( child.pid(), SIGTERM );

        int stat;
        waitpid( child.pid(), &stat, 0 );

        srv.close();
        srv.wait();
      }
      catch ( ... ) {
      }
    }

    (&tcnd)->~__condition<true>();
    shm_cnd.deallocate( &tcnd, 1 );
    (&fcnd)->~__condition<true>();
    shm_cnd.deallocate( &fcnd, 1 );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

/* ************************************************************ */

class long_msg_processor // 
{
  public:
    long_msg_processor( std::sockstream& );

    void connect( std::sockstream& );
    void close();

    static xmt::__condition<true> *cnd;
};

long_msg_processor::long_msg_processor( std::sockstream& )
{
  // cerr << "long_msg_processor::long_msg_processor" << endl;
}

void long_msg_processor::connect( std::sockstream& s )
{
  // cerr << "long_msg_processor::connect" << endl;

  string l;

  getline( s, l );

  // cerr << "Is good? " << s.good() << endl;
}

void long_msg_processor::close()
{
  // cerr << "long_msg_processor::close()" << endl;
  cnd->set( true );
}

xmt::__condition<true> *long_msg_processor::cnd;

int EXAM_IMPL(sockios_test::long_msg)
{
  try {
    xmt::__condition<true>& fcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
    fcnd.set( false );

    xmt::__condition<true>& srv_cnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
    srv_cnd.set( false );

    long_msg_processor::cnd = &srv_cnd;

    try {
      xmt::fork();

      fcnd.try_wait();

      {
        sockstream s( "localhost", port );

        s << "POST /test.php HTTP/1.1\r\n"
          << "xmlrequest=<?xml version=\"1.0\"?>\
<RWRequest><REQUEST domain=\"network\" service=\"ComplexReport\" nocache=\"n\" \
contact_id=\"1267\" entity=\"1\" filter_entity_id=\"1\" \
clientName=\"ui.ent\"><ROWS><ROW type=\"group\" priority=\"1\" ref=\"entity_id\" \
includeascolumn=\"n\"/><ROW type=\"group\" priority=\"2\" \
ref=\"advertiser_line_item_id\" includeascolumn=\"n\"/><ROW type=\"total\"/></ROWS><COLUMNS><COLUMN \
ref=\"advertiser_line_item_name\"/><COLUMN ref=\"seller_imps\"/><COLUMN \
ref=\"seller_clicks\"/><COLUMN ref=\"seller_convs\"/><COLUMN \
ref=\"click_rate\"/><COLUMN ref=\"conversion_rate\"/><COLUMN ref=\"roi\"/><COLUMN \
ref=\"network_revenue\"/><COLUMN ref=\"network_gross_cost\"/><COLUMN \
ref=\"network_gross_profit\"/><COLUMN ref=\"network_revenue_ecpm\"/><COLUMN \
ref=\"network_gross_cost_ecpm\"/><COLUMN \
ref=\"network_gross_profit_ecpm\"/></COLUMNS><FILTERS><FILTER ref=\"time\" \
macro=\"yesterday\"/></FILTERS></REQUEST></RWRequest>";
        s.flush();
      }     
      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      sockmgr_stream_MP<long_msg_processor> srv( port );
      fcnd.set( true );

      srv_cnd.try_wait();

      srv.close();
      srv.wait();
      
    }

    (&fcnd)->~__condition<true>();
    shm_cnd.deallocate( &fcnd, 1 );

    (&srv_cnd)->~__condition<true>();
    shm_cnd.deallocate( &srv_cnd, 1 );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

/* ************************************************************
 * The problem:
 *  1. Server listen socket (process A)
 *  2. Client connect to server (process B, server --- process A)
 *  3. Client try to read from socket (from server) and block on it,
 *     due to server write nothing (thread i) [Hmm, really here 
 *     poll with POLLIN flag and then read]
 *  4. In another thread (thread ii) client call system()
 *  5. Due to fork/execl (in system()) client return from read (step 3)
 *     with 0, i.e. as on close connection
 *
 * The POSIX say:
 *   (execl)
 * If the Asynchronous Input and Output option is supported, any
 * outstanding asynchronous I/O operations may be canceled. Those
 * asynchronous I/O operations that are not canceled will complete
 * as if the exec function had not yet occurred, but any associated
 * signal notifications are suppressed. It is unspecified whether
 * the exec function itself blocks awaiting such I/O completion. In no event,
 * however, will the new process image created by the exec function be affected
 * by the presence of outstanding asynchronous I/O operations at the time
 * the exec function is called. Whether any I/O is cancelled, and which I/O may
 * be cancelled upon exec, is implementation-dependent.
 *
 *  Client open socket  --------- system()
 *                      \
 *                        read...[poll signaled, read -> 0]
 *
 * The same issue on server side: poll that wait POLLIN from clients
 * signaled too.
 *
 */

class ConnectionProcessor5 // dummy variant
{
  public:
    ConnectionProcessor5( std::sockstream& );

    void connect( std::sockstream& );
    void close();

    static xmt::__barrier<true> *b;
};


xmt::__barrier<true> *ConnectionProcessor5::b = 0;

ConnectionProcessor5::ConnectionProcessor5( std::sockstream& s )
{
  // pr_lock.lock();
  // EXAM_MESSAGE( "Server seen connection" );

  EXAM_CHECK_ASYNC( s.good() );
  // pr_lock.unlock();
  
  // cerr << "ConnectionProcessor5::ConnectionProcessor5\n";
  // delay( xmt::timespec(3,0) );

  int n = 1;
  // cerr << "ConnectionProcessor5::ConnectionProcessor5, write\n";
  b->wait();
  s.write( (const char *)&n, sizeof( int ) ).flush();
}

void ConnectionProcessor5::connect( std::sockstream& s )
{
}

void ConnectionProcessor5::close()
{
  // pr_lock.lock();
  // EXAM_MESSAGE( "Server: client close connection" );
  // pr_lock.unlock();
}

xmt::Thread::ret_t thread_entry( void *par )
{
  xmt::condition& cnd = *reinterpret_cast<xmt::condition *>(par);

  // sem.wait(); // wait server for listen us
  sockstream sock( "localhost", ::port );
  int buff = 0;
  // cerr << "thread_entry" << endl;
  cnd.set( true );
  // Note: due to this is another process then main, boost can report
  // about errors here, but don't count error it in summary, if it occur!
  EXAM_CHECK_ASYNC( sock.read( (char *)&buff, sizeof(int) ).good() ); // <---- key line
  EXAM_CHECK_ASYNC( buff == 1 );
  // cerr << "Read pass" << endl;
  
  return 0;
}

int EXAM_IMPL(sockios_test::read0)
{
  try {
    xmt::__condition<true>& fcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
    xmt::__barrier<true>& b = *new ( shm_b.allocate( 1 ) ) xmt::__barrier<true>();
    ConnectionProcessor5::b = &b;
    // nm.named( fcnd, 1 );
    fcnd.set( false );

    try {
      xmt::fork();                      // <---- key line
      fcnd.try_wait(); // wait server for listen us

      xmt::condition cnd;
      cnd.set( false );

      xmt::Thread thr( thread_entry, &cnd );

      cnd.try_wait(); // wait for read call

      // delay( xmt::timespec(1,0) );

      // cerr << "system" << endl;
      system( "echo > /dev/null" );  // <------ key line
      // cerr << "after system" << endl;

      b.wait();

      thr.join();
      // cerr << "exit child" << endl;
      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      // cerr << "** 3" << endl;
      sockmgr_stream_MP<ConnectionProcessor5> srv( ::port ); // start server

      fcnd.set( true );

      int stat;
      waitpid( child.pid(), &stat, 0 );
      srv.close();
      srv.wait();
    }

    (&b)->~__barrier<true>();
    shm_b.deallocate( &b, 1 );
    (&fcnd)->~__condition<true>();
    shm_cnd.deallocate( &fcnd, 1 );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

/* ************************************************************ */

class ConnectionProcessor6 // dummy variant
{
  public:
    ConnectionProcessor6( std::sockstream& );

    void connect( std::sockstream& );
    void close();

    static xmt::condition cnd;
};

xmt::condition ConnectionProcessor6::cnd;

ConnectionProcessor6::ConnectionProcessor6( std::sockstream& s )
{
  // pr_lock.lock();
  // EXAM_MESSAGE( "Server seen connection" );

  EXAM_CHECK_ASYNC( s.good() );
  // pr_lock.unlock();

  cnd.set( true );
}

void ConnectionProcessor6::connect( std::sockstream& s )
{
}

void ConnectionProcessor6::close()
{
  // pr_lock.lock();
  // EXAM_MESSAGE( "Server: client close connection" );
  // pr_lock.unlock();
}

int EXAM_IMPL(sockios_test::read0_srv)
{
  try {
    sockmgr_stream_MP<ConnectionProcessor6> srv( ::port );

    EXAM_CHECK( srv.good() );
    ConnectionProcessor6::cnd.set( false );

    {
      // It should work as before system call...
      sockstream s( "localhost", ::port );

      s << "1" << endl;

      EXAM_CHECK( s.good() );

      ConnectionProcessor6::cnd.try_wait();
    }

    ConnectionProcessor6::cnd.set( false );

    system( "echo > /dev/null" );  // <------ key line

    EXAM_CHECK( srv.good() );

    {
      // ... as after system call.
      sockstream s( "localhost", ::port );

      s << "1" << endl;

      EXAM_CHECK( s.good() );

      ConnectionProcessor6::cnd.try_wait();
    }

    EXAM_CHECK( srv.good() ); // server must correctly process interrupt during system call

    srv.close();
    srv.wait();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

/* ************************************************************ */

class LongBlockReader // dummy variant
{
  public:
    LongBlockReader( std::sockstream& );

    void connect( std::sockstream& );
    void close();

    static xmt::condition cnd;
};

xmt::condition LongBlockReader::cnd;

LongBlockReader::LongBlockReader( std::sockstream& s )
{
  EXAM_CHECK_ASYNC( s.good() );
}

void LongBlockReader::connect( std::sockstream& s )
{
  char buf[1024];
  int count = 0;

  for ( int i = 0; i < 1024 * 1024; ++i ) {
    s.read( buf, 1024 );
  }
  cnd.set( true );
}

void LongBlockReader::close()
{
  // pr_lock.lock();
  // EXAM_MESSAGE( "Server: client close connection" );
  // pr_lock.unlock();
}

int EXAM_IMPL(sockios_test::long_block_read)
{
  LongBlockReader::cnd.set( false );

  sockmgr_stream_MP<LongBlockReader> srv( ::port );
  
  EXAM_REQUIRE( srv.good() );

  sockstream s;

  s.open( "localhost", ::port );

  EXAM_REQUIRE( s.good() );

  char buf[1024];

  for ( int i = 0; i < 1024 * 1024; ++i ) {
    s.write( buf, 1024 );
  }
  s.flush();

  EXAM_CHECK( s.good() );

  s.close();
  LongBlockReader::cnd.try_wait();

  srv.close();
  srv.wait();

  // try {
  // }
  // catch () {
  // }
}
