// -*- C++ -*- Time-stamp: <09/06/26 16:19:25 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2006-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "Node.h"

#include <mt/date_time>

using namespace std;
using namespace stem;
using namespace std::tr2;

Node::Node() :
    EventHandler(),
    v( 0 )
{
}

Node::Node( stem::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
}

Node::Node( stem::addr_type id, int nice ) :
    EventHandler( id, nice ),
    v( 0 )
{
}

Node::Node( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    v( 0 )
{
}

Node::~Node()
{
}

void Node::handler1( const stem::Event& )
{
  lock_guard<mutex> lk( m );
  // std::cerr << "I am here 1\n";
  v = 1;
  cnd.notify_one();
  // std::cerr << "I am here 2\n";
}

bool Node::wait()
{
  unique_lock<mutex> lk( m );
  return cnd.timed_wait( lk, std::tr2::milliseconds( 800 ), check_v( *this ) );
}

DEFINE_RESPONSE_TABLE( Node )
  EV_EDS(0,NODE_EV1,handler1)
END_RESPONSE_TABLE

// the same as Node, just another class

NewNode::NewNode() :
    EventHandler(),
    v( 0 )
{
}

NewNode::NewNode( stem::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
}

NewNode::~NewNode()
{
}

void NewNode::handler1( const stem::Event& )
{
  lock_guard<mutex> lk( m );
  // std::cerr << "I am here 1\n";
  v = 1;
  cnd.notify_one();
  // std::cerr << "I am here 2\n";
}

bool NewNode::wait()
{
  unique_lock<mutex> lk( m );
  return cnd.timed_wait( lk, std::tr2::milliseconds( 500 ), check_v( *this )  );
}

DEFINE_RESPONSE_TABLE( NewNode )
  EV_EDS(0,NODE_EV1,handler1)
END_RESPONSE_TABLE
