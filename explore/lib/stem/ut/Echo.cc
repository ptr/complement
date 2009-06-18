// -*- C++ -*- Time-stamp: <09/06/05 14:10:19 ptr>

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

#include <exam/defs.h>

using namespace std;
using namespace stem;
using namespace std::tr2;

StEMecho::StEMecho()
{
}

StEMecho::StEMecho( const char* info ) :
    EventHandler( info )
{
}

StEMecho::StEMecho( addr_type id ) :
    EventHandler( id )
{
}

StEMecho::StEMecho( addr_type id, const char* info ) :
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
  manager()->annotate( ev.src(), ev.value() );

  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( StEMecho )
  EV_EDS( 0, NODE_EV_ECHO, echo )
  EV_EDS( 0, NODE_EV_REGME, regme )
END_RESPONSE_TABLE

EchoClientTrivial::EchoClientTrivial() :
    EventHandler(),
    mess( "echo string" )
{
}

EchoClientTrivial::~EchoClientTrivial()
{
}

void EchoClientTrivial::handler1( const stem::Event& ev )
{
  EXAM_CHECK_ASYNC( ev.value() == mess );
}

DEFINE_RESPONSE_TABLE( EchoClientTrivial )
  EV_EDS(0,NODE_EV_ECHO,handler1)
END_RESPONSE_TABLE

EchoClient::EchoClient() :
    EventHandler(),
    mess( "echo string" ),
    v(0)
{
}

EchoClient::EchoClient( stem::addr_type id ) :
    EventHandler( id ),
    mess( "echo string" ),
    v(0)
{
}

EchoClient::EchoClient( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    mess( "echo string" ),
    v(0)
{
}

EchoClient::~EchoClient()
{
  // cnd.wait();
}

void EchoClient::handler1( const stem::Event& ev )
{
  lock_guard<mutex> lk( m );
  
  EXAM_CHECK_ASYNC( ev.value() == mess );
  
  v = 1;
  cnd.notify_one();
}

bool EchoClient::wait()
{
  unique_lock<mutex> lk( m );
  bool res = cnd.timed_wait( lk, std::tr2::milliseconds( 2000 ), check_v( *this ) );
  
  v = 0;
   
  return res; 
}

DEFINE_RESPONSE_TABLE( EchoClient )
  EV_EDS(0,NODE_EV_ECHO,handler1)
END_RESPONSE_TABLE


void UglyEchoSrv::echo( const Event& ev )
{
  Event eev( ev.code() );

  eev.dest( ev.src() );
  eev.value() = ev.value();

  for ( int i = 0; i < 8; ++i ) { // to do: check also with Event in the loop
    Send( eev );
  }
}

DEFINE_RESPONSE_TABLE( UglyEchoSrv )
  EV_EDS( 0, NODE_EV_ECHO, echo )
END_RESPONSE_TABLE



void UglyEchoClient::handler1( const stem::Event& ev )
{
  lock.lock();
  EXAM_CHECK_ASYNC( ev.value() == mess );
  
  if (ev.value() != mess ) {
    failflag = true;
    cout << mess << "\n:\n" << ev.value() << endl;
    for ( int i = 0; i < min(ev.value().size(), mess.size()); ++i ) {
      if ( ev.value()[i] != mess[i] ) {
        cout << i << endl;
        cout << mess.substr(i) << "\n\n" << ev.value().substr(i) << endl;
        break;
      }
    }
  }
  lock.unlock();
 
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );

  if ( failflag ) {
    cout << rsp_count << endl;
  }

  ++rsp_count;

  cnd.notify_one();
}

bool UglyEchoClient::wait()
{
  std::tr2::unique_lock<std::tr2::mutex> lk( mtx );

  bool result = cnd.timed_wait(lk, std::tr2::milliseconds( 1000 ), rsp_count_8( *this ) );

  if ( result ) {
    rsp_count = 0;
  }

  bool localflag = failflag;
  failflag = false;
  
  return result && !localflag;
}

DEFINE_RESPONSE_TABLE( UglyEchoClient )
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

EchoLast::EchoLast( const char* info ) :
    EventHandler( info )
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
  // cerr << "Echo\n";

 /*
  * Conformation required, if I want to indeed receive 'last'
  * message; it's required mainly for client side, to allow
  * client wait something.
  *
  * Otherwise, client side can close StEM transport channels
  * _before_ delivery event (but after accepting event for
  * delivery---event stored in queue).
  */

  Event conf( NODE_EV_LAST_CONFORMATION );

  conf.dest( ev.src() );

  Send( conf );

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

LastEvent::LastEvent( const char* info ) :
    EventHandler( info ),
    mess( info ),
    peer( stem::badaddr )
{
}

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

    // See comments in EchoLast::last above
    cnd_conf.timed_wait( std::tr2::milliseconds( 500 ) );
  }
}

void LastEvent::handler( const stem::Event& ev )
{
  EXAM_CHECK_ASYNC( ev.value() == mess );

  peer = ev.src();

  cnd.notify_one();
}

void LastEvent::conformation( const stem::Event& ev )
{
  // See comments in EchoLast::last above
  cnd_conf.notify_one();
}

bool LastEvent::wait()
{
  return cnd.timed_wait( std::tr2::milliseconds( 500 ) );
}

DEFINE_RESPONSE_TABLE( LastEvent )
  EV_EDS(0,NODE_EV_ECHO,handler)
  EV_EDS(0,NODE_EV_LAST_CONFORMATION,conformation)
END_RESPONSE_TABLE

ProxyEcho::ProxyEcho() :
  sender(0)
{
}

ProxyEcho::ProxyEcho( const char* info ) :
    EventHandler( info ),
    sender(0)
{
}

ProxyEcho::ProxyEcho( addr_type id ) :
    EventHandler( id ),
    sender(0)
{
}

ProxyEcho::ProxyEcho( addr_type id, const char* info ) :
    EventHandler( id, info ),
    sender(0)
{
}

ProxyEcho::~ProxyEcho()
{
  if (sender != 0) {
    delete sender;
  }
}

void ProxyEcho::proxy( const stem::Event& ev )
{
  if ( sender == 0 ) {
    sender = new MsgSender();
  }
  sender->echo( ev );
}

MsgSender::MsgSender()
{
}

void MsgSender::echo( const stem::Event& ev )
{
  Event rsp( ev.code() );
  rsp.dest( ev.src() );
  rsp.value() = ev.value();
  Send( rsp );
}

DEFINE_RESPONSE_TABLE( ProxyEcho )
  EV_EDS( 0, NODE_EV_ECHO, proxy )
END_RESPONSE_TABLE

