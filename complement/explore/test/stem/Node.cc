// -*- C++ -*- Time-stamp: <06/09/29 23:23:57 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "Node.h"

using namespace std;
using namespace stem;
using namespace xmt;

Node::Node() :
    EventHandler(),
    v( 0 )
{
  cnd.set( false );
}

Node::Node( stem::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
  cnd.set( false );
}

Node::Node( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    v( 0 )
{
  cnd.set( false );
}

Node::~Node()
{
  // cnd.wait();
}

void Node::handler1( const stem::Event& )
{
  // std::cerr << "I am here 1\n";
  v = 1;
  cnd.set(true);
  // std::cerr << "I am here 2\n";
}

void Node::wait()
{
  cnd.try_wait();
}

DEFINE_RESPONSE_TABLE( Node )
  EV_EDS(0,NODE_EV1,handler1)
END_RESPONSE_TABLE

// the same as Node, just another class

NewNode::NewNode() :
    EventHandler(),
    v( 0 )
{
  cnd.set( false );
}

NewNode::NewNode( stem::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
  cnd.set( false );
}

NewNode::~NewNode()
{
  // cnd.wait();
}

void NewNode::handler1( const stem::Event& )
{
  // std::cerr << "I am here 1\n";
  v = 1;
  cnd.set(true);
  // std::cerr << "I am here 2\n";
}

void NewNode::wait()
{
  cnd.try_wait();
}

DEFINE_RESPONSE_TABLE( NewNode )
  EV_EDS(0,NODE_EV1,handler1)
END_RESPONSE_TABLE
