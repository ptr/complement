// -*- C++ -*- Time-stamp: <10/05/25 11:46:48 ptr>

/*
 *
 * Copyright (c) 2007-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "Convert.h"
#include <mt/date_time>
#include <string>

using namespace std::tr2;

void mess::pack( std::ostream& s ) const
{
  __pack( s, super_id );
  __pack( s, message );
}

void mess::unpack( std::istream& s )
{
  __unpack( s, super_id );
  __unpack( s, message );
}

int Convert::v = 0;


Convert::Convert() :
    EventHandler()
{
}

Convert::Convert( stem::addr_type id ) :
    EventHandler( id )
{
}

Convert::Convert( stem::addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

Convert::~Convert()
{
}

void Convert::handler0()
{
  lock_guard<mutex> lk( mtx );
  v = -1;
  cnd.notify_one();
}

void Convert::handler1( const stem::Event& )
{
  lock_guard<mutex> lk( mtx );
  v = 1;
  cnd.notify_one();
}

void Convert::handler2( const stem::Event_base<mess>& ev )
{
  lock_guard<mutex> lk( mtx );
  v = ev.value().super_id;
  m2 = ev.value().message;

  cnd.notify_one();
}

void Convert::handler3( const mess& m )
{
  lock_guard<mutex> lk( mtx );
  v = m.super_id;
  m3 = m.message;

  cnd.notify_one();
}

void Convert::handler4( const stem::EventVoid& ev )
{
  lock_guard<mutex> lk( mtx );
  v = -2;
  cnd.notify_one();
}

void Convert::handler5( const std::string& m )
{
  lock_guard<mutex> lk( mtx );
  v = -3;
  m3 = m;

  cnd.notify_one();
}

void Convert::handler6( const stem::Event_base<Convert::compaund_type>& ev )
{
  lock_guard<mutex> lk( mtx );
  v = ev.value().first;
  m3 = ev.value().second;

  cnd.notify_one();
}

bool Convert::wait()
{
  unique_lock<mutex> lk( mtx );
  return cnd.timed_wait( lk, std::tr2::milliseconds( 500 ), v_nz_check );
}

DEFINE_RESPONSE_TABLE( Convert )
  EV_VOID( ST_NULL, CONV_EV0, handler0 )
  EV_EDS( ST_NULL, CONV_EV1, handler1 )
  EV_Event_base_T_( ST_NULL, CONV_EV2, handler2, mess )
  EV_T_( ST_NULL, CONV_EV3, handler3, mess )
  EV_Event_base_T_( ST_NULL, CONV_EV4, handler4, void )
  EV_T_( ST_NULL, CONV_EV5, handler5, std::string )
  EV_Event_base_T_( ST_NULL, CONV_EV6, handler6, compaund_type )
END_RESPONSE_TABLE
