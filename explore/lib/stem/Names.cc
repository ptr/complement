// -*- C++ -*- Time-stamp: <06/10/03 11:06:19 ptr>

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
  list<NameRecord> lst;
  {
    MT_REENTRANT( manager()->_lock_heap, _x1 );
    for ( EvManager::heap_type::iterator i = manager()->heap.begin(); i != manager()->heap.end(); ++i ) {
      if ( ((*i).first & extbit) == 0 ) { // only local...
        lst.push_back( NameRecord((*i).first, (*i).second.info ) );
      }
    }
  }

  Event_base<NameRecord> rs( EV_EDS_NM_LIST );

  rs.dest( rq.src() );
  for ( list<NameRecord>::const_iterator i = lst.begin(); i != lst.end(); ++i ) {
    rs.value() = *i;
    Send( Event_convert<NameRecord>()( rs ) );
  }
  // end of table
  rs.value().addr = badaddr;
  rs.value().record.clear();
  Send( Event_convert<NameRecord>()( rs ) );
}

void __FIT_DECLSPEC Names::get_ext_list( const Event& rq )
{
  list<NameRecord> lst;
  {
    MT_REENTRANT( manager()->_lock_heap, _x1 );
    for ( EvManager::heap_type::const_iterator i = manager()->heap.begin(); i != manager()->heap.end(); ++i ) {
      if ( ((*i).first & extbit) != 0 ) { // only external...
        lst.push_back( NameRecord((*i).first, (*i).second.info ) );
      }
    }
  }
  Event_base<NameRecord> rs( EV_EDS_NM_LIST );

  rs.dest( rq.src() );
  for ( list<NameRecord>::const_iterator i = lst.begin(); i != lst.end(); ++i ) {
    rs.value() = *i;
    Send( Event_convert<NameRecord>()( rs ) );
  }
  // end of table
  rs.value().addr = badaddr;
  rs.value().record.clear();
  Send( Event_convert<NameRecord>()( rs ) );
}

void __FIT_DECLSPEC Names::get_by_name( const Event& rq )
{
  list<NameRecord> lst;
  {
    MT_REENTRANT( manager()->_lock_heap, _x1 );
    for ( EvManager::heap_type::const_iterator i = manager()->heap.begin(); i != manager()->heap.end(); ++i ) {
      if ( ((*i).first & extbit) == 0 && (*i).second.info == rq.value() ) { // only local...
        lst.push_back( NameRecord((*i).first, (*i).second.info ) );
      }
    }
  }
  Event_base<NameRecord> rs( EV_EDS_NS_ADDR );

  rs.dest( rq.src() );
  for ( list<NameRecord>::const_iterator i = lst.begin(); i != lst.end(); ++i ) {
    rs.value() = *i;
    Send( Event_convert<NameRecord>()( rs ) );
  }
  // end of table
  rs.value().addr = badaddr;
  rs.value().record.clear();
  Send( Event_convert<NameRecord>()( rs ) );
}

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

DEFINE_RESPONSE_TABLE( Names )
  EV_EDS(ST_NULL,EV_EDS_RQ_ADDR_LIST,get_list)
  EV_EDS(ST_NULL,EV_EDS_RQ_EXT_ADDR_LIST,get_ext_list)
  EV_EDS(ST_NULL,EV_EDS_RQ_ADDR_BY_NAME,get_by_name)
END_RESPONSE_TABLE

} // namespace stem
