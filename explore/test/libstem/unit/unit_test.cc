// -*- C++ -*- Time-stamp: <06/08/04 12:08:37 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.1
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include <iostream>
#include <mt/xmt.h>

// #include <sockios/sockstream>
// #include <sockios/sockmgr.h>

#include <stem/EventHandler.h>

using namespace std;

class Node :
    public EDS::EventHandler
{
  public:
    Node();
    Node( EDS::addr_type id );
    ~Node();

    void handler1( const EDS::Event& );

    void wait();

    int v;

  private:
    xmt::Condition cnd;

    DECLARE_RESPONSE_TABLE( Node, EDS::EventHandler );
};

Node::Node() :
    EventHandler(),
    v( 0 )
{
  cnd.set( false );
}

Node::Node( EDS::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
  cnd.set( false );
}

Node::~Node()
{
  // cnd.wait();
}

void Node::handler1( const EDS::Event& )
{
  std::cerr << "I am here 1\n";
  v = 1;
  cnd.set(true);
  std::cerr << "I am here 2\n";
}

void Node::wait()
{
  cnd.wait();
}

#define NODE_EV1 0x900

DEFINE_RESPONSE_TABLE( Node )
  EV_EDS(0,NODE_EV1,handler1)
END_RESPONSE_TABLE

void send_test()
{
  Node node( 2000 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node.Send( ev );

  node.wait();

  BOOST_CHECK_EQUAL( node.v, 1 );
}

xmt::Thread::ret_code thr1( void * )
{
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  Node node( 2001 );

  EDS::Event ev( NODE_EV1 );

  ev.dest( 2000 );
  node.Send( ev );

  return rt;
}

void send2_test()
{
  Node node( 2000 );
  
  xmt::Thread t1( thr1 );
  // __impl::Thread t2( thr2 );

  // t2.join();
  t1.join();

  node.wait();
//  BOOST_CHECK_EQUAL( node.v, 1 );  
}

test_suite *init_unit_test_suite( int argc, char **argv )
{
  test_suite *ts = BOOST_TEST_SUITE( "libstem test" );

  ts->add( BOOST_TEST_CASE( &send_test ) );
  ts->add( BOOST_TEST_CASE( &send2_test ) );

  return ts;
}
