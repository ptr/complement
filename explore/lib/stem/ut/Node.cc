// -*- C++ -*-

/*
 *
 * Copyright (c) 2002, 2003, 2006-2009, 2020
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "Node.h"

#include <chrono>

using namespace std;
using namespace stem;

using std::lock_guard;
using std::mutex;

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
  lock_guard<mutex> lk(m);
  // std::cerr << "I am here 1\n";
  v = 1;
  cnd.notify_one();
  // std::cerr << "I am here 2\n";
}

bool Node::wait()
{
  unique_lock<mutex> lk( m );
  return cnd.wait_for(lk, chrono::milliseconds(800), check_v( *this ));
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
  return cnd.wait_for(lk, chrono::milliseconds(500), check_v(*this));
}

DEFINE_RESPONSE_TABLE( NewNode )
  EV_EDS(0,NODE_EV1,handler1)
END_RESPONSE_TABLE
