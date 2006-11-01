// -*- C++ -*- Time-stamp: <06/10/04 09:51:02 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include "stem/Names.h"
#include "stem/EvManager.h"
#include "stem/EDSEv.h"
#include <list>

namespace stem {

using namespace std;

__FIT_DECLSPEC Names::Names() :
    EventHandler()
{
}

__FIT_DECLSPEC Names::Names( const char *info ) :
    EventHandler( info )
{
}

__FIT_DECLSPEC Names::Names( addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

__FIT_DECLSPEC Names::~Names()
{
}

void __FIT_DECLSPEC Names::get_list( const Event& rq )
{
  typedef NameRecords<addr_type,string> Seq;
  Event_base<Seq> rs( EV_STEM_NS1_LIST );
  Seq::container_type& lst = rs.value().container;

  manager()->_lock_heap.lock();
  for ( EvManager::heap_type::iterator i = manager()->heap.begin(); i != manager()->heap.end(); ++i ) {
    if ( ((*i).first & extbit) == 0 ) { // only local...
      lst.push_back( make_pair((*i).first, (*i).second.info) );
    }
  }
  manager()->_lock_heap.unlock();

  if ( rq.code() == EV_STEM_RQ_ADDR_LIST1 ) {
    rs.dest( rq.src() );
    Send( rs );
  } else {
    Event_base<NameRecord> rs_( EV_EDS_NM_LIST );

    rs_.dest( rq.src() );
    for ( Seq::const_iterator i = lst.begin(); i != lst.end(); ++i ) {
      rs_.value().addr = i->first;
      rs_.value().record = i->second;
      Send( Event_convert<NameRecord>()( rs_ ) );
    }
    // end of table
    rs_.value().addr = badaddr;
    rs_.value().record.clear();
    Send( Event_convert<NameRecord>()( rs_ ) );
  }
}

void __FIT_DECLSPEC Names::get_ext_list( const Event& rq )
{
  typedef NameRecords<addr_type,string> Seq;
  Event_base<Seq> rs( EV_STEM_NS1_LIST );
  Seq::container_type& lst = rs.value().container;

  manager()->_lock_heap.lock();
  for ( EvManager::heap_type::const_iterator i = manager()->heap.begin(); i != manager()->heap.end(); ++i ) {
    if ( ((*i).first & extbit) != 0 ) { // only external...
      lst.push_back( make_pair((*i).first, (*i).second.info) );
    }
  }
  manager()->_lock_heap.unlock();

  if ( rq.code() == EV_STEM_RQ_EXT_ADDR_LIST1 ) {
    rs.dest( rq.src() );
    Send( rs );
  } else {
    Event_base<NameRecord> rs_( EV_EDS_NM_LIST );

    rs_.dest( rq.src() );
    for ( Seq::const_iterator i = lst.begin(); i != lst.end(); ++i ) {
      rs_.value().addr = i->first;
      rs_.value().record = i->second;
      Send( Event_convert<NameRecord>()( rs_ ) );
    }
    // end of table
    rs_.value().addr = badaddr;
    rs_.value().record.clear();
    Send( Event_convert<NameRecord>()( rs_ ) );
  }
}

void __FIT_DECLSPEC Names::get_by_name( const Event& rq )
{
  typedef NameRecords<addr_type,string> Seq;
  Event_base<Seq> rs( EV_STEM_NS1_NAME );
  Seq::container_type& lst = rs.value().container;

  manager()->_lock_heap.lock();
  for ( EvManager::heap_type::const_iterator i = manager()->heap.begin(); i != manager()->heap.end(); ++i ) {
    if ( ((*i).first & extbit) == 0 && (*i).second.info == rq.value() ) { // only local...
      lst.push_back( make_pair((*i).first, (*i).second.info) );
    }
  }
  manager()->_lock_heap.unlock();

  if ( rq.code() == EV_STEM_RQ_ADDR_BY_NAME1 ) {
    rs.dest( rq.src() );
    Send( rs );
  } else {
    Event_base<NameRecord> rs_( EV_EDS_NS_ADDR );

    rs_.dest( rq.src() );
    for ( Seq::const_iterator i = lst.begin(); i != lst.end(); ++i ) {
      rs_.value().addr = i->first;
      rs_.value().record = i->second;
      Send( Event_convert<NameRecord>()( rs_ ) );
    }
    // end of table
    rs_.value().addr = badaddr;
    rs_.value().record.clear();
    Send( Event_convert<NameRecord>()( rs_ ) );
  }
}

DEFINE_RESPONSE_TABLE( Names )
  EV_EDS(ST_NULL,EV_EDS_RQ_ADDR_LIST,get_list)
  EV_EDS(ST_NULL,EV_STEM_RQ_ADDR_LIST1,get_list)
  EV_EDS(ST_NULL,EV_EDS_RQ_EXT_ADDR_LIST,get_ext_list)
  EV_EDS(ST_NULL,EV_STEM_RQ_EXT_ADDR_LIST1,get_ext_list)
  EV_EDS(ST_NULL,EV_EDS_RQ_ADDR_BY_NAME,get_by_name)
  EV_EDS(ST_NULL,EV_STEM_RQ_ADDR_BY_NAME1,get_by_name)
END_RESPONSE_TABLE

__FIT_DECLSPEC
void NameRecord::pack( std::ostream& s ) const
{
  __pack( s, addr );
  __pack( s, record );
}

__FIT_DECLSPEC
void NameRecord::net_pack( std::ostream& s ) const
{
  __net_pack( s, addr );
  __net_pack( s, record );
}

__FIT_DECLSPEC
void NameRecord::unpack( std::istream& s )
{
  __unpack( s, addr );
  __unpack( s, record );
}

__FIT_DECLSPEC
void NameRecord::net_unpack( std::istream& s )
{
  __net_unpack( s, addr );
  __net_unpack( s, record );
}

} // namespace stem
