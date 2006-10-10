// -*- C++ -*- Time-stamp: <06/10/10 16:49:12 ptr>

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

#include <stem/EventHandler.h>
#include <stem/Names.h>
#include <stem/EDSEv.h>

#include "Node.h"
#include "NodeDL.h"
#include "NameService.h"

#include <dlfcn.h>

#include "Echo.h"
#include <stem/NetTransport.h>
#include <sockios/sockmgr.h>

using namespace std;

struct stem_test
{
    void basic1();
    void basic2();
    void basic1new();
    void basic2new();
    void dl();
    void ns();
    void ns1();

    void echo();

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
#ifdef _STLP_DEBUG
  void *lh = dlopen( "dl/obj/gcc/shared-stlg/libloadable_stemstlg.so", RTLD_LAZY );
#elif defined(DEBUG)
  void *lh = dlopen( "dl/obj/gcc/shared-g/libloadable_stemg.so", RTLD_LAZY );
#else
  void *lh = dlopen( "dl/obj/gcc/shared/libloadable_stem.so", RTLD_LAZY );
#endif
  BOOST_REQUIRE( lh != NULL );
  void *(*f)(unsigned) = reinterpret_cast<void *(*)(unsigned)>( dlsym( lh, "create_NewNodeDL" ) );
  BOOST_REQUIRE( f != NULL );
  void (*g)(void *) = reinterpret_cast<void (*)(void *)>( dlsym( lh, "destroy_NewNodeDL" ) );
  BOOST_REQUIRE( g != NULL );
  void (*w)(void *) = reinterpret_cast<void (*)(void *)>( dlsym( lh, "wait_NewNodeDL" ) );
  BOOST_REQUIRE( w != NULL );
  int (*v)(void *) = reinterpret_cast<int (*)(void *)>( dlsym( lh, "v_NewNodeDL" ) );
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

  stem::Event ev( EV_EDS_RQ_ADDR_LIST );
  ev.dest( stem::ns_addr );
  nm.Send( ev );

  nm.wait();

  list<stem::NameRecord>::const_iterator i = find( nm.lst.begin(), nm.lst.end(), string( "ns" ) );

  BOOST_CHECK( i != nm.lst.end() );
  BOOST_CHECK( i->record == "ns" );
  BOOST_CHECK( i->addr == stem::ns_addr );

  i = find( nm.lst.begin(), nm.lst.end(), string( "Node" ) );

  BOOST_CHECK( i != nm.lst.end() );
  BOOST_CHECK( i->record == "Node" );
  BOOST_CHECK( i->addr == 2003 );

  i = find( nm.lst.begin(), nm.lst.end(), nm.self_id() );

  BOOST_CHECK( i != nm.lst.end() );
  BOOST_CHECK( i->addr == nm.self_id() );
  BOOST_CHECK( i->record.length() == 0 );

  nm.lst.clear();
  nm.reset();

  BOOST_CHECK( nm.lst.empty() );

  stem::Event evname( EV_EDS_RQ_ADDR_BY_NAME );
  evname.dest( stem::ns_addr );
  evname.value() = "Node";
  nm.Send( evname );

  nm.wait();

  i = find( nm.lst.begin(), nm.lst.end(), string( "ns" ) );

  BOOST_CHECK( i == nm.lst.end() );

  i = find( nm.lst.begin(), nm.lst.end(), string( "Node" ) );

  BOOST_CHECK( i != nm.lst.end() );
  BOOST_CHECK( i->record == "Node" );
  BOOST_CHECK( i->addr == 2003 );

  i = find( nm.lst.begin(), nm.lst.end(), nm.self_id() );

  BOOST_CHECK( i == nm.lst.end() );

  nm.lst.clear();
  nm.reset();

  BOOST_CHECK( nm.lst.empty() );

  evname.value() = "No-such-name";
  nm.Send( evname );

  nm.wait();

  BOOST_CHECK( nm.lst.empty() );
}

void stem_test::ns1()
{
  Node node( 2003, "Node" );
  Naming nm;

  stem::Event ev( EV_STEM_RQ_ADDR_LIST1 );
  ev.dest( stem::ns_addr );
  nm.Send( ev );

  nm.wait();

  // this is sample of all inline find:
  Naming::nsrecords_type::const_iterator i = find_if( nm.lst1.begin(), nm.lst1.end(), compose1( bind2nd( equal_to<string>(), string( "ns" ) ), select2nd<pair<stem::addr_type,string> >() ) );

  BOOST_CHECK( i != nm.lst1.end() );
  BOOST_CHECK( i->second == "ns" );
  BOOST_CHECK( i->first == stem::ns_addr );

  // well, but for few seaches declare and reuse functors:
  equal_to<string> eq;
  equal_to<stem::addr_type> eqa;

  select1st<pair<stem::addr_type,string> > first;
  select2nd<pair<stem::addr_type,string> > second;

  i = find_if( nm.lst1.begin(), nm.lst1.end(), compose1( bind2nd( eq, string( "Node" ) ), second ) );

  BOOST_CHECK( i != nm.lst1.end() );
  BOOST_CHECK( i->second == "Node" );
  BOOST_CHECK( i->first == 2003 );

  i = find_if( nm.lst1.begin(), nm.lst1.end(), compose1( bind2nd( eqa, nm.self_id() ), first ) );

  BOOST_CHECK( i != nm.lst1.end() );
  BOOST_CHECK( i->first == nm.self_id() );
  BOOST_CHECK( i->second.length() == 0 );

  nm.lst1.clear();
  nm.reset();

  BOOST_CHECK( nm.lst1.empty() );

  stem::Event evname( EV_STEM_RQ_ADDR_BY_NAME1 );
  evname.dest( stem::ns_addr );
  evname.value() = "Node";
  nm.Send( evname );

  nm.wait();

  i = find_if( nm.lst1.begin(), nm.lst1.end(), compose1( bind2nd( eq, string( "ns" ) ), second ) );

  BOOST_CHECK( i == nm.lst1.end() );

  i = find_if( nm.lst1.begin(), nm.lst1.end(), compose1( bind2nd( eq, string( "Node" ) ), second ) );

  BOOST_CHECK( i != nm.lst1.end() );
  BOOST_CHECK( i->second == "Node" );
  BOOST_CHECK( i->first == 2003 );

  i = find_if( nm.lst1.begin(), nm.lst1.end(), compose1( bind2nd( eqa, nm.self_id() ), first ) );

  BOOST_CHECK( i == nm.lst1.end() );

  nm.lst1.clear();
  nm.reset();

  BOOST_CHECK( nm.lst1.empty() );

  evname.value() = "No-such-name";
  nm.Send( evname );

  nm.wait();

  BOOST_CHECK( nm.lst1.empty() );
}

void stem_test::echo()
{
  try {
    sockmgr_stream_MP<stem::NetTransport> srv( 6995 );
    stem::NetTransportMgr mgr;

    StEMecho echo( 0, "echo service"); // <= zero!

    stem::addr_type zero = mgr.open( "localhost", 6995 );

    BOOST_CHECK( zero != stem::badaddr );
    BOOST_CHECK( zero != 0 );

    EchoClient node;
    
    EDS::Event ev( NODE_EV_ECHO );

    ev.dest( zero );
    ev.value() = node.mess;

    node.Send( ev );

    node.wait();
    
    mgr.close();
    // mgr.join();

    srv.close();
    srv.wait();
  }
  catch ( ... ) {
  }
  // cerr << "Fine\n";
}

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
  test_case *ns1_tc = BOOST_CLASS_TEST_CASE( &stem_test::ns1, instance );
  test_case *echo_tc = BOOST_CLASS_TEST_CASE( &stem_test::echo, instance );

  basic2_tc->depends_on( basic1_tc );
  basic1n_tc->depends_on( basic1_tc );
  basic2n_tc->depends_on( basic2_tc );
  basic2n_tc->depends_on( basic1n_tc );
  dl_tc->depends_on( basic2n_tc );
  ns_tc->depends_on( basic1_tc );
  ns1_tc->depends_on( basic1_tc );

  echo_tc->depends_on( basic2_tc );

  add( basic1_tc );
  add( basic2_tc );
  add( basic1n_tc );
  add( basic2n_tc );
  add( dl_tc );
  add( ns_tc );
  add( ns1_tc );

  add( echo_tc );
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
