// -*- C++ -*- Time-stamp: <06/09/29 22:53:34 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include "../NodeDL.h"

using namespace stem;

NodeDL::NodeDL() :
    EventHandler(),
    v( 0 )
{
  cnd.set( false );
}

NodeDL::NodeDL( stem::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
  cnd.set( false );
}

NodeDL::~NodeDL()
{
  // cnd.wait();
}

void NodeDL::handler1( const stem::Event& )
{
  std::cerr << "I am here 1\n";
  v = 1;
  cnd.set(true);
  std::cerr << "I am here 2\n";
}

void NodeDL::wait()
{
  cnd.wait();
}

DEFINE_RESPONSE_TABLE( NodeDL )
  EV_EDS(0,NODE_EV2,handler1)
END_RESPONSE_TABLE

// the same as NodeDL, just another class

NewNodeDL::NewNodeDL() :
    EventHandler(),
    v( 0 )
{
  cnd.set( false );
}

NewNodeDL::NewNodeDL( stem::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
  cnd.set( false );
}

NewNodeDL::~NewNodeDL()
{
  // cnd.wait();
}

void NewNodeDL::handler1( const stem::Event& )
{
  std::cerr << "I am here 1\n";
  v = 1;
  cnd.set(true);
  std::cerr << "I am here 2\n";
}

void NewNodeDL::wait()
{
  cnd.wait();
}

DEFINE_RESPONSE_TABLE( NewNodeDL )
  EV_EDS(0,NODE_EV2,handler1)
END_RESPONSE_TABLE

void *create_NewNodeDL( unsigned a )
{
  return (void *)new NewNodeDL( a );
}

void wait_NewNodeDL( void *p )
{
  reinterpret_cast<NewNodeDL *>( p )->wait();
}

int v_NewNodeDL( void *p )
{
  return reinterpret_cast<NewNodeDL *>( p )->v;
}

void destroy_NewNodeDL( void *p )
{
  NewNodeDL *node = reinterpret_cast<NewNodeDL *>( p );
  delete node;
}
