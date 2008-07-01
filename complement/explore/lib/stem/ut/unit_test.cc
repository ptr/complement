// -*- C++ -*- Time-stamp: <08/06/30 19:19:39 yeti>

/*
 * Copyright (c) 2002, 2003, 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <exam/suite.h>

#include <iostream>
#include <mt/thread>
#include <mt/uid.h>
#include <mt/shm.h>

#include <stem/EventHandler.h>
#include <stem/Names.h>
#include <stem/EDSEv.h>

#include "Node.h"
#include "NodeDL.h"
#include "NameService.h"

#include <dlfcn.h>

#include "Echo.h"
#include "Convert.h"

#include <stem/NetTransport.h>
#include <stem/EvManager.h>
#include <sockios/socksrv.h>

#ifndef STLPORT
#include <ext/functional>
using namespace __gnu_cxx;
#endif

#include <sys/wait.h>

#include <signal.h>

#include <misc/opts.h>

using namespace std;
using namespace std::tr2;

class stem_test
{
  public:
    stem_test();
    ~stem_test();

    int EXAM_DECL(basic1);
    int EXAM_DECL(basic2);
    int EXAM_DECL(basic1new);
    int EXAM_DECL(basic2new);
    int EXAM_DECL(dl);
    int EXAM_DECL(ns);

    int EXAM_DECL(echo);
    int EXAM_DECL(echo_net);
    int EXAM_DECL(net_echo);
    int EXAM_DECL(peer);
    int EXAM_DECL(boring_manager);
    int EXAM_DECL(convert);

    static void thr1();
    static void thr1new();

  private:
};

int EXAM_IMPL(stem_test::basic1)
{
  Node node( 2000 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node.Send( ev );

  node.wait();
  EXAM_CHECK( node.v == 1 );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::basic2)
{
  Node node( 2000 );
  
  thread t1( thr1 );

  t1.join();

  node.wait();

  EXAM_CHECK( node.v == 1 );

  return EXAM_RESULT;
}

void stem_test::thr1()
{
  Node node( 2001 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node.Send( ev );
}

int EXAM_IMPL(stem_test::basic1new)
{
  NewNode *node = new NewNode( 2000 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node->Send( ev );

  node->wait();

  EXAM_CHECK( node->v == 1 );
  delete node;

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::basic2new)
{
  NewNode *node = new NewNode( 2000 );
  
  thread t1( thr1new );

  t1.join();

  node->wait();
  EXAM_CHECK( node->v == 1 );
  delete node;

  return EXAM_RESULT;
}

void stem_test::thr1new()
{
  NewNode *node = new NewNode( 2001 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node->Send( ev );

  delete node;
}

int EXAM_IMPL(stem_test::dl)
{
  void *lh = dlopen( "libloadable_stem.so", RTLD_LAZY ); // Path was passed via -Wl,--rpath=

  EXAM_REQUIRE( lh != NULL );
  void *(*f)(unsigned);
  void (*g)(void *);
  int (*w)(void *);
  int (*v)(void *);

  *(void **)(&f) = dlsym( lh, "create_NewNodeDL" );
  EXAM_REQUIRE( f != NULL );
  *(void **)(&g) = dlsym( lh, "destroy_NewNodeDL" );
  EXAM_REQUIRE( g != NULL );
  *(void **)(&w) = dlsym( lh, "wait_NewNodeDL" );
  EXAM_REQUIRE( w != NULL );
  *(void **)(&v) = dlsym( lh, "v_NewNodeDL" );
  EXAM_REQUIRE( v != NULL );

  NewNodeDL *node = reinterpret_cast<NewNodeDL *>( f( 2002 )  );
  stem::Event ev( NODE_EV2 );
  ev.dest( 2002 );
  node->Send( ev );

  EXAM_CHECK( w( reinterpret_cast<void *>(node) ) == 1 );
  EXAM_CHECK( v(reinterpret_cast<void *>(node)) == 1 );

  g( reinterpret_cast<void *>(node) );
  dlclose( lh );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::ns)
{
  Node node( 2003, "Node" );
  Naming nm;

  stem::Event ev( EV_STEM_GET_NS_LIST );
  ev.dest( stem::ns_addr );
  nm.Send( ev );

  nm.wait();

  // this is sample of all inline find:
  Naming::nsrecords_type::const_iterator i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( equal_to<string>(), string( "ns" ) ), select2nd<pair<stem::gaddr_type,string> >() ) );

  EXAM_CHECK( i != nm.lst.end() );
  EXAM_CHECK( i->second == "ns" );
  EXAM_CHECK( i->first.hid == xmt::hostid() );
  EXAM_CHECK( i->first.pid == std::tr2::getpid() );
  EXAM_CHECK( i->first.addr == stem::ns_addr );

  // well, but for few seaches declare and reuse functors:
  equal_to<string> eq;
  equal_to<stem::gaddr_type> eqa;

  select1st<pair<stem::gaddr_type,string> > first;
  select2nd<pair<stem::gaddr_type,string> > second;

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eq, string( "Node" ) ), second ) );

  EXAM_CHECK( i != nm.lst.end() );
  EXAM_CHECK( i->second == "Node" );
  EXAM_CHECK( i->first.addr == 2003 );

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eqa, nm.self_glid() ), first ) );

  EXAM_CHECK( i != nm.lst.end() );
  EXAM_CHECK( i->first == nm.self_glid() );
  EXAM_CHECK( i->second.length() == 0 );

  nm.lst.clear();
  nm.reset();

  EXAM_CHECK( nm.lst.empty() );

  stem::Event evname( EV_STEM_GET_NS_NAME );
  evname.dest( stem::ns_addr );
  evname.value() = "Node";
  nm.Send( evname );

  nm.wait();

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eq, string( "ns" ) ), second ) );

  EXAM_CHECK( i == nm.lst.end() );

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eq, string( "Node" ) ), second ) );

  EXAM_CHECK( i != nm.lst.end() );
  EXAM_CHECK( i->second == "Node" );
  EXAM_CHECK( i->first.addr == 2003 );

  i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eqa, nm.self_glid() ), first ) );

  EXAM_CHECK( i == nm.lst.end() );

  nm.lst.clear();
  nm.reset();

  EXAM_CHECK( nm.lst.empty() );

  evname.value() = "No-such-name";
  nm.Send( evname );

  nm.wait();

  EXAM_CHECK( nm.lst.empty() );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::echo)
{
  try {
    connect_processor<stem::NetTransport> srv( 6995 );
    stem::NetTransportMgr mgr;

    StEMecho echo( 0, "echo service"); // <= zero!

    stem::addr_type zero = mgr.open( "localhost", 6995 );

    EXAM_CHECK( zero != stem::badaddr );
    EXAM_CHECK( zero == 0 ); // NetTransportMgr should detect local delivery

    EchoClient node;
    
    stem::Event ev( NODE_EV_ECHO );

    ev.dest( zero );
    ev.value() = node.mess;

    node.Send( ev );

    EXAM_CHECK( node.wait() );
    
    mgr.close();
    mgr.join();

    srv.close();
    srv.wait();
  }
  catch ( ... ) {
  }

  return EXAM_RESULT;
}

const char fname[] = "/tmp/stem_test.shm";
xmt::shm_alloc<0> seg;
xmt::allocator_shm<condition_event_ip,0> shm_cnd;
xmt::allocator_shm<barrier_ip,0>   shm_b;

stem_test::stem_test()
{
  try {
    seg.allocate( fname, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
  }
  catch ( const xmt::shm_bad_alloc& err ) {
    try {
      seg.allocate( fname, 4*4096, 0, 0600 );
    }
    catch ( const xmt::shm_bad_alloc& err2 ) {
      string s = err.what();
      s += "; ";
      s += err2.what();
      EXAM_ERROR_ASYNC( s.c_str() );
    }
    // EXAM_ERROR_ASYNC( err.what() );
  }
}

stem_test::~stem_test()
{
  seg.deallocate();
  unlink( fname );
}

int EXAM_IMPL(stem_test::echo_net)
{
  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();

  try {
    std::tr2::this_thread::fork();

    int eflag = 0;

    try {
      stem::NetTransportMgr mgr;

      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      stem::addr_type zero = mgr.open( "localhost", 6995 );

      EXAM_CHECK_ASYNC_F( zero != stem::badaddr, eflag );
      EXAM_CHECK_ASYNC_F( zero != 0, eflag ); // NetTransportMgr should detect external delivery

      EchoClient node;
    
      stem::Event ev( NODE_EV_ECHO );

      ev.dest( zero );
      ev.value() = node.mess;

      node.Send( ev );

      EXAM_CHECK_ASYNC_F( node.wait(), eflag );
    
      mgr.close();
      mgr.join();
    }
    catch ( ... ) {
    }

    exit( eflag );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    try {
      connect_processor<stem::NetTransport> srv( 6995 );

      StEMecho echo( 0, "echo service"); // <= zero!

      fcnd.notify_one();

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }

      srv.close();
      srv.wait();
    }
    catch ( ... ) {
    }
  }

  (&fcnd)->~condition_event_ip();
  shm_cnd.deallocate( &fcnd, 1 );

  // cerr << "Fine\n";

  return EXAM_RESULT;
}

// same as echo_net(), but server in child process

int EXAM_IMPL(stem_test::net_echo)
{
  try {
    barrier_ip& b = *new ( shm_b.allocate( 1 ) ) barrier_ip();
    condition_event_ip& c = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();

    try {
      std::tr2::this_thread::fork();

      int eflag = 0;
      // server part
      {
        connect_processor<stem::NetTransport> srv( 6995 );
        StEMecho echo( 0, "echo service");

        // echo.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch );
        // echo.manager()->settrs( &std::cerr );

        EXAM_CHECK_ASYNC_F( srv.good(), eflag );
        c.notify_one(); // ok, server listen

        b.wait(); // server may go away

        srv.close();
        srv.wait();
      }

      exit( eflag );
    }
    catch ( std::tr2::fork_in_parent& child ) {
      // client part

      stem::NetTransportMgr mgr;
      // mgr.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
      // mgr.manager()->settrs( &std::cerr );

      EXAM_CHECK( c.timed_wait( std::tr2::milliseconds( 800 ) ) ); // wait server start

      stem::addr_type zero = mgr.open( "localhost", 6995 );

      EXAM_REQUIRE( mgr.good() );
      EXAM_REQUIRE( zero != stem::badaddr );

      EchoClient node;
      stem::Event ev( NODE_EV_ECHO );
      ev.dest( zero );

      ev.value() = node.mess;
      node.Send( ev );

      EXAM_CHECK( node.wait() );

      mgr.close();
      mgr.join();

      b.wait(); // server may go away

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    (&c)->~condition_event_ip();
    shm_cnd.deallocate( &c, 1 );
    (&b)->~barrier_ip();
    shm_b.deallocate( &b, 1 );
  }
  catch (  xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

extern "C" {

static void dummy_signal_handler( int )
{ }

}

int EXAM_IMPL(stem_test::peer)
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

  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();
  condition_event_ip& pcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();
  condition_event_ip& scnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();

  try {
    // Client 1
    std::tr2::this_thread::fork();
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

    int eflag = 0;

    try {
      stem::NetTransportMgr mgr;

      PeerClient c1( "c1 local" );  // c1 client
      Naming nm;

      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      stem::addr_type zero = mgr.open( "localhost", 6995 ); // take address of 'zero' (aka default) object via net transport from server
      // It done like it should on client side

      EXAM_CHECK_ASYNC_F( zero != stem::badaddr, eflag );
      EXAM_CHECK_ASYNC_F( zero != 0, eflag );
      EXAM_CHECK_ASYNC_F( zero & stem::extbit, eflag ); // "external" address

      stem::Event ev( NODE_EV_REGME );
      ev.dest( zero );

      ev.value() = "c1@here";
      c1.Send( ev ); // 'register' c1 client on 'echo' server

      stem::gaddr_type ga( c1.manager()->reflect( zero ) );

      EXAM_CHECK_ASYNC_F( ga.addr == 0, eflag );
      EXAM_CHECK_ASYNC_F( ga.pid != -1, eflag );

      ga.addr = stem::ns_addr; // this will be global address of ns of the same process
                               // as zero

      EXAM_CHECK_ASYNC_F( c1.manager()->reflect( ga ) != stem::badaddr, eflag );

      stem::Event evname( EV_STEM_GET_NS_NAME );
      evname.dest( c1.manager()->reflect( ga ) );
      evname.value() = "c2@here";

      EXAM_CHECK_ASYNC_F( pcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      Naming::nsrecords_type::const_iterator i;

      do {
        nm.reset();
        nm.lst.clear();
        nm.Send( evname );
        nm.wait();
        i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( equal_to<string>(), string( "c2@here" ) ), select2nd<pair<stem::gaddr_type,string> >() ) );
      } while ( i == nm.lst.end() );

      EXAM_CHECK_ASYNC_F( i != nm.lst.end(), eflag );
      EXAM_CHECK_ASYNC_F( i->second == "c2@here", eflag );

      stem::addr_type pa = c1.manager()->reflect( i->first );
      if ( pa == stem::badaddr ) { // unknown yet
        pa = c1.manager()->SubscribeRemote( i->first, i->second );
        if ( pa == stem::badaddr ) { // it still unknown, transport not found
          // hint: use transport as object zero used
          pa = c1.manager()->SubscribeRemote( c1.manager()->transport( zero ), i->first, i->second );
        }
      }

      EXAM_CHECK_ASYNC_F( pa != stem::badaddr, eflag );

      if ( pa != stem::badaddr ) {
        stem::Event pe( NODE_EV_ECHO );
        pe.dest( pa );
        pe.value() = "c2 local"; // <<-- mess is like name ... |
                                 //                            .
        c1.Send( pe );
      }

      EXAM_CHECK_ASYNC_F( scnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      mgr.close();
      mgr.join();
    }
    catch ( ... ) {
      eflag = 2;
    }

    exit( eflag );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    fpid = child.pid();
  }

  try {
    // Client 2
    std::tr2::this_thread::fork();

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

    int eflag = 0;

    try {
      stem::NetTransportMgr mgr;
                                   //                                          ^
      PeerClient c2( "c2 local" ); // <<--- name the same as mess expected ... |

      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );
      stem::addr_type zero = mgr.open( "localhost", 6995 ); // take address of 'zero' (aka default) object via net transport from server
      // It done like it should on client side

      EXAM_CHECK_ASYNC_F( zero != stem::badaddr, eflag );
      EXAM_CHECK_ASYNC_F( zero != 0, eflag );
      EXAM_CHECK_ASYNC_F( zero & stem::extbit, eflag ); // "external" address

      stem::Event ev( NODE_EV_REGME );
      ev.dest( zero );

      ev.value() = "c2@here";
      c2.Send( ev ); // 'register' c2 client on 'echo' server

      pcnd.notify_one();

      c2.wait();
      // cerr << "Fine!" << endl;
      scnd.notify_one();

      mgr.close();
      mgr.join();
    }
    catch ( ... ) {
      eflag = 2;
    }

    exit( eflag );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    connect_processor<stem::NetTransport> srv( 6995 ); // server, it serve 'echo'
    StEMecho echo( 0, "echo service"); // <= zero! 'echo' server, default ('zero' address)

    fcnd.notify_all();

    int stat = -1;
    EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    if ( WIFEXITED(stat) ) {
      EXAM_CHECK( WEXITSTATUS(stat) == 0 );
    } else {
      EXAM_ERROR( "child interrupted" );
    }
    stat = -1;
    EXAM_CHECK( waitpid( fpid, &stat, 0 ) == fpid );
    if ( WIFEXITED(stat) ) {
      EXAM_CHECK( WEXITSTATUS(stat) == 0 );
    } else {
      EXAM_ERROR( "child interrupted" );
    }

    srv.close();
    srv.wait();
  }

  (&fcnd)->~condition_event_ip();
  shm_cnd.deallocate( &fcnd, 1 );
  (&pcnd)->~condition_event_ip();
  shm_cnd.deallocate( &pcnd, 1 );
  (&scnd)->~condition_event_ip();
  shm_cnd.deallocate( &scnd, 1 );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::boring_manager)
{
  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();

  try {
    // Client
    std::tr2::this_thread::fork();

    int eflag = 0;

    try {
      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      for ( int i = 0; i < 10; ++i ) {
        const int n = 10;
        stem::NetTransportMgr *mgr[n];
        mgr[0] = new stem::NetTransportMgr;
        mgr[0]->open( "localhost", 6995 );

        for ( int j = 1; j < n; ++j ) {
          mgr[j] = new stem::NetTransportMgr;
          stem::addr_type a = mgr[j]->open( "localhost", 6995 );
          EXAM_CHECK_ASYNC_F( a != stem::badaddr, eflag );
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
      eflag = 2;
    }
    exit( eflag );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    connect_processor<stem::NetTransport> srv( 6995 ); // server, it serve 'echo'
    StEMecho echo( 0, "echo service"); // <= zero! 'echo' server, default ('zero' address)

    fcnd.notify_all();

    int stat = -1;
    EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
    } else {
      EXAM_ERROR( "child interrupted" );
    }

    srv.close();
    srv.wait();
  }

  (&fcnd)->~condition_event_ip();
  shm_cnd.deallocate( &fcnd, 1 );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::convert)
{
  Convert conv;
  mess m;

  m.super_id = 2;
  m.message = "hello";

  stem::Event_base<mess> ev( CONV_EV0 );

  ev.dest( conv.self_id() );
  ev.value() = m;

  conv.Send( ev );

  EXAM_CHECK( conv.wait() );
  EXAM_CHECK( conv.v == -1 );

  conv.v = 0;

  stem::Event_base<mess> ev1( CONV_EV1 );

  ev1.dest( conv.self_id() );
  ev1.value() = m;

  conv.Send( ev1 );

  EXAM_CHECK( conv.wait() );
  EXAM_CHECK( conv.v == 1 );

  conv.v = 0;

  stem::Event_base<mess> ev2( CONV_EV2 );

  ev2.dest( conv.self_id() );
  ev2.value() = m;

  conv.Send( ev2 );

  EXAM_CHECK( conv.wait() );

  EXAM_CHECK( conv.v == 2 );
  EXAM_CHECK( conv.m2 == "hello" );

  conv.v = 0;

  stem::Event_base<mess> ev3( CONV_EV3 );

  ev3.dest( conv.self_id() );
  ev3.value().super_id = 3;
  ev3.value().message = ", wold!";

  conv.Send( ev3 );

  EXAM_CHECK( conv.wait() );

  EXAM_CHECK( conv.v == 3 );
  EXAM_CHECK( conv.m3 == ", wold!" );

  conv.v = 0;

  return EXAM_RESULT;
}

int main( int argc, const char** argv )
{
  exam::test_suite::test_case_type tc[4];

  exam::test_suite t( "libstem test suite" );
  stem_test test;

  tc[1] = t.add( &stem_test::basic2, test, "basic2",
                 tc[0] = t.add( &stem_test::basic1, test, "basic1" ) );

  tc[2] = t.add( &stem_test::basic1new, test, "basic1new", tc[0] );  

  t.add( &stem_test::dl, test, "dl",
         t.add( &stem_test::basic2new, test, "basic2new", tc + 1, tc + 3 ) );
  t.add( &stem_test::ns, test, "ns", tc[0] );

  t.add( &stem_test::net_echo, test, "net echo",
         t.add( &stem_test::echo_net, test, "echo_net",
                tc[3] = t.add( &stem_test::echo, test, "echo", tc[1] ) ) );

  t.add( &stem_test::boring_manager, test, "boring_manager",
         t.add( &stem_test::peer, test, "peer", tc[3] ) );

  t.add( &stem_test::convert, test, "convert", tc[0] );

  Opts opts;

  opts.description( "test suite for 'StEM' framework" );
  opts.usage( "[options]" );

  opts << option<bool>( "print this help message", 'h', "help" )
       << option<bool>( "list all test cases", 'l', "list" )
       << option<string>( "run tests by number", 'r', "run" )["0"]
       << option<bool>( "print status of tests within test suite", 'v', "verbose" )
       << option<bool>(  "trace checks", 't', "trace" );

  try {
    opts.parse( argc, argv );
  }
  catch (...) {
    opts.help( cerr );
    return 1;
  }

  if ( opts.is_set( 'h' ) ) {
    opts.help( cerr );
    return 0;
  }

  if ( opts.is_set( 'l' ) ) {
    t.print_graph( cerr );
    return 0;
  }

  if ( opts.is_set( 'v' ) ) {
    t.flags( t.flags() | exam::base_logger::verbose );
  }

  if ( opts.is_set( 't' ) ) {
    t.flags( t.flags() | exam::base_logger::trace );
  }

  if ( opts.is_set( 'r' ) ) {
    stringstream ss( opts.get<string>( 'r' ) );
    int n;
    while ( ss >> n ) {
      t.single( n );
    }

    return 0;
  }

  return t.girdle();
}
