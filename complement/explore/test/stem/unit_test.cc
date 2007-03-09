// -*- C++ -*- Time-stamp: <07/03/09 20:43:27 ptr>

/*
 * Copyright (c) 2002, 2003, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include <iostream>
#include <mt/xmt.h>
#include <mt/shm.h>

#include <stem/EventHandler.h>
#include <stem/Names.h>
#include <stem/EDSEv.h>

#include "Node.h"
#include "NodeDL.h"
#include "NameService.h"

#include <dlfcn.h>

#include "Echo.h"
#include <stem/NetTransport.h>
#include <stem/EvManager.h>
#include <sockios/sockmgr.h>

#ifndef STLPORT
#include <ext/functional>
using namespace __gnu_cxx;
#endif

#include <sys/wait.h>

#include <signal.h>

using namespace std;

struct stem_test
{
    void shm_init();
    void shm_finit();

    void basic1();
    void basic2();
    void basic1new();
    void basic2new();
    void dl();
    void ns();

    void echo();
    void echo_net();
    void peer();
    void boring_manager();
    void net_echo();
    void net_echo2();

    static xmt::Thread::ret_code thr1( void * );
    static xmt::Thread::ret_code thr1new( void * );
};

void stem_test::basic1()
{
  Node node( 2000 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node.Send( ev );

  node.wait();
  BOOST_CHECK_EQUAL( node.v, 1 );
}

void stem_test::basic2()
{
  Node node( 2000 );
  
  xmt::Thread t1( thr1 );

  t1.join();

  node.wait();

  BOOST_CHECK_EQUAL( node.v, 1 );
}

xmt::Thread::ret_code stem_test::thr1( void * )
{
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  Node node( 2001 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node.Send( ev );

  return rt;
}

void stem_test::basic1new()
{
  NewNode *node = new NewNode( 2000 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node->Send( ev );

  node->wait();

  BOOST_CHECK_EQUAL( node->v, 1 );
  delete node;
}

void stem_test::basic2new()
{
  NewNode *node = new NewNode( 2000 );
  
  xmt::Thread t1( thr1new );

  t1.join();

  node->wait();
  BOOST_CHECK_EQUAL( node->v, 1 );
  delete node;
}

xmt::Thread::ret_code stem_test::thr1new( void * )
{
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  NewNode *node = new NewNode( 2001 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node->Send( ev );

  delete node;

  return rt;
}

void stem_test::dl()
{
  void *lh = dlopen( "libloadable_stem.so", RTLD_LAZY ); // Path was passed via -Wl,--rpath=

  BOOST_REQUIRE( lh != NULL );
  void *(*f)(unsigned);
  void (*g)(void *);
  void (*w)(void *);
  int (*v)(void *);

  *(void **)(&f) = dlsym( lh, "create_NewNodeDL" );
  BOOST_REQUIRE( f != NULL );
  *(void **)(&g) = dlsym( lh, "destroy_NewNodeDL" );
  BOOST_REQUIRE( g != NULL );
  *(void **)(&w) = dlsym( lh, "wait_NewNodeDL" );
  BOOST_REQUIRE( w != NULL );
  *(void **)(&v) = dlsym( lh, "v_NewNodeDL" );
  BOOST_REQUIRE( v != NULL );

  NewNodeDL *node = reinterpret_cast<NewNodeDL *>( f( 2002 )  );
  stem::Event ev( NODE_EV2 );
  ev.dest( 2002 );
  node->Send( ev );

  w( reinterpret_cast<void *>(node) );
  BOOST_CHECK_EQUAL( v(reinterpret_cast<void *>(node)), 1 );

  g( reinterpret_cast<void *>(node) );
  dlclose( lh );
}

void stem_test::ns()
{
  Node node( 2003, "Node" );
  Naming nm;

  stem::Event ev( EV_STEM_GET_NS_LIST );
  ev.dest( stem::ns_addr );
  nm.Send( ev );

  nm.wait();

  // this is sample of all inline find:
  Naming::nsrecords_type::const_iterator i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( equal_to<string>(), string( "ns" ) ), select2nd<pair<stem::gaddr_type,string> >() ) );

  BOOST_CHECK( i != nm.lst.end() );
  BOOST_CHECK( i->second == "ns" );
  BOOST_CHECK( i->first.hid == xmt::hostid() );
  BOOST_CHECK( i->first.pid == getpid() );
  BOOST_CHECK( i->first.addr == stem::ns_addr );

  // well, but for few seaches declare and reuse functors:
  equal_to<string> eq;
  equal_to<stem::gaddr_type> eqa;

  select1st<pair<stem::gaddr_type,string> > first;
  select2nd<pair<stem::gaddr_type,string> > second;

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eq, string( "Node" ) ), second ) );

  BOOST_CHECK( i != nm.lst.end() );
  BOOST_CHECK( i->second == "Node" );
  BOOST_CHECK( i->first.addr == 2003 );

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eqa, nm.self_glid() ), first ) );

  BOOST_CHECK( i != nm.lst.end() );
  BOOST_CHECK( i->first == nm.self_glid() );
  BOOST_CHECK( i->second.length() == 0 );

  nm.lst.clear();
  nm.reset();

  BOOST_CHECK( nm.lst.empty() );

  stem::Event evname( EV_STEM_GET_NS_NAME );
  evname.dest( stem::ns_addr );
  evname.value() = "Node";
  nm.Send( evname );

  nm.wait();

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eq, string( "ns" ) ), second ) );

  BOOST_CHECK( i == nm.lst.end() );

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eq, string( "Node" ) ), second ) );

  BOOST_CHECK( i != nm.lst.end() );
  BOOST_CHECK( i->second == "Node" );
  BOOST_CHECK( i->first.addr == 2003 );

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eqa, nm.self_glid() ), first ) );

  BOOST_CHECK( i == nm.lst.end() );

  nm.lst.clear();
  nm.reset();

  BOOST_CHECK( nm.lst.empty() );

  evname.value() = "No-such-name";
  nm.Send( evname );

  nm.wait();

  BOOST_CHECK( nm.lst.empty() );
}

void stem_test::echo()
{
  try {
    sockmgr_stream_MP<stem::NetTransport> srv( 6995 );
    stem::NetTransportMgr mgr;

    StEMecho echo( 0, "echo service"); // <= zero!

    stem::addr_type zero = mgr.open( "localhost", 6995 );

    BOOST_CHECK( zero != stem::badaddr );
    BOOST_CHECK( zero == 0 ); // NetTransportMgr should detect local delivery

    EchoClient node;
    
    stem::Event ev( NODE_EV_ECHO );

    ev.dest( zero );
    ev.value() = node.mess;

    node.Send( ev );

    node.wait();
    
    mgr.close();
    mgr.join();

    srv.close();
    srv.wait();
  }
  catch ( ... ) {
  }
}

const char fname[] = "/tmp/stem_test.shm";
xmt::shm_alloc<0> seg;
xmt::allocator_shm<xmt::__Condition<true>,0> shm_cnd;
xmt::allocator_shm<xmt::__Barrier<true>,0>   shm_b;

void stem_test::shm_init()
{
  try {
    seg.allocate( fname, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
  }
  catch ( const xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}

void stem_test::shm_finit()
{
  seg.deallocate();
  unlink( fname );
}

void stem_test::echo_net()
{
  xmt::__Condition<true>& fcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__Condition<true>();
  fcnd.set( false );

  try {
    xmt::fork();

    try {
      stem::NetTransportMgr mgr;

      fcnd.try_wait();

      stem::addr_type zero = mgr.open( "localhost", 6995 );

      BOOST_CHECK( zero != stem::badaddr );
      BOOST_CHECK( zero != 0 ); // NetTransportMgr should detect external delivery

      EchoClient node;
    
      stem::Event ev( NODE_EV_ECHO );

      ev.dest( zero );
      ev.value() = node.mess;

      node.Send( ev );

      node.wait();
    
      mgr.close();
      mgr.join();
    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    try {
      sockmgr_stream_MP<stem::NetTransport> srv( 6995 );

      StEMecho echo( 0, "echo service"); // <= zero!

      fcnd.set( true );

      int stat;
      waitpid( child.pid(), &stat, 0 );

      srv.close();
      srv.wait();
    }
    catch ( ... ) {
    }
  }

  (&fcnd)->~__Condition<true>();
  shm_cnd.deallocate( &fcnd, 1 );

  // cerr << "Fine\n";
}

extern "C" {

static void dummy_signal_handler( int )
{ }

}

void stem_test::peer()
{
  /*
   * Scheme:
   *                      / NetTransport      / c1
   *  Local Event Manager  -  NetTransportMgr - c2
   *                      \ echo
   * Due to all objects in the same process space,
   * c1, c2 and echo in different processes.
   *
   * The logical scheme is:
   *
   *        / c1
   *   echo
   *        \ c2
   *
   * (c1 <-> c2, through 'echo')
   */

  pid_t fpid;

  xmt::__Condition<true>& fcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__Condition<true>();
  fcnd.set( false );

  xmt::__Condition<true>& pcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__Condition<true>();
  pcnd.set( false );

  xmt::__Condition<true>& scnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__Condition<true>();
  scnd.set( false );

  try {
    // Client 1
    xmt::fork();
#if 0
    struct sigaction action;
    struct sigaction old_action;
    action.sa_flags = 0;
    action.sa_handler = &dummy_signal_handler;
    sigemptyset( &action.sa_mask );

    sigaction( SIGFPE, &action, &old_action );
    sigaction( SIGTRAP, &action, &old_action );
    sigaction( SIGSEGV, &action, &old_action );
    sigaction( SIGBUS, &action, &old_action );
    sigaction( SIGABRT, &action, &old_action );
    sigaction( SIGALRM, &action, &old_action );
#endif

    try {
      stem::NetTransportMgr mgr;

      PeerClient c1( "c1 local" );  // c1 client
      Naming nm;

      fcnd.try_wait();

      stem::addr_type zero = mgr.open( "localhost", 6995 ); // take address of 'zero' (aka default) object via net transport from server
      // It done like it should on client side

      BOOST_CHECK( zero != stem::badaddr );
      BOOST_CHECK( zero != 0 );
      BOOST_CHECK( zero & stem::extbit ); // "external" address

      stem::Event ev( NODE_EV_REGME );
      ev.dest( zero );

      ev.value() = "c1@here";
      c1.Send( ev ); // 'register' c1 client on 'echo' server

      stem::gaddr_type ga( c1.manager()->reflect( zero ) );

      BOOST_CHECK( ga.addr == 0 );
      BOOST_CHECK( ga.pid != -1 );

      ga.addr = stem::ns_addr; // this will be global address of ns of the same process
                               // as zero

      BOOST_CHECK( c1.manager()->reflect( ga ) != stem::badaddr );

      stem::Event evname( EV_STEM_GET_NS_NAME );
      evname.dest( c1.manager()->reflect( ga ) );
      evname.value() = "c2@here";

      pcnd.try_wait();

      Naming::nsrecords_type::const_iterator i;

      do {
        nm.reset();
        nm.lst.clear();
        nm.Send( evname );
        nm.wait();
        i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( equal_to<string>(), string( "c2@here" ) ), select2nd<pair<stem::gaddr_type,string> >() ) );
      } while ( i == nm.lst.end() );

      BOOST_CHECK( i != nm.lst.end() );
      BOOST_CHECK( i->second == "c2@here" );

      stem::addr_type pa = c1.manager()->reflect( i->first );
      if ( pa == stem::badaddr ) { // unknown yet
        pa = c1.manager()->SubscribeRemote( i->first, i->second );
        if ( pa == stem::badaddr ) { // it still unknown, transport not found
          // hint: use transport as object zero used
          pa = c1.manager()->SubscribeRemote( c1.manager()->transport( zero ), i->first, i->second );
        }
      }

      BOOST_CHECK( pa != stem::badaddr );

      if ( pa != stem::badaddr ) {
        stem::Event pe( NODE_EV_ECHO );
        pe.dest( pa );
        pe.value() = "c2 local"; // <<-- mess is like name ... |
                                 //                            .
        c1.Send( pe );
      }

      scnd.try_wait();

      mgr.close();
      mgr.join();
    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    fpid = child.pid();
  }

  try {
    // Client 2
    xmt::fork();

#if 0
    struct sigaction action;
    struct sigaction old_action;
    action.sa_flags = 0;
    action.sa_handler = &dummy_signal_handler;
    sigemptyset( &action.sa_mask );

    sigaction( SIGFPE, &action, &old_action );
    sigaction( SIGTRAP, &action, &old_action );
    sigaction( SIGSEGV, &action, &old_action );
    sigaction( SIGBUS, &action, &old_action );
    sigaction( SIGABRT, &action, &old_action );
    sigaction( SIGALRM, &action, &old_action );
#endif

    try {
      stem::NetTransportMgr mgr;
                                   //                                          ^
      PeerClient c2( "c2 local" ); // <<--- name the same as mess expected ... |

      fcnd.try_wait();
      stem::addr_type zero = mgr.open( "localhost", 6995 ); // take address of 'zero' (aka default) object via net transport from server
      // It done like it should on client side

      BOOST_CHECK( zero != stem::badaddr );
      BOOST_CHECK( zero != 0 );
      BOOST_CHECK( zero & stem::extbit ); // "external" address

      stem::Event ev( NODE_EV_REGME );
      ev.dest( zero );

      ev.value() = "c2@here";
      c2.Send( ev ); // 'register' c2 client on 'echo' server

      pcnd.set( true );

      c2.wait();
      // cerr << "Fine!" << endl;
      scnd.set( true );

      mgr.close();
      mgr.join();
    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    sockmgr_stream_MP<stem::NetTransport> srv( 6995 ); // server, it serve 'echo'
    StEMecho echo( 0, "echo service"); // <= zero! 'echo' server, default ('zero' address)

    fcnd.set( true, true );

    int stat;
    waitpid( child.pid(), &stat, 0 );
    waitpid( fpid, &stat, 0 );

    srv.close();
    srv.wait();
  }

  (&fcnd)->~__Condition<true>();
  shm_cnd.deallocate( &fcnd, 1 );
  (&pcnd)->~__Condition<true>();
  shm_cnd.deallocate( &pcnd, 1 );
  (&scnd)->~__Condition<true>();
  shm_cnd.deallocate( &scnd, 1 );
}

void stem_test::boring_manager()
{
  xmt::__Condition<true>& fcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__Condition<true>();
  fcnd.set( false );

  try {
    // Client
    xmt::fork();
    try {
      fcnd.try_wait();

      for ( int i = 0; i < 10; ++i ) {
        const int n = 10;
        stem::NetTransportMgr *mgr[n];
        mgr[0] = new stem::NetTransportMgr;
        mgr[0]->open( "localhost", 6995 );

        for ( int j = 1; j < n; ++j ) {
          mgr[j] = new stem::NetTransportMgr;
          stem::addr_type a = mgr[j]->open( "localhost", 6995 );
          mgr[j]->close();
          mgr[j]->join();
          delete mgr[j];
        }
        mgr[0]->close();
        mgr[0]->join();
        delete mgr[0];
      }
    }
    catch ( ... ) {
    }
    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    sockmgr_stream_MP<stem::NetTransport> srv( 6995 ); // server, it serve 'echo'
    StEMecho echo( 0, "echo service"); // <= zero! 'echo' server, default ('zero' address)

    fcnd.set( true, true );

    int stat;
    waitpid( child.pid(), &stat, 0 );

    srv.close();
    srv.wait();
  }

  (&fcnd)->~__Condition<true>();
  shm_cnd.deallocate( &fcnd, 1 );
}

// -----------------

#define EV_STU_MESS 0x801
#define EV_STU_ECHO 0x802

class TestObj :
    public stem::EventHandler
{
  public:
    TestObj()
      { }

    void reset()
      { cnd.set( false ); }

    void wait()
      { cnd.try_wait(); }

    const std::string& msg() const
      { return m; }

    void mess( const stem::Event& );
    void echo( const stem::Event& );

  private:
    xmt::Condition cnd;
    std::string m;

    DECLARE_RESPONSE_TABLE( TestObj, stem::EventHandler );
};

void TestObj::mess( const stem::Event& ev )
{
  // cerr << "hi!\n";
  m = ev.value();
  cnd.set( true );
}

void TestObj::echo( const stem::Event& ev )
{
  m = ev.value();
  cnd.set( true );
}

DEFINE_RESPONSE_TABLE( TestObj )
  EV_EDS( ST_NULL, EV_STU_MESS, mess )
  EV_EDS( ST_NULL, EV_STU_ECHO, echo )
END_RESPONSE_TABLE

class EchoSrv :
    public stem::EventHandler
{
  public:
    EchoSrv();
    EchoSrv( stem::addr_type id );
    EchoSrv( stem::addr_type id, const char * );

    void echo( const stem::Event& );

  private:
    DECLARE_RESPONSE_TABLE( EchoSrv, stem::EventHandler );
};

EchoSrv::EchoSrv()
{
}

EchoSrv::EchoSrv( stem::addr_type id ) :
    stem::EventHandler( id )
{
}

EchoSrv::EchoSrv( stem::addr_type id, const char *info ) :
    stem::EventHandler( id, info )
{
}

// Unconditionally echo message

void EchoSrv::echo( const stem::Event& ev )
{
  cerr << __FILE__ << ":" << __LINE__ << endl;
  ev.dest( ev.src() );
  Forward( ev );
}

DEFINE_RESPONSE_TABLE( EchoSrv )
  EV_EDS( ST_NULL, EV_STU_ECHO, echo )
END_RESPONSE_TABLE

void stem_test::net_echo()
{
  try {
    xmt::__Barrier<true>& b = *new ( shm_b.allocate( 1 ) ) xmt::__Barrier<true>();
    xmt::__Condition<true>& c = *new ( shm_cnd.allocate( 1 ) ) xmt::__Condition<true>();

    c.set( false );

    try {
      xmt::fork();

      // server part
      {
        std::sockmgr_stream_MP<stem::NetTransport> srv( 6890 );
        EchoSrv echosrv( 0, "Echo service");

        BOOST_REQUIRE( srv.good() );
        c.set( true ); // ok, server listen

        b.wait(); // server may go away

        srv.close();
        srv.wait();
      }

      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      // client part

      stem::NetTransportMgr client;

      c.try_wait(); // wait server start

      stem::addr_type a = client.open( "localhost", 6890 );

      BOOST_REQUIRE( client.good() );
      BOOST_REQUIRE( a != stem::badaddr );

      TestObj t1;

      t1.reset();

      stem::Event ev( EV_STU_ECHO );
      ev.dest( a );

      ev.value() = "echo";
      t1.Send( ev );

      t1.wait();

      BOOST_CHECK( t1.msg() == "echo" );

      client.close();
      client.join();

      b.wait(); // server may go away

      int stat;
      BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }

    (&c)->~__Condition<true>();
    shm_cnd.deallocate( &c, 1 );
    (&b)->~__Barrier<true>();
    shm_b.deallocate( &b, 1 );
  }
  catch (  xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}

void stem_test::net_echo2()
{
  try {
    xmt::__Barrier<true>& b = *new ( shm_b.allocate( 1 ) ) xmt::__Barrier<true>();
    xmt::__Condition<true>& c = *new ( shm_cnd.allocate( 1 ) ) xmt::__Condition<true>();

    c.set( false );

    try {
      xmt::fork();

      // server part
      {
        std::sockmgr_stream_MP<stem::NetTransport> srv( 6890 );
        EchoSrv echosrv( 0, "Echo service");

        BOOST_REQUIRE( srv.good() );
        c.set( true ); // ok, server listen

        b.wait(); // server may go away

        srv.close();
        srv.wait();
      }

      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      // client part

      stem::NetTransportMgr client;

      c.try_wait(); // wait server start

      stem::addr_type a = client.open( "localhost", 6890 );

      BOOST_REQUIRE( client.good() );
      BOOST_REQUIRE( a != stem::badaddr );

      TestObj t1;

      t1.reset();

      stem::Event ev( EV_STU_ECHO );
      ev.dest( a );

      ev.value() = "echo";
      t1.Send( ev );

      t1.wait();

      BOOST_CHECK( t1.msg() == "echo" );

      client.close();
      client.join();

      b.wait(); // server may go away

      int stat;
      BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }

    (&c)->~__Condition<true>();
    shm_cnd.deallocate( &c, 1 );
    (&b)->~__Barrier<true>();
    shm_b.deallocate( &b, 1 );
  }
  catch (  xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}

// -----------------

struct stem_test_suite :
    public test_suite
{
    stem_test_suite();
};


stem_test_suite::stem_test_suite() :
    test_suite( "StEM test suite" )
{
  boost::shared_ptr<stem_test> instance( new stem_test() );

  test_case *basic1_tc = BOOST_CLASS_TEST_CASE( &stem_test::basic1, instance );
  test_case *basic2_tc = BOOST_CLASS_TEST_CASE( &stem_test::basic2, instance );
  test_case *basic1n_tc = BOOST_CLASS_TEST_CASE( &stem_test::basic1new, instance );
  test_case *basic2n_tc = BOOST_CLASS_TEST_CASE( &stem_test::basic2new, instance );
  test_case *dl_tc = BOOST_CLASS_TEST_CASE( &stem_test::dl, instance );
  test_case *ns_tc = BOOST_CLASS_TEST_CASE( &stem_test::ns, instance );
  test_case *echo_tc = BOOST_CLASS_TEST_CASE( &stem_test::echo, instance );
  test_case *shm_init_tc = BOOST_CLASS_TEST_CASE( &stem_test::shm_init, instance );
  test_case *echo_net_tc = BOOST_CLASS_TEST_CASE( &stem_test::echo_net, instance );
  test_case *peer_tc = BOOST_CLASS_TEST_CASE( &stem_test::peer, instance );
  test_case *boring_manager_tc = BOOST_CLASS_TEST_CASE( &stem_test::boring_manager, instance );
  test_case *net_echo_tc = BOOST_CLASS_TEST_CASE( &stem_test::net_echo, instance );
  // test_case *net_echo2_tc = BOOST_CLASS_TEST_CASE( &stem_test::net_echo2, instance );
  test_case *shm_finit_tc = BOOST_CLASS_TEST_CASE( &stem_test::shm_finit, instance );

  basic2_tc->depends_on( basic1_tc );
  basic1n_tc->depends_on( basic1_tc );
  basic2n_tc->depends_on( basic2_tc );
  basic2n_tc->depends_on( basic1n_tc );
  dl_tc->depends_on( basic2n_tc );
  ns_tc->depends_on( basic1_tc );

  echo_tc->depends_on( basic2_tc );
  echo_net_tc->depends_on( shm_init_tc );
  echo_net_tc->depends_on( echo_tc );
  peer_tc->depends_on( echo_tc );
  peer_tc->depends_on( shm_init_tc );
  boring_manager_tc->depends_on( peer_tc );
  boring_manager_tc->depends_on( shm_init_tc );
  net_echo_tc->depends_on( shm_init_tc );
  net_echo_tc->depends_on( echo_net_tc );
  // net_echo2_tc->depends_on( shm_init_tc );
  // net_echo2_tc->depends_on( net_echo_tc );
  shm_finit_tc->depends_on( shm_init_tc );

  add( basic1_tc );
  add( basic2_tc );
  add( basic1n_tc );
  add( basic2n_tc );
  add( dl_tc );
  add( ns_tc );

  add( echo_tc );
  add( shm_init_tc );
  add( echo_net_tc );
  add( peer_tc );
  add( boring_manager_tc );
  add( net_echo_tc );
  // add( net_echo2_tc );
  add( shm_finit_tc );
}

test_suite *init_unit_test_suite( int argc, char **argv )
{
  // test_suite *ts = BOOST_TEST_SUITE( "libstem test" );

  // ts->add( new stem_test_suite() );

  // ts->add( BOOST_TEST_CASE( &send_test ) );
  // ts->add( BOOST_TEST_CASE( &send2_test ) );

  // return ts;
  return new stem_test_suite();
}
