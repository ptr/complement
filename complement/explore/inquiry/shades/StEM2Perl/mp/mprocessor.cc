// -*- C++ -*- Time-stamp: <06/10/03 17:47:37 ptr>

#include "mprocessor.h"
#include <iostream>
#include <stem/EDSEv.h>


namespace test {

using namespace stem;
using namespace std;
using namespace xmt;

NsWrapper::NsWrapper()
{
}

void NsWrapper::ask_names( addr_type nsaddr )
{
  if ( nsaddr != badaddr ) {
    lcnd.set( false );
    lst.clear();

    Event ev( EV_EDS_RQ_ADDR_LIST );
    // Event ev( EV_EDS_RQ_EXT_ADDR_LIST );
    ev.dest( nsaddr );

    Send( ev );
  }
}

const list<stem::NameRecord>& NsWrapper::names()
{
  timespec t;
  t.tv_sec = 2;
  t.tv_nsec = 0;

  lcnd.try_wait_delay( &t );

  return lst;
}

void NsWrapper::names_list( const stem::NameRecord& nr )
{
  if ( nr.addr == stem::badaddr ) { // the last record
    lcnd.set(true);
  } else {
    lst.push_back( nr );
    cerr << hex << nr.addr << " " << nr.record << endl;
  }
}

void NsWrapper::names_name( const stem::NameRecord& nr )
{
  if ( nr.addr == stem::badaddr ) { // the last record
    lcnd.set(true);
  } else {
    lst.push_back( nr );
  }
}

DEFINE_RESPONSE_TABLE( NsWrapper )
  EV_T_(0,EV_EDS_NM_LIST,names_list,stem::NameRecord)
  EV_T_(0,EV_EDS_NS_ADDR,names_name,stem::NameRecord)
END_RESPONSE_TABLE

MProcessor::MProcessor( const char *srv_name ) :
    EventHandler( srv_name )
{
  cnd.set( false );
}

void MProcessor::send( stem::addr_type addr, const char *msg )
{
  // cerr << hex << (unsigned)get_ev_table_decl() << dec << endl;
  // cerr << hex << addr << endl;
  Event ev( /* 0x7200 */ 0x5000 );

  ev.dest( addr );
  ev.value() = msg;

  // DispatchTrace( ev, cerr );
  // Trace( cerr );
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
  // cerr << "I see event " << ev.value() << endl;
  MT_REENTRANT( lock, _1 );

  income_queue.push_back( ev );
  cnd.set( true );
  // cerr << "I see event " << ev.value() << endl;
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

