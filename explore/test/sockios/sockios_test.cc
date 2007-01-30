// -*- C++ -*- Time-stamp: <07/01/30 11:45:52 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_test.h"
#include "message.h"

#include <boost/test/unit_test.hpp>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <list>

#include <arpa/inet.h>

#include <mt/shm.h>

// #include <sys/shm.h>
#include <sys/wait.h>

#include <signal.h>

using namespace boost::unit_test_framework;
using namespace std;

void sockios_test::hostname_test()
{
  unsigned long local = htonl( 0x7f000001 ); // 127.0.0.1

#ifdef _LITTLE_ENDIAN
  BOOST_CHECK_EQUAL( local, 0x0100007f );
#endif

#ifdef _BIG_ENDIAN
  BOOST_CHECK_EQUAL( local, 0x7f000001 );
#endif

  BOOST_CHECK_EQUAL( std::hostname( local ), "localhost [127.0.0.1]" );

#ifdef __unix
  char buff[1024];

  gethostname( buff, 1024 );

  BOOST_CHECK_EQUAL( std::hostname(), buff );
#endif
}

void sockios_test::service_test()
{
#ifdef __unix
  BOOST_CHECK( std::service( "ftp", "tcp" ) == 21 );
  BOOST_CHECK( std::service( 7, "udp" ) == "echo" );
#else
  BOOST_ERROR( "requests for service (/etc/services) not implemented on this platform" );
#endif
}

void sockios_test::hostaddr_test1()
{
#ifdef __unix
  in_addr addr = std::findhost( "localhost" );

# ifdef _LITTLE_ENDIAN
  BOOST_CHECK_EQUAL( addr.s_addr, 0x0100007f );
# endif

# ifdef _BIG_ENDIAN
  BOOST_CHECK_EQUAL( addr.s_addr, 0x7f000001 );
# endif
  
#else
  BOOST_ERROR( "Not implemented" );
#endif
}

void sockios_test::hostaddr_test2()
{
#ifdef __unix
  list<in_addr> haddrs;
  std::gethostaddr( "localhost", back_inserter(haddrs) );

  bool localhost_found = false;

  for ( list<in_addr>::const_iterator i = haddrs.begin(); i != haddrs.end(); ++i ) {
    if ( i->s_addr == htonl( 0x7f000001 ) ) { // 127.0.0.1
      localhost_found = true;
      break;
    }
  }
  
  BOOST_CHECK( localhost_found == true );
  
#else
  BOOST_ERROR( "Not implemented" );
#endif
}

void sockios_test::hostaddr_test3()
{
#ifdef __unix
  list<sockaddr> haddrs;
  gethostaddr2( "localhost", back_inserter(haddrs) );

  bool localhost_found = false;

  for ( list<sockaddr>::const_iterator i = haddrs.begin(); i != haddrs.end(); ++i ) {
    switch ( i->sa_family ) {
      case PF_INET:
        if ( ((sockaddr_in *)&*i)->sin_addr.s_addr == htonl( 0x7f000001 ) ) {
          localhost_found = true;
        }
        break;
      case PF_INET6:
        if ( ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[0] == 0 &&
             ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[1] == 0 && 
             ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[2] == 0 &&
             ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[3] == 1 ) {
          localhost_found = true;
        }
        break;
    }
  }
  
  BOOST_CHECK( localhost_found == true );
  
#else
  BOOST_ERROR( "Not implemented" );
#endif
}

class Cnt
{
  public:
    Cnt( sockstream& )
      { xmt::Locker lk(lock); ++cnt; }

    ~Cnt()
      { xmt::Locker lk(lock); --cnt; }

    void connect( sockstream& )
      { }

    void close()
      { }

    static xmt::Mutex lock;
    static int cnt;
};

xmt::Mutex Cnt::lock;
int Cnt::cnt = 0;

void sockios_test::ctor_dtor()
{
  // Check, that naumber of ctors of Cnt is the same as number of called dtors
  // i.e. all created Cnt freed.
  // due to async nature of communication, no way to check Cnt::cnt
  // before server stop.
  {
    sockmgr_stream_MP<Cnt> srv( port );

    {
      sockstream s1( "localhost", port );

      BOOST_CHECK( s1.good() );
      BOOST_CHECK( s1.is_open() );

      s1 << "1234" << endl;

      BOOST_CHECK( s1.good() );
      BOOST_CHECK( s1.is_open() );
    }

    srv.close();
    srv.wait();

    Cnt::lock.lock();
    BOOST_CHECK( Cnt::cnt == 0 );
    Cnt::lock.unlock();
    
  }
  {
    sockmgr_stream_MP<Cnt> srv( port );

    {
      sockstream s1( "localhost", port );
      sockstream s2( "localhost", port );

      BOOST_CHECK( s1.good() );
      BOOST_CHECK( s1.is_open() );
      BOOST_CHECK( s2.good() );
      BOOST_CHECK( s2.is_open() );

      s1 << "1234" << endl;
      s2 << "1234" << endl;

      BOOST_CHECK( s1.good() );
      BOOST_CHECK( s1.is_open() );
      BOOST_CHECK( s2.good() );
      BOOST_CHECK( s2.is_open() );
    }

    srv.close();
    srv.wait();

    Cnt::lock.lock();
    BOOST_CHECK( Cnt::cnt == 0 );
    Cnt::lock.unlock();
    
  }
}

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

xmt::Thread::ret_code client_thr( void * )
{
  xmt::Thread::ret_code ret;

  ret.iword = 0;

  sockstream s( "localhost", port );

  char buf[1024];

  fill( buf, buf + 1024, 0 );

  while( true ) {
    s.write( buf, 1024 );
    s.flush();

    s.read( buf, 1024 );
  }

  return ret;
}

void sigpipe_handler( int sig, siginfo_t *si, void * )
{
  cerr << "-----------------------------------------------\n"
       << "my pid: " << xmt::getpid() << ", ppid: " << xmt::getppid() << "\n"
       << "signal: " << sig << ", number " << si->si_signo
       << " errno " << si->si_errno
       << " code " << si->si_code << endl;
}
 
void sockios_test::sigpipe()
{
  const char fname[] = "/tmp/sockios_test.shm";
  enum {
    in_Child_Condition = 1,
    threads_Started_Condition = 2
  };
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( fname, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
    xmt::shm_name_mgr<0>& nm = seg.name_mgr();

    xmt::allocator_shm<xmt::__Condition<true>,0> shm;

    xmt::__Condition<true>& fcnd = *new ( shm.allocate( 1 ) ) xmt::__Condition<true>();
    nm.named( fcnd, in_Child_Condition );
    fcnd.set( false );

    xmt::__Condition<true>& tcnd = *new ( shm.allocate( 1 ) ) xmt::__Condition<true>();
    nm.named( tcnd, threads_Started_Condition );
    tcnd.set( false );

    try {
      xmt::fork();

      fcnd.try_wait();

      try {
        /*
         * This process will be killed,
         * so I don't care about safe termination.
         */
        xmt::Thread *th1 = new xmt::Thread( client_thr );
        for ( int i = 0; i < /* 10 */ 2; ++i ) {
          new xmt::Thread( client_thr );
          new xmt::Thread( client_thr );
          new xmt::Thread( client_thr );
          new xmt::Thread( client_thr );
        }

        xmt::delay( xmt::timespec(1,0) );

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

    (&tcnd)->~__Condition<true>();
    shm.deallocate( &tcnd, 1 );
    (&fcnd)->~__Condition<true>();
    shm.deallocate( &fcnd, 1 );

    seg.deallocate();
    unlink( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}

class long_msg_processor // 
{
  public:
    long_msg_processor( std::sockstream& );

    void connect( std::sockstream& );
    void close();

    static xmt::__Condition<true> *cnd;
};

long_msg_processor::long_msg_processor( std::sockstream& )
{
  cerr << "long_msg_processor::long_msg_processor" << endl;
}

void long_msg_processor::connect( std::sockstream& s )
{
  cerr << "long_msg_processor::connect" << endl;

  string l;

  getline( s, l );

  cerr << "Is good? " << s.good() << endl;
}

void long_msg_processor::close()
{
  cerr << "long_msg_processor::close()" << endl;
  cnd->set( true );
}

xmt::__Condition<true> *long_msg_processor::cnd;

void sockios_test::long_msg_test()
{
  const char fname[] = "/tmp/sockios_test.shm";
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( fname, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
    xmt::shm_name_mgr<0>& nm = seg.name_mgr();

    xmt::allocator_shm<xmt::__Condition<true>,0> shm;

    xmt::__Condition<true>& fcnd = *new ( shm.allocate( 1 ) ) xmt::__Condition<true>();
    fcnd.set( false );

    xmt::__Condition<true>& srv_cnd = *new ( shm.allocate( 1 ) ) xmt::__Condition<true>();
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

    (&fcnd)->~__Condition<true>();
    shm.deallocate( &fcnd, 1 );

    (&srv_cnd)->~__Condition<true>();
    shm.deallocate( &srv_cnd, 1 );

    seg.deallocate();
    unlink( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }

}
