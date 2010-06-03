// -*- C++ -*- Time-stamp: <10/05/25 11:56:13 ptr>

/*
 * Copyright (c) 2002, 2003, 2006-2009
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
#include "Cron.h"
#include "vf.h"

#include <stem/NetTransport.h>
#include <stem/EvManager.h>
#include <stem/Cron.h>
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
    int EXAM_DECL(weight);
    int EXAM_DECL(dl);
    int EXAM_DECL(ns);

    int EXAM_DECL(echo);
    int EXAM_DECL(echo_net);
    int EXAM_DECL(echo_local);
    int EXAM_DECL(net_echo);
    int EXAM_DECL(triple_echo);
    int EXAM_DECL(ugly_echo_net);
    int EXAM_DECL(peer);
    int EXAM_DECL(last_event);
    int EXAM_DECL(boring_manager);
    int EXAM_DECL(boring_manager_more);
    int EXAM_DECL(convert);
    int EXAM_DECL(cron);
    int EXAM_DECL(vf);
    int EXAM_DECL(vf1);
    int EXAM_DECL(command_mgr);

    int EXAM_DECL(route_to_net);
    int EXAM_DECL(route_from_net);

    static void thr1();
    static void thr1new();

  private:
};

int EXAM_IMPL(stem_test::basic1)
{
  stem::addr_type addr = xmt::uid();

  Node node( addr );

  stem::stem_scope scope( node );

  stem::Event ev( NODE_EV1 );

  ev.dest( addr );
  node.Send( ev );

  node.wait();
  EXAM_CHECK( node.v == 1 );

  return EXAM_RESULT;
}

stem::addr_type exchange_addr = xmt::uid();

int EXAM_IMPL(stem_test::basic2)
{
  Node node( exchange_addr );
  
  stem::stem_scope scope( node );

  thread t1( thr1 );

  t1.join();

  node.wait();

  EXAM_CHECK( node.v == 1 );

  return EXAM_RESULT;
}

void stem_test::thr1()
{
  Node node( xmt::uid() );

  stem::stem_scope scope( node );

  stem::Event ev( NODE_EV1 );

  ev.dest( exchange_addr );
  node.Send( ev );
}

int EXAM_IMPL(stem_test::basic1new)
{
  stem::addr_type addr = xmt::uid();

  NewNode *node = new NewNode( addr );

  {
    stem::stem_scope scope( *node );

    stem::Event ev( NODE_EV1 );

    ev.dest( addr );
    node->Send( ev );

    node->wait();

    EXAM_CHECK( node->v == 1 );
  }

  delete node;

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::basic2new)
{
  NewNode *node = new NewNode( exchange_addr );
  
  {
    stem::stem_scope scope( *node );

    thread t1( thr1new );

    t1.join();

    node->wait();
    EXAM_CHECK( node->v == 1 );
  }

  delete node;

  return EXAM_RESULT;
}

void stem_test::thr1new()
{
  NewNode *node = new NewNode( xmt::uid() );

  {
    stem::stem_scope scope( *node );

    stem::Event ev( NODE_EV1 );

    ev.dest( exchange_addr );
    node->Send( ev );
  }

  delete node;
}

int EXAM_IMPL(stem_test::weight)
{
  stem::addr_type addr = xmt::uid();

  Node node( addr );          // <- The same address
  Node node_nice( addr, -1 ); // <- but different weight

  {
    stem::stem_scope scope( node );
    stem::stem_scope scope_nice( node_nice );

    // This event should receive node_nice, but not node:
    stem::Event ev( NODE_EV1 );

    ev.dest( addr );
    node.Send( ev );

    EXAM_CHECK( node_nice.wait() );
    EXAM_CHECK( node_nice.v == 1 );

    EXAM_CHECK( !node.wait() );
    EXAM_CHECK( node.v == 0 );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::dl)
{
  void *lh = dlopen( "libloadable_stem.so", RTLD_LAZY ); // Path was passed via -Wl,--rpath=

  EXAM_REQUIRE( lh != NULL );
  void *(*f)(stem::addr_type);
  void (*g)(void *);
  int (*w)(void *);
  int (*v)(void *);

  // *(void **)(&f) = dlsym( lh, "create_NewNodeDL" );
  f = reinterpret_cast<void *(*)(stem::addr_type)>(dlsym( lh, "create_NewNodeDL" ));
  EXAM_REQUIRE( f != NULL );
  // *(void **)(&g) = dlsym( lh, "destroy_NewNodeDL" );
  g = reinterpret_cast<void (*)(void *)>(dlsym( lh, "destroy_NewNodeDL" ));
  EXAM_REQUIRE( g != NULL );
  // *(void **)(&w) = dlsym( lh, "wait_NewNodeDL" );
  w = reinterpret_cast<int (*)(void *)>(dlsym( lh, "wait_NewNodeDL" ));
  EXAM_REQUIRE( w != NULL );
  // *(void **)(&v) = dlsym( lh, "v_NewNodeDL" );
  v = reinterpret_cast<int (*)(void *)>(dlsym( lh, "v_NewNodeDL" ));
  EXAM_REQUIRE( v != NULL );

  stem::addr_type addr = xmt::uid();

  NewNodeDL *node = reinterpret_cast<NewNodeDL *>( f( addr )  );

  {
    stem::stem_scope scope( *node );

    stem::Event ev( NODE_EV2 );
    ev.dest( addr );
    node->Send( ev );

    EXAM_CHECK( w( reinterpret_cast<void *>(node) ) == 1 );
    EXAM_CHECK( v(reinterpret_cast<void *>(node)) == 1 );
  }

  g( reinterpret_cast<void *>(node) );
  dlclose( lh );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::ns)
{
  stem::addr_type addr = xmt::uid();

  Node node( addr, "Node" );
  Naming nm;

  {
    stem::stem_scope scope( node );
    stem::stem_scope nm_scope( nm );

    stem::Event ev( EV_STEM_GET_NS_LIST );
    ev.dest( stem::EventHandler::ns() );
    nm.Send( ev );

    nm.wait();

    // this is sample of all inline find:
    Naming::nsrecords_type::const_iterator i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( equal_to<string>(), string( "ns" ) ), select2nd<pair<stem::addr_type,string> >() ) );

    EXAM_CHECK( i != nm.lst.end() );
    EXAM_CHECK( i->second == "ns" );
    EXAM_CHECK( i->first == stem::EventHandler::ns() );

    // well, but for few seaches declare and reuse functors:
    equal_to<string> eq;
    equal_to<stem::addr_type> eqa;

    select1st<pair<stem::addr_type,string> > first;
    select2nd<pair<stem::addr_type,string> > second;

    // find address of object with "Node" annotation
    i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eq, string( "Node" ) ), second ) );

    EXAM_CHECK( i != nm.lst.end() );
    EXAM_CHECK( i->second == "Node" );
    EXAM_CHECK( i->first == addr );
    EXAM_CHECK( i->first == node.self_id() );

    // Try to find record for nm...
    i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eqa, nm.self_id() ), first ) );
    // ... no such record, because nm has empty annotation:
    EXAM_CHECK( i == nm.lst.end() );

    nm.lst.clear();
    nm.reset();

    EXAM_CHECK( nm.lst.empty() );

    stem::Event evname( EV_STEM_GET_NS_NAME );
    evname.dest( stem::EventHandler::ns() );
    evname.value() = "Node";
    nm.Send( evname );

    nm.wait();

    i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eq, string( "ns" ) ), second ) );

    EXAM_CHECK( i == nm.lst.end() );

    i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eq, string( "Node" ) ), second ) );

    EXAM_CHECK( i != nm.lst.end() );
    EXAM_CHECK( i->second == "Node" );
    EXAM_CHECK( i->first == addr );

    i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( eqa, nm.self_id() ), first ) );

    EXAM_CHECK( i == nm.lst.end() );

    nm.lst.clear();
    nm.reset();

    EXAM_CHECK( nm.lst.empty() );

    evname.value() = "No-such-name";
    nm.Send( evname );

    nm.wait();

    EXAM_CHECK( nm.lst.empty() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::echo)
{
  try {
    connect_processor<stem::NetTransport> srv( 6995 );
    stem::NetTransportMgr mgr;

    stem::addr_type addr = xmt::uid();

    StEMecho echo( addr, "echo service");
    {
      stem::stem_scope scope( echo );

      stem::addr_type remote_default = mgr.open( "localhost", 6995 );

      EXAM_CHECK( remote_default == stem::badaddr ); // not assigned default
      // EXAM_CHECK( remote_ns != stem::badaddr );
      // EXAM_CHECK( remote_ns == stem::EventHandler::ns() ); // Name service of this StEM process

      EchoClient node;

      stem::stem_scope scope_node( node );
   
      stem::Event ev( NODE_EV_ECHO );

      ev.dest( addr );
      ev.value() = node.mess;

      node.Send( ev );

      EXAM_CHECK( node.wait() );
    
      mgr.close();
      mgr.join();
    }

    srv.close();
    srv.wait();
  }
  catch ( ... ) {
  }

  return EXAM_RESULT;
}

xmt::shm_alloc<0> seg;
xmt::allocator_shm<condition_event_ip,0> shm_cnd;
xmt::allocator_shm<barrier_ip,0>         shm_b;
xmt::allocator_shm<stem::addr_type,0>    shm_a;

stem_test::stem_test()
{
  try {
    seg.allocate( 70000, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
  }
  catch ( const xmt::shm_bad_alloc& err ) {
    try {
      seg.allocate( 70000, 4*4096, 0, 0600 );
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
}

int EXAM_IMPL(stem_test::echo_net)
{
  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();
  stem::addr_type& addr = *new ( shm_a.allocate( 1 ) ) stem::addr_type();

  try {
    std::tr2::this_thread::fork();

    int eflag = 0;

    try {
      stem::NetTransportMgr mgr;

      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      stem::addr_type def_object = mgr.open( "localhost", 6995 );

      EXAM_CHECK_ASYNC_F( def_object != stem::badaddr, eflag );
      EXAM_CHECK_ASYNC_F( def_object == addr, eflag );

      EchoClient node;
      {
        stem::stem_scope scope( node );
    
        stem::Event ev( NODE_EV_ECHO );

        ev.dest( addr );
        ev.value() = node.mess;

        // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
        // stem::EventHandler::manager()->settrs( &std::cerr );

        node.Send( ev );

        EXAM_CHECK_ASYNC_F( node.wait(), eflag );
      }

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

      StEMecho echo( "echo service" );

      {
        stem::stem_scope scope( echo );

        addr = echo.self_id();
        echo.set_default(); // become default object

        // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
        // stem::EventHandler::manager()->settrs( &std::cerr );

        fcnd.notify_one();

        int stat = -1;
        EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );

        if ( WIFEXITED(stat) ) {
          EXAM_CHECK( WEXITSTATUS(stat) == 0 );
        } else {
          EXAM_ERROR( "child interrupted" );
        }
      }

      srv.close();
      srv.wait();
    }
    catch ( ... ) {
    }
  }

  (&fcnd)->~condition_event_ip();
  shm_cnd.deallocate( &fcnd, 1 );
  shm_a.deallocate( &addr, 1 );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::echo_local)
{
  // throw exam::skip_exception();

  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();
  stem::addr_type& addr = *new ( shm_a.allocate( 1 ) ) stem::addr_type();

  const char f[] = "/tmp/stem_echo";

  try {
    std::tr2::this_thread::fork();

    int eflag = 0;

    try {
      stem::NetTransportMgr mgr;

      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      stem::addr_type def_object = mgr.open( f /* , sock_base::sock_stream */ );

      EXAM_CHECK_ASYNC_F( def_object != stem::badaddr, eflag );
      EXAM_CHECK_ASYNC_F( def_object == addr, eflag );

      EchoClient node;
      {
        stem::stem_scope scope( node );
    
        stem::Event ev( NODE_EV_ECHO );

        ev.dest( addr );
        ev.value() = node.mess;

        // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
        // stem::EventHandler::manager()->settrs( &std::cerr );

        node.Send( ev );

        EXAM_CHECK_ASYNC_F( node.wait(), eflag );
      }

      mgr.close();
      mgr.join();
    }
    catch ( ... ) {
    }

    exit( eflag );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    try {
      connect_processor<stem::NetTransport> srv( f );

      StEMecho echo( "echo service" );

      EXAM_CHECK( srv.good() );
      EXAM_CHECK( srv.is_open() );

      {
        stem::stem_scope scope( echo );

        addr = echo.self_id();
        echo.set_default(); // become default object

        // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
        // stem::EventHandler::manager()->settrs( &std::cerr );

        fcnd.notify_one();

        int stat = -1;
        EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );

        if ( WIFEXITED(stat) ) {
          EXAM_CHECK( WEXITSTATUS(stat) == 0 );
        } else {
          EXAM_ERROR( "child interrupted" );
        }
      }

      srv.close();
      srv.wait();
    }
    catch ( ... ) {
    }
  }

  (&fcnd)->~condition_event_ip();
  shm_cnd.deallocate( &fcnd, 1 );
  shm_a.deallocate( &addr, 1 );

  unlink( f );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::triple_echo)
{
  try {
    xmt::allocator_shm<barrier_ip,0> shm;
    xmt::allocator_shm<condition_event_ip,0> cnd;

    barrier_ip& b1 = *new ( shm.allocate( 1 ) ) barrier_ip();
    barrier_ip& b2 = *new ( shm.allocate( 1 ) ) barrier_ip();
    condition_event_ip& c = *new ( cnd.allocate(1) ) condition_event_ip();

    try {
      int res = 0;

      this_thread::fork();

      try {

        this_thread::fork();

        {
#if 0
          stem::EventHandler dummy;

          stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault | stem::EvManager::tracesubscr);
          stem::EventHandler::manager()->settrs( &std::cerr );
#endif

          // srv
          connect_processor<stem::NetTransport> srv( 6995 );

          StEMecho echo( "echo service" );
        
          stem::stem_scope scope( echo );

          echo.set_default(); // become default object       

          stem::NetTransportMgr mgr;
        
          stem::addr_type def_object = mgr.open( "localhost", 6995 );

          EXAM_CHECK_ASYNC_F( def_object != stem::badaddr, res );

          EchoClient node;

          stem::stem_scope node_scope( node );
      
          stem::Event ev( NODE_EV_ECHO );

          ev.dest( def_object );
          ev.value() = node.mess;

          // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
          // stem::EventHandler::manager()->settrs( &std::cerr );

          node.Send( ev );

          EXAM_CHECK_ASYNC_F( node.wait(), res );
        
          mgr.close();
          mgr.join();
        
          b1.wait();
        
          c.wait();

          srv.close();
          srv.wait();
        }
        exit(res);
      }
      catch( std::tr2::fork_in_parent& child ) {
        // echo client
        b1.wait();

#if 0        
        stem::EventHandler dummy;

        stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault | stem::EvManager::tracesubscr);
        stem::EventHandler::manager()->settrs( &std::cerr );
#endif

        stem::NetTransportMgr mgr;
        
        stem::addr_type def_object = mgr.open( "localhost", 6995 );

        EXAM_CHECK_ASYNC_F( def_object != stem::badaddr, res );

        EchoClient node;

        {
          stem::stem_scope scope( node );
      
          stem::Event ev( NODE_EV_ECHO );

          ev.dest( def_object );
          ev.value() = node.mess;

          // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
          // stem::EventHandler::manager()->settrs( &std::cerr );

          node.Send( ev );

          EXAM_CHECK_ASYNC_F( node.wait(), res );
        }
      
        mgr.close();
        mgr.join();

        b2.wait();

        c.wait();

        int stat = -1;
        EXAM_CHECK_ASYNC_F( waitpid( child.pid(), &stat, 0 ) == child.pid(), res );
        if ( WIFEXITED(stat) ) {
          EXAM_CHECK_ASYNC_F( WEXITSTATUS(stat) == 0, res );
        } else {
          EXAM_ERROR_ASYNC_F( "child interrupted", res );
        }

        exit(res);
      }
    }
    catch( std::tr2::fork_in_parent& child ) {
      // echo client
      b2.wait();
      
      {
        stem::NetTransportMgr mgr;
      
        stem::addr_type def_object = mgr.open( "localhost", 6995 );

        EXAM_CHECK( def_object != stem::badaddr );

        EchoClient node;

        stem::stem_scope scope( node );
   
        stem::Event ev( NODE_EV_ECHO );

        ev.dest( def_object );
        ev.value() = node.mess;
          
        // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
        // stem::EventHandler::manager()->settrs( &std::cerr );

        node.Send( ev );

        EXAM_CHECK( node.wait() );
      
        mgr.close();
        mgr.join();
      }

      c.notify_all();

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    cnd.deallocate( &c );
    shm.deallocate( &b2 );
    shm.deallocate( &b1 );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }
  
  return EXAM_RESULT;
}

// same as echo_net(), but server in child process
int EXAM_IMPL(stem_test::net_echo)
{
  try {
    barrier_ip& b = *new ( shm_b.allocate( 1 ) ) barrier_ip();
    condition_event_ip& c = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();
    stem::addr_type& addr = *new ( shm_a.allocate( 1 ) ) stem::addr_type();

    try {
      std::tr2::this_thread::fork();

      int eflag = 0;
      // server part
      {
        connect_processor<stem::NetTransport> srv( 6995 );
        StEMecho echo( "echo service" );

        stem::stem_scope scope( echo );

        addr = echo.self_id();
        echo.set_default(); // become default object

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

      stem::addr_type def_object = mgr.open( "localhost", 6995 );

      EXAM_REQUIRE( mgr.good() );
      EXAM_REQUIRE( def_object != stem::badaddr );
      EXAM_CHECK( def_object == addr );

      EchoClient node;

      stem::stem_scope scope( node );

      stem::Event ev( NODE_EV_ECHO );
      ev.dest( def_object );

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
    shm_a.deallocate( &addr, 1 );
  }
  catch (  xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::ugly_echo_net)
{
  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();

  try {
    std::tr2::this_thread::fork();

    int eflag = 0;

    try {
      stem::NetTransportMgr mgr;

      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      stem::addr_type def_object = mgr.open( "localhost", 6995 );

      EXAM_CHECK_ASYNC_F( def_object != stem::badaddr, eflag );

      UglyEchoClient node;

      {
        stem::stem_scope scope( node );

        stem::Event ev( NODE_EV_ECHO );
        ev.dest( def_object );
      
        bool ok = true;
        for ( int i = 0; i < 10000 && ok; ++i ) {
          node.lock.lock();

          node.mess.clear();
          for ( int j = 0; j < 16; ++j ) {
            node.mess += xmt::uid_str();
          }
          
          ev.value() = node.mess;
          node.lock.unlock();
        
          node.Send( ev );
              
          ok = node.wait();
        
          if ( !ok ) {
            cerr << "Failed on iteration # " << i << endl;
            break;
          }
        }
      
        EXAM_CHECK_ASYNC_F( ok, eflag );
      }
    
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

      UglyEchoSrv echo( "ugly echo service");

      {
        stem::stem_scope scope( echo );

        echo.set_default();

        fcnd.notify_one();

        int stat = -1;
        EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
        if ( WIFEXITED(stat) ) {
          EXAM_CHECK( WEXITSTATUS(stat) == 0 );
        } else {
          EXAM_ERROR( "child interrupted" );
        }
      }

      srv.close();
      srv.wait();
    }
    catch ( ... ) {
    }
  }

  (&fcnd)->~condition_event_ip();
  shm_cnd.deallocate( &fcnd, 1 );

  return EXAM_RESULT;
}

extern "C" {

static void dummy_signal_handler( int )
{ }

}

int EXAM_IMPL(stem_test::peer)
{
  /*
   * The logical scheme is:
   *
   *        / c1
   *   echo
   *        \ c2
   *
   * (c1 <-> c2, through 'echo')
   *
   * echo, c1 and c2 are in different processes.
   *
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

      {
        stem::stem_scope scope( c1 );
        stem::stem_scope nm_scope( nm );

        EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

        stem::addr_type zero = mgr.open( "localhost", 6995 ); // take address of 'zero' (aka default) object via net transport from server
        // It done like it should on client side

        EXAM_CHECK_ASYNC_F( zero != stem::badaddr, eflag );

        stem::Event ev( NODE_EV_REGME );
        ev.dest( zero );

        ev.value() = "c1@here";
        c1.Send( ev ); // 'register' c1 client on 'echo' server

        stem::Event evname( EV_STEM_GET_NS_NAME );
        Naming::nsrecords_type::const_iterator i;

        evname.dest( stem::EventHandler::ns() );
        evname.value() = "nsf"; // <- ask for "name service foreign"

        nm.reset();
        nm.lst.clear();
        nm.Send( evname );

        EXAM_CHECK_ASYNC_F( nm.wait(), eflag );
        EXAM_CHECK_ASYNC_F( !nm.lst.empty(), eflag );

        if ( !nm.lst.empty() ) {
          EXAM_CHECK_ASYNC_F( nm.lst.begin()->first != stem::badaddr, eflag );
          EXAM_CHECK_ASYNC_F( nm.lst.begin()->second == "nsf", eflag );

          // Ask NS of process with c2 about address of "c2@here"

          stem::addr_type nsf = nm.lst.begin()->first;

          evname.dest( nsf );
          evname.value() = "c2@here";

          EXAM_CHECK_ASYNC_F( pcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

          do {
            nm.reset();
            nm.lst.clear();
            nm.Send( evname );
            nm.wait();
            i = find_if( nm.lst.begin(), nm.lst.end(), compose1( bind2nd( equal_to<string>(), string( "c2@here" ) ), select2nd<pair<stem::addr_type,string> >() ) );
          } while ( i == nm.lst.end() );

          EXAM_CHECK_ASYNC_F( i != nm.lst.end(), eflag );
          EXAM_CHECK_ASYNC_F( i->second == "c2@here", eflag );

          // c2 accessable via mgr; set nice to 1000,
          // as for common remote object (may be 2000, routing
          // via two net connctions?)
          c1.manager()->Subscribe( i->first, &mgr, i->second, 1000 );

          stem::Event pe( NODE_EV_ECHO );
          pe.dest( i->first );
          pe.value() = "c2 local"; // <<-- mess is like name ... |
                                 //                            .
          c1.Send( pe );

          EXAM_CHECK_ASYNC_F( scnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );
        }
      }

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

      {
        stem::stem_scope scope( c2 );

        EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );
        stem::addr_type zero = mgr.open( "localhost", 6995 ); // take address of 'zero' (aka default) object via net transport from server
        // It done like it should on client side

        EXAM_CHECK_ASYNC_F( zero != stem::badaddr, eflag );

        stem::Event ev( NODE_EV_REGME );
        ev.dest( zero );

        ev.value() = "c2@here";
        c2.Send( ev ); // 'register' c2 client on 'echo' server

        pcnd.notify_one();

        c2.wait();
        // cerr << "Fine!" << endl;
        scnd.notify_one();
      }

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
    StEMecho echo( "echo service"); // <= zero! 'echo' server, default ('zero' address)
    {
      stem::stem_scope scope( echo );

      echo.set_default();

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

int EXAM_IMPL(stem_test::last_event)
{
  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();

  try {
    // Client
    std::tr2::this_thread::fork();

    int eflag = 0;

    try {
      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );
      stem::NetTransportMgr mgr;

      stem::addr_type def_object = mgr.open( "localhost", 6995 );

      EXAM_REQUIRE( mgr.good() );
      EXAM_REQUIRE( def_object != stem::badaddr );

      /*
      stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
      stem::EventHandler::manager()->settrs( &std::cerr );
      */

      stem::Event ev( NODE_EV_ECHO );
      ev.value() = "ping";
      ev.dest( def_object );

      {
        LastEvent le( "ping" );
        le.enable();

        le.Send( ev );

        EXAM_CHECK_ASYNC_F( le.wait(), eflag );

        // last event from dtor and conformation
      }
    }
    catch ( ... ) {
      eflag = 2;
    }
    exit( eflag );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    connect_processor<stem::NetTransport> srv( 6995 );
    EchoLast echo( "echo service" );

    stem::stem_scope scope( echo );

    echo.set_default(); // become default object

    fcnd.notify_all();
    
    EXAM_CHECK( echo.wait() ); // wait last event

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
    EXAM_CHECK( srv.is_open() );
    EXAM_CHECK( srv.good() );
    StEMecho echo( "echo service" );
    stem::stem_scope scope( echo );

    echo.set_default();

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

int EXAM_IMPL(stem_test::boring_manager_more)
{
  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();

  try {
    // Client
    std::tr2::this_thread::fork();

    int eflag = 0;
    stem::Event ev( NODE_EV_ECHO );

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

          ev.dest( a );
          ev.value() = "echo string";

          EchoClientTrivial node;
          stem::stem_scope scope( node );

          node.Send( ev );

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
    EXAM_CHECK( srv.is_open() );
    EXAM_CHECK( srv.good() );
    StEMecho echo( "echo service");
    stem::stem_scope scope( echo );

    echo.set_default();

    fcnd.notify_all();

    int stat = -1;
    pid_t chp = waitpid( child.pid(), &stat, 0 );
    EXAM_CHECK( chp == child.pid() );
    if ( chp != -1 ) {
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else if ( WIFSIGNALED(stat) ) {
        stringstream s( "child interrupted by uncatch signal " );
        s << WTERMSIG(stat);
        EXAM_ERROR( s.str().c_str() );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    } else {
      std::system_error err( errno, std::get_posix_category(), std::string( "waitpid interrupted" ) );

      EXAM_ERROR( err.what() );
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
  stem::stem_scope scope( conv );

  // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
  // stem::EventHandler::manager()->settrs( &std::cerr );

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
  ev3.value().message = ", world!";

  conv.Send( ev3 );

  EXAM_CHECK( conv.wait() );

  EXAM_CHECK( conv.v == 3 );
  EXAM_CHECK( conv.m3 == ", world!" );

  conv.v = 0;

  stem::EventVoid ev4( CONV_EV4 );

  ev4.dest( conv.self_id() );

  conv.Send( ev4 );

  EXAM_CHECK( conv.wait() );
  EXAM_CHECK( conv.v == -2 );

  conv.v = 0;

  stem::Event ev5( CONV_EV5 );

  ev5.dest( conv.self_id() );
  ev5.value() = "\nMessage pass";

  conv.Send( ev5 );

  EXAM_CHECK( conv.wait() );

  EXAM_CHECK( conv.v == -3 );
  EXAM_CHECK( conv.m3 == "\nMessage pass" );

  conv.v = 0;

  stem::Event_base<pair<int32_t,std::string> > ev6( CONV_EV6 );

  ev6.dest( conv.self_id() );
  ev6.value().first = 6;
  ev6.value().second = "pair";

  conv.Send( ev6 );

  EXAM_CHECK( conv.wait() );

  EXAM_CHECK( conv.v == 6 );
  EXAM_CHECK( conv.m3 == "pair" );

  conv.v = 0;

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::cron)
{
  {
    stem::Cron cron_obj( "cron" );
    stem::addr_type ca = cron_obj.self_id();

    EXAM_CHECK( ca != stem::badaddr );

    CronClient client;
    stem::stem_scope client_scope( client );

    stem::Event_base<stem::CronEntry> ev( EV_EDS_CRON_ADD );
    stem::Event cr( TEST_EV_CRON );

    cr.dest( client.self_id() );
    cr.src( client.self_id() );
    cr.value() = "3";
 
    ev.dest( ca );
    ev.value().ev = cr;
    system_time rec = std::tr2::get_system_time();
    ev.value().start = rec + std::tr2::milliseconds( 500 );
    ev.value().n = 1;
    // ev.value().period = 0;

    unique_lock<mutex> lk( client.m );
    client.Send( ev );

    EXAM_CHECK( client.cnd.timed_wait( lk, std::tr2::milliseconds( 800 ) ) );
    // 'True' on the line above guarantee for
    // (std::tr2::get_system_time() - rec) < std::tr2::milliseconds( 800 )

    // cerr << (std::tr2::get_system_time() - rec).count() << endl;

    EXAM_CHECK( client.see == "3" );
    EXAM_CHECK( client.visited == 1 );
  }

  EXAM_MESSAGE( "Cron stopped" );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::vf)
{
  {
    stem::addr_type addr = xmt::uid();
 
    VF obj( addr );
    stem::stem_scope scope( obj );

    stem::Event ev( 0x300 ); // not significant
    
    ev.dest( addr );

    for ( int i = 0; i < 1000; ++i ) {
      obj.Send( ev );
    }
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::vf1)
{
  {
    stem::addr_type addr = xmt::uid();
 
    VF1 obj( addr );
    stem::stem_scope scope( obj );

    stem::Event ev( 0x300 ); // not significant
    
    ev.dest( addr );

    obj.Send( ev );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::command_mgr)
{  
  try {    
    xmt::allocator_shm<barrier_ip,0> shm;
    xmt::allocator_shm<condition_event_ip,0> cnd;

    barrier_ip& b1 = *new ( shm.allocate( 1 ) ) barrier_ip();
    condition_event_ip& c = *new ( cnd.allocate(1) ) condition_event_ip();

    try {
      int res = 0;

      this_thread::fork();

      {
        connect_processor<stem::NetTransport> srv( 6995 );

        EXAM_CHECK_ASYNC_F( srv.good(), res );
      
        ProxyEcho pe;
        stem::stem_scope scope( pe );

        pe.set_default();

        b1.wait();

        c.wait();

        //srv.close();
        //srv.wait();
      }

      exit(res);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      // client
      b1.wait();
      
      {
        stem::NetTransportMgr mgr;

        stem::addr_type addr = mgr.open( "localhost", 6995 );

        EXAM_CHECK( addr != stem::badaddr );

        EchoClient node;
        stem::stem_scope scope( node );
      
        for ( int i = 0; i < 1000; ++i ) {
          stem::Event ev( NODE_EV_ECHO );

          ev.dest( addr );
          ev.value() = node.mess;

          node.Send( ev );

          EXAM_CHECK( node.wait() );
        }

        mgr.close();
        mgr.join();
      }    
      
      c.notify_all();

      int stat = -1;
      pid_t r = waitpid( child.pid(), &stat, 0 );
      EXAM_CHECK( r == child.pid() );
      if ( r != -1 ) {
        if ( WIFEXITED(stat) ) {
          EXAM_CHECK( WEXITSTATUS(stat) == 0 );
        } else if ( WIFSIGNALED(stat) ) {
          stringstream s( "child interrupted by uncatch signal " );
          s << WTERMSIG(stat);
          EXAM_ERROR( s.str().c_str() );
        } else {
          EXAM_ERROR( "child interrupted?" );
        }
      } else {
        EXAM_ERROR( "waitpid interrupted?" );
      }
    }

    cnd.deallocate( &c );
    shm.deallocate( &b1 );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }
  
  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::route_to_net)
{
  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();
  stem::addr_type& addr = *new ( shm_a.allocate( 1 ) ) stem::addr_type();

  addr = stem::badaddr;

  try {
    std::tr2::this_thread::fork();

    int eflag = 0;

    try {
      stem::NetTransportMgr mgr;

      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      /* stem::addr_type def_object = */ mgr.open( "localhost", 6995 );

      // EXAM_CHECK_ASYNC_F( def_object != stem::badaddr, eflag );
      // EXAM_CHECK_ASYNC_F( def_object == addr, eflag );
      EXAM_CHECK_ASYNC_F( addr != stem::badaddr, eflag );

      mgr.add_route( addr );

      EchoClient node;
      {
        stem::stem_scope scope( node );
    
        stem::Event ev( NODE_EV_ECHO );

        ev.dest( addr );
        ev.value() = node.mess;

        // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
        // stem::EventHandler::manager()->settrs( &std::cerr );

        node.Send( ev );

        EXAM_CHECK_ASYNC_F( node.wait(), eflag );
      }

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

      StEMecho echo( "echo service" );

      {
        stem::stem_scope scope( echo );

        addr = echo.self_id();
        // echo.set_default(); // become default object

        // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
        // stem::EventHandler::manager()->settrs( &std::cerr );

        fcnd.notify_one();

        int stat = -1;
        EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );

        if ( WIFEXITED(stat) ) {
          EXAM_CHECK( WEXITSTATUS(stat) == 0 );
        } else {
          EXAM_ERROR( "child interrupted" );
        }
      }

      srv.close();
      srv.wait();
    }
    catch ( ... ) {
    }
  }

  (&fcnd)->~condition_event_ip();
  shm_cnd.deallocate( &fcnd, 1 );
  shm_a.deallocate( &addr, 1 );

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_test::route_from_net)
{
  condition_event_ip& fcnd = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();
  condition_event_ip& fcnd2 = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();
  condition_event_ip& fcnd3 = *new ( shm_cnd.allocate( 1 ) ) condition_event_ip();
  stem::addr_type& addr = *new ( shm_a.allocate( 1 ) ) stem::addr_type();

  addr = stem::badaddr;

  try {
    std::tr2::this_thread::fork();

    int eflag = 0;

    try {
      stem::NetTransportMgr mgr;

      StEMecho echo( "echo service" );
      addr = echo.self_id();

      EXAM_CHECK_ASYNC_F( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );

      /* stem::addr_type def_object = */ mgr.open( "localhost", 6995 );

      EXAM_CHECK_ASYNC_F( mgr.is_open(), eflag );
      EXAM_CHECK_ASYNC_F( mgr.good(), eflag );

      {
        stem::stem_scope scope( echo );
        mgr.add_remote_route( echo.self_id() );

        this_thread::sleep( milliseconds(200) ); // chance to accept on other side

        fcnd2.notify_one();
        EXAM_CHECK_ASYNC_F( fcnd3.timed_wait( std::tr2::milliseconds( 800 ) ), eflag );
      }

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

      fcnd.notify_one();

      EchoClient node;
      {
        stem::stem_scope scope( node );
    
        stem::Event ev( NODE_EV_ECHO );

        EXAM_CHECK( fcnd2.timed_wait( std::tr2::milliseconds( 800 ) ) );
        EXAM_CHECK( addr != stem::badaddr );
        EXAM_CHECK( node.is_avail( addr ) );

        ev.dest( addr );
        ev.value() = node.mess;

        // stem::EventHandler::manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
        // stem::EventHandler::manager()->settrs( &std::cerr );

        node.Send( ev );

        EXAM_CHECK( node.wait() );
      }

      fcnd3.notify_one();

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
  (&fcnd2)->~condition_event_ip();
  (&fcnd3)->~condition_event_ip();

  shm_cnd.deallocate( &fcnd, 1 );
  shm_cnd.deallocate( &fcnd2, 1 );
  shm_cnd.deallocate( &fcnd3, 1 );

  shm_a.deallocate( &addr, 1 );

  return EXAM_RESULT;
}

int main( int argc, const char** argv )
{
  exam::test_suite::test_case_type tc[7];

  exam::test_suite t( "libstem test suite" );
  stem_test test;

  tc[1] = t.add( &stem_test::basic2, test, "basic2",
                 tc[0] = t.add( &stem_test::basic1, test, "basic1" ) );

  tc[2] = t.add( &stem_test::basic1new, test, "basic1new", tc[0] );  

  t.add( &stem_test::dl, test, "dl",
         t.add( &stem_test::basic2new, test, "basic2new", tc + 1, tc + 3 ) );
  t.add( &stem_test::weight, test, "weight", tc, tc + 2 );
  t.add( &stem_test::ns, test, "ns", tc[0] );
  
  t.add( &stem_test::ugly_echo_net, test, "ugly echo net", 
    t.add( &stem_test::triple_echo, test, "triple echo", 
      t.add( &stem_test::net_echo, test, "net echo",
        tc[4] = t.add( &stem_test::echo_net, test, "echo_net",
            tc[3] = t.add( &stem_test::echo, test, "echo", tc[1] ) ) ) ) );
  t.add( &stem_test::echo_local, test, "echo unix socket", tc[3] );
  t.add( &stem_test::route_from_net, test, "route from net", 
    t.add( &stem_test::route_to_net, test, "route to net", tc[4] ) );

  exam::test_suite::test_case_type cases_next[10];

  cases_next[2] =
    t.add( &stem_test::boring_manager_more, test, "boring_manager with echo",
      t.add( &stem_test::boring_manager, test, "boring_manager",
        t.add( &stem_test::peer, test, "peer", tc[3] ) ) );
                
  tc[5] = t.add( &stem_test::convert, test, "convert", tc[0] );

  tc[6] = t.add( &stem_test::last_event, test, "last event", tc[4] );

  cases_next[0] = tc[2];
  cases_next[1] = tc[5];

  t.add( &stem_test::cron, test, "cron", cases_next, cases_next + 2 );

  cases_next[3] = t.add( &stem_test::vf, test, "dtor when come event", tc[6] );
  t.add( &stem_test::vf1, test, "dtor when come event - resource control", cases_next[3] );
  t.add( &stem_test::command_mgr, test, "command mgr", cases_next + 2, cases_next + 4 );

  Opts opts;

  opts.description( "test suite for 'StEM' framework" );
  opts.usage( "[options]" );

  opts << option<void>( "print this help message", 'h', "help" )
       << option<void>( "list all test cases", 'l', "list" )
       << option<string>( "run tests by <numbers list>", 'r', "run" )
       << option<void>( "print status of tests within test suite", 'v', "verbose" )
       << option<void>(  "trace checks", 't', "trace" );

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
