// -*- C++ -*- Time-stamp: <09/04/30 11:34:39 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2006-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "../NodeDL.h"

using namespace stem;
using namespace std::tr2;

NodeDL::NodeDL() :
    EventHandler(),
    v( 0 )
{
}

NodeDL::NodeDL( stem::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
}

NodeDL::~NodeDL()
{
}

void NodeDL::handler1( const stem::Event& )
{
  lock_guard<mutex> lk( m );
  v = 1;
  cnd.notify_one();
}

bool NodeDL::wait()
{
  unique_lock<mutex> lk( m );
  return cnd.timed_wait( lk, std::tr2::milliseconds( 500 ), check_v( *this ) );
}

DEFINE_RESPONSE_TABLE( NodeDL )
  EV_EDS(0,NODE_EV2,handler1)
END_RESPONSE_TABLE

// the same as NodeDL, just another class

NewNodeDL::NewNodeDL() :
    EventHandler(),
    v( 0 )
{
}

NewNodeDL::NewNodeDL( stem::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
}

NewNodeDL::~NewNodeDL()
{
}

void NewNodeDL::handler1( const stem::Event& )
{
  lock_guard<mutex> lk( m );
  v = 1;
  cnd.notify_one();
}

bool NewNodeDL::wait()
{
  unique_lock<mutex> lk( m );
  return cnd.timed_wait( lk, std::tr2::milliseconds( 800 ), check_v( *this )  );
}

DEFINE_RESPONSE_TABLE( NewNodeDL )
  EV_EDS(0,NODE_EV2,handler1)
END_RESPONSE_TABLE

void *create_NewNodeDL( stem::addr_type a )
{
  return (void *)new NewNodeDL( a );
}

int wait_NewNodeDL( void *p )
{
  return reinterpret_cast<NewNodeDL *>( p )->wait() ? 1 : 0;
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
