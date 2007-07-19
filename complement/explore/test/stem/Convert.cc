// -*- C++ -*- Time-stamp: <07/07/20 00:05:52 ptr>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "Convert.h"

void mess::pack( std::ostream& s ) const
{
  __pack( s, super_id );
  __pack( s, message );
}

void mess::net_pack( std::ostream& s ) const
{
  __net_pack( s, super_id );
  __net_pack( s, message );
}

void mess::unpack( std::istream& s )
{
  __unpack( s, super_id );
  __unpack( s, message );
}

void mess::net_unpack( std::istream& s )
{
  __net_unpack( s, super_id );
  __net_unpack( s, message );
}

Convert::Convert() :
    EventHandler(),
    v( 0 )
{
  cnd.set( false );
}

Convert::Convert( stem::addr_type id ) :
    EventHandler( id ),
    v( 0 )
{
  cnd.set( false );
}

Convert::Convert( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    v( 0 )
{
  cnd.set( false );
}

Convert::~Convert()
{
  // cnd.wait();
}

void Convert::handler0()
{
  v = -1;
  cnd.set(true);
}

void Convert::handler1( const stem::Event& )
{
  v = 1;
  cnd.set(true);
}

void Convert::handler2( const stem::Event_base<mess>& ev )
{
  v = ev.value().super_id;
  m2 = ev.value().message;

  cnd.set(true);
}

void Convert::handler3( const mess& m )
{
  v = m.super_id;
  m3 = m.message;

  cnd.set(true);
}

void Convert::wait()
{
  cnd.try_wait();

  cnd.set( false );
}

DEFINE_RESPONSE_TABLE( Convert )
  EV_VOID( ST_NULL, CONV_EV0, handler0 )
  EV_EDS( ST_NULL, CONV_EV1, handler1 )
  EV_Event_base_T_( ST_NULL, CONV_EV2, handler2, mess )
  EV_T_( ST_NULL, CONV_EV3, handler3, mess )
END_RESPONSE_TABLE
