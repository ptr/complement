// -*- C++ -*- Time-stamp: <06/11/29 13:02:34 ptr>

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
  cnd.set( true );
}

DEFINE_RESPONSE_TABLE( StEMecho )
  EV_EDS( 0, NODE_EV_ECHO, echo )
  EV_EDS( 0, NODE_EV_REGME, regme )
END_RESPONSE_TABLE

EchoClient::EchoClient() :
    EventHandler(),
    mess( "echo string" )
{
  cnd.set( false );
}

EchoClient::EchoClient( stem::addr_type id ) :
    EventHandler( id ),
    mess( "echo string" )
{
  cnd.set( false );
}

EchoClient::EchoClient( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    mess( "echo string" )
{
  cnd.set( false );
}

EchoClient::~EchoClient()
{
  // cnd.wait();
}

void EchoClient::handler1( const stem::Event& ev )
{
  EXAM_CHECK_ASYNC( ev.value() == mess );
  cnd.set(true);
}

void EchoClient::wait()
{
  cnd.try_wait();
}

DEFINE_RESPONSE_TABLE( EchoClient )
  EV_EDS(0,NODE_EV_ECHO,handler1)
END_RESPONSE_TABLE

PeerClient::PeerClient() :
    EventHandler(),
    mess( "peer client" )
{
  cnd.set( false );
}

PeerClient::PeerClient( stem::addr_type id ) :
    EventHandler( id ),
    mess( "peer client" )
{
  cnd.set( false );
}

PeerClient::PeerClient( const char *info ) :
    EventHandler( info ),
    mess( info )
{
  cnd.set( false );
}

PeerClient::PeerClient( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    mess( info )
{
  cnd.set( false );
}

PeerClient::~PeerClient()
{
  // cnd.wait();
}

void PeerClient::handler1( const stem::Event& ev )
{
  EXAM_CHECK( ev.value() == mess );

  cnd.set(true);
}

void PeerClient::wait()
{
  cnd.try_wait();
}

DEFINE_RESPONSE_TABLE( PeerClient )
  EV_EDS(0,NODE_EV_ECHO,handler1)
END_RESPONSE_TABLE
