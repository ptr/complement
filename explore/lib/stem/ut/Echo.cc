// -*- C++ -*- Time-stamp: <08/11/27 11:42:36 ptr>

/*
 * Copyright (c) 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "Echo.h"

#include <stem/NetTransport.h>
#include <stem/EvManager.h>

#include <exam/suite.h>
#include <mt/date_time>

using namespace stem;

StEMecho::StEMecho()
{
}

StEMecho::StEMecho( addr_type id ) :
    EventHandler( id )
{
}

StEMecho::StEMecho( addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

void StEMecho::echo( const Event& ev )
{
  Event eev( ev.code() );
  eev.value() = ev.value();

  eev.dest( ev.src() );

  Send( eev );
}

void StEMecho::regme( const stem::Event& ev )
{
  // cerr << "Echo\n";
  manager()->change_announce( ev.src(), ev.value() );

  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( StEMecho )
  EV_EDS( 0, NODE_EV_ECHO, echo )
  EV_EDS( 0, NODE_EV_REGME, regme )
END_RESPONSE_TABLE

EchoClient::EchoClient() :
    EventHandler(),
    mess( "echo string" )
{
}

EchoClient::EchoClient( stem::addr_type id ) :
    EventHandler( id ),
    mess( "echo string" )
{
}

EchoClient::EchoClient( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    mess( "echo string" )
{
}

EchoClient::~EchoClient()
{
  // cnd.wait();
}

void EchoClient::handler1( const stem::Event& ev )
{
  EXAM_CHECK_ASYNC( ev.value() == mess );

  cnd.notify_one();
}

bool EchoClient::wait()
{
  return cnd.timed_wait( std::tr2::milliseconds( 500 ) );
}

DEFINE_RESPONSE_TABLE( EchoClient )
  EV_EDS(0,NODE_EV_ECHO,handler1)
END_RESPONSE_TABLE

PeerClient::PeerClient() :
    EventHandler(),
    mess( "peer client" )
{
}

PeerClient::PeerClient( stem::addr_type id ) :
    EventHandler( id ),
    mess( "peer client" )
{
}

PeerClient::PeerClient( const char *info ) :
    EventHandler( info ),
    mess( info )
{
}

PeerClient::PeerClient( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    mess( info )
{
}

PeerClient::~PeerClient()
{
  // cnd.wait();
}

void PeerClient::handler1( const stem::Event& ev )
{
  EXAM_CHECK_ASYNC( ev.value() == mess );

  cnd.notify_one();
}

bool PeerClient::wait()
{
  return cnd.timed_wait( std::tr2::milliseconds( 500 ) );
}

DEFINE_RESPONSE_TABLE( PeerClient )
  EV_EDS(0,NODE_EV_ECHO,handler1)
END_RESPONSE_TABLE

EchoLast::EchoLast()
{
}

EchoLast::EchoLast( addr_type id ) :
    EventHandler( id )
{
}

EchoLast::EchoLast( addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

void EchoLast::echo( const Event& ev )
{
  Event eev( ev.code() );
  eev.value() = ev.value();

  eev.dest( ev.src() );

  Send( eev );
}

void EchoLast::last( const stem::Event& ev )
{
  cerr << "Echo\n";

  cnd.notify_one();
}

bool EchoLast::wait()
{
  return cnd.timed_wait( std::tr2::milliseconds( 800 ) );
}

DEFINE_RESPONSE_TABLE( EchoLast )
  EV_EDS( 0, NODE_EV_ECHO, echo )
  EV_EDS( 0, NODE_EV_LAST, last )
END_RESPONSE_TABLE

LastEvent::LastEvent( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    mess( info ),
    peer( stem::badaddr )
{
}

LastEvent::~LastEvent()
{
  if ( peer != stem::badaddr ) {
    Event ev( NODE_EV_LAST );

    ev.dest( peer );

    Send( ev );
  }
}

void LastEvent::handler( const stem::Event& ev )
{
  EXAM_CHECK_ASYNC( ev.value() == mess );

  peer = ev.src();

  cnd.notify_one();
}

bool LastEvent::wait()
{
  return cnd.timed_wait( std::tr2::milliseconds( 500 ) );
}

DEFINE_RESPONSE_TABLE( LastEvent )
  EV_EDS(0,NODE_EV_ECHO,handler)
END_RESPONSE_TABLE
