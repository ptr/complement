// -*- C++ -*- Time-stamp: <06/09/29 18:40:31 ptr>

#include "mprocessor.h"
#include <iostream>

namespace test {

using namespace stem;
using namespace std;
using namespace xmt;

MProcessor::MProcessor( const char *srv_name ) :
    EventHandler( srv_name )
{
  cnd.set( false );
}

void MProcessor::send( stem::addr_type addr, const char *msg )
{
  cerr << hex << (unsigned)get_ev_table_decl() << dec << endl;
  Event ev( /* 0x7200 */ 0x5000 );

  ev.dest( addr );
  ev.value() = msg;

  DispatchTrace( ev, cerr );
  Trace( cerr );
  Send( ev );
}

void MProcessor::send( stem::addr_type addr, const char *msg, size_t len )
{
  Event ev( /* 0x7200 */ 0x5000 );

  ev.dest( addr );
  ev.value().assign( msg, len );

  Send( ev );
}

void MProcessor::receive( const stem::Event& ev )
{
  cerr << "I see event " << ev.value() << endl;
  MT_REENTRANT( lock, _1 );

  income_queue.push_back( ev );
  cnd.set( true );
  cerr << "I see event " << ev.value() << endl;
}

const char *MProcessor::get()
{
  MT_REENTRANT( lock, _1 );
  if ( !income_queue.empty() ) {
    last = income_queue.front();
    income_queue.pop_front();

    if ( income_queue.empty() ) {
      cnd.set( false );
    }

    return last.value().c_str();
  }
  return 0;
}

const char *MProcessor::getbinary( unsigned& len )
{
  MT_REENTRANT( lock, _1 );
  if ( !income_queue.empty() ) {
    last = income_queue.front();
    income_queue.pop_front();

    if ( income_queue.empty() ) {
      cnd.set( false );
    }

    len = last.value().size();

    return last.value().c_str();
  }
  len = 0;
  return 0;
}

const char *MProcessor::get( unsigned tv_sec, unsigned tv_nsec )
{
  timespec t;
  t.tv_sec = tv_sec;
  t.tv_nsec = tv_nsec;

  cnd.try_wait_delay( &t );
  return get();
}

DEFINE_RESPONSE_TABLE( MProcessor )
  EV_EDS(0, /* 0x7200 */ 0x5000, MProcessor::receive)
END_RESPONSE_TABLE

}
