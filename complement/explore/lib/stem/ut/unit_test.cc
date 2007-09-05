// -*- C++ -*- Time-stamp: <07/09/05 01:08:18 ptr>

/*
 * Copyright (c) 2002, 2003, 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <exam/suite.h>

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
#include "Convert.h"

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

    static xmt::Thread::ret_t thr1( void * );
    static xmt::Thread::ret_t thr1new( void * );

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
  
  xmt::Thread t1( thr1 );

  EXAM_CHECK( t1.join() == 0 );

  node.wait();

  EXAM_CHECK( node.v == 1 );

  return EXAM_RESULT;
}

xmt::Thread::ret_t stem_test::thr1( void * )
{
  Node node( 2001 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node.Send( ev );

  return 0;
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
  
  xmt::Thread t1( thr1new );

  t1.join();

  node->wait();
  EXAM_CHECK( node->v == 1 );
  delete node;

  return EXAM_RESULT;
}

xmt::Thread::ret_t stem_test::thr1new( void * )
{
  NewNode *node = new NewNode( 2001 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node->Send( ev );

  delete node;

  return 0;
}

int EXAM_IMPL(stem_test::dl)
{
  void *lh = dlopen( "libloadable_stem.so", RTLD_LAZY ); // Path was passed via -Wl,--rpath=

  EXAM_REQUIRE( lh != NULL );
  void *(*f)(unsigned);
  void (*g)(void *);
  void (*w)(void *);
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

  w( reinterpret_cast<void *>(node) );
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
  EXAM_CHECK( i->first.pid == getpid() );
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
    sockmgr_stream_MP<stem::NetTransport> srv( 6995 );
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

    node.wait();
    
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
xmt::allocator_shm<xmt::__condition<true>,0> shm_cnd;
xmt::allocator_shm<xmt::__barrier<true>,0>   shm_b;

stem_test::stem_test()
{
  try {
    seg.allocate( fname, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
  }
  catch ( const xmt::shm_bad_alloc& err ) {
    EXAM_ERROR_ASYNC( err.what() );
  }
}

stem_test::~stem_test()
{
  seg.deallocate();
  unlink( fname );
}

int EXAM_IMPL(stem_test::echo_net)
{
  xmt::__condition<true>& fcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
  fcnd.set( false );

  try {
    xmt::fork();

    try {
      stem::NetTransportMgr mgr;

      fcnd.try_wait();

      stem::addr_type zero = mgr.open( "localhost", 6995 );

      EXAM_CHECK_ASYNC( zero != stem::badaddr );
      EXAM_CHECK_ASYNC( zero != 0 ); // NetTransportMgr should detect external delivery

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
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );

      srv.close();
      srv.wait();
    }
    catch ( ... ) {
    }
  }

  (&fcnd)->~__condition<true>();
  shm_cnd.deallocate( &fcnd, 1 );

  // cerr << "Fine\n";

  return EXAM_RESULT;
}

// same as echo_net(), but server in child process

int EXAM_IMPL(stem_test::net_echo)
{
  try {
    xmt::__barrier<true>& b = *new ( shm_b.allocate( 1 ) ) xmt::__barrier<true>();
    xmt::__condition<true>& c = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();

    c.set( false );

    try {
      xmt::fork();

      // server part
      {
        std::sockmgr_stream_MP<stem::NetTransport> srv( 6995 );
        StEMecho echo( 0, "echo service");

        // echo.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch );
        // echo.manager()->settrs( &std::cerr );

        EXAM_CHECK_ASYNC( srv.good() );
        c.set( true ); // ok, server listen

        b.wait(); // server may go away

        srv.close();
        srv.wait();
      }

      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      // client part

      stem::NetTransportMgr mgr;
      // mgr.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
      // mgr.manager()->settrs( &std::cerr );

      c.try_wait(); // wait server start

      stem::addr_type zero = mgr.open( "localhost", 6995 );

      EXAM_REQUIRE( mgr.good() );
      EXAM_REQUIRE( zero != stem::badaddr );

      EchoClient node;
      stem::Event ev( NODE_EV_ECHO );
      ev.dest( zero );

      ev.value() = node.mess;
      node.Send( ev );

      node.wait();

      mgr.close();
      mgr.join();

      b.wait(); // server may go away

      int stat;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }

    (&c)->~__condition<true>();
    shm_cnd.deallocate( &c, 1 );
    (&b)->~__barrier<true>();
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

  xmt::__condition<true>& fcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
  fcnd.set( false );

  xmt::__condition<true>& pcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
  pcnd.set( false );

  xmt::__condition<true>& scnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
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

      EXAM_CHECK_ASYNC( zero != stem::badaddr );
      EXAM_CHECK_ASYNC( zero != 0 );
      EXAM_CHECK_ASYNC( zero & stem::extbit ); // "external" address

      stem::Event ev( NODE_EV_REGME );
      ev.dest( zero );

      ev.value() = "c1@here";
      c1.Send( ev ); // 'register' c1 client on 'echo' server

      stem::gaddr_type ga( c1.manager()->reflect( zero ) );

      EXAM_CHECK_ASYNC( ga.addr == 0 );
      EXAM_CHECK_ASYNC( ga.pid != -1 );

      ga.addr = stem::ns_addr; // this will be global address of ns of the same process
                               // as zero

      EXAM_CHECK_ASYNC( c1.manager()->reflect( ga ) != stem::badaddr );

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

      EXAM_CHECK_ASYNC( i != nm.lst.end() );
      EXAM_CHECK_ASYNC( i->second == "c2@here" );

      stem::addr_type pa = c1.manager()->reflect( i->first );
      if ( pa == stem::badaddr ) { // unknown yet
        pa = c1.manager()->SubscribeRemote( i->first, i->second );
        if ( pa == stem::badaddr ) { // it still unknown, transport not found
          // hint: use transport as object zero used
          pa = c1.manager()->SubscribeRemote( c1.manager()->transport( zero ), i->first, i->second );
        }
      }

      EXAM_CHECK_ASYNC( pa != stem::badaddr );

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

      EXAM_CHECK_ASYNC( zero != stem::badaddr );
      EXAM_CHECK_ASYNC( zero != 0 );
      EXAM_CHECK_ASYNC( zero & stem::extbit ); // "external" address

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

  (&fcnd)->~__condition<true>();
  shm_cnd.deallocate( &fcnd, 1 );
  (&pcnd)->~__condition<true>();
  shm_cnd.deallocate( &pcnd, 1 );
  (&scnd)->~__condition<true>();
  shm_cnd.deallocate( &scnd, 1 );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::boring_manager)
{
  xmt::__condition<true>& fcnd = *new ( shm_cnd.allocate( 1 ) ) xmt::__condition<true>();
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

  (&fcnd)->~__condition<true>();
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

  conv.wait();

  EXAM_CHECK( conv.v == -1 );

  stem::Event_base<mess> ev1( CONV_EV1 );

  ev1.dest( conv.self_id() );
  ev1.value() = m;

  conv.Send( ev1 );

  conv.wait();

  EXAM_CHECK( conv.v == 1 );

  stem::Event_base<mess> ev2( CONV_EV2 );

  ev2.dest( conv.self_id() );
  ev2.value() = m;

  conv.Send( ev2 );

  conv.wait();

  EXAM_CHECK( conv.v == 2 );
  EXAM_CHECK( conv.m2 == "hello" );

  stem::Event_base<mess> ev3( CONV_EV3 );

  ev3.dest( conv.self_id() );
  ev3.value().super_id = 3;
  ev3.value().message = ", wold!";

  conv.Send( ev3 );

  conv.wait();

  EXAM_CHECK( conv.v == 3 );
  EXAM_CHECK( conv.m3 == ", wold!" );

  return EXAM_RESULT;
}

// -----------------

// -----------------

int EXAM_DECL(stem_test_suite);

int EXAM_IMPL(stem_test_suite)
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

  return t.girdle();
}

int main( int, char ** )
{
  return stem_test_suite(0);
}
