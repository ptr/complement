// -*- C++ -*- Time-stamp: <00/02/24 19:37:34 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ident "$SunId$ %Q%"

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include "EDS/Names.h"
#include "EDS/EvManager.h"
#include "EDS/EDSEv.h"

namespace EDS {

__PG_DECLSPEC Names::Names() :
    EventHandler()
{
}

__PG_DECLSPEC Names::Names( const char *info ) :
    EventHandler( info )
{
}

__PG_DECLSPEC Names::Names( addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

void __PG_DECLSPEC Names::get_list( const Event& rq )
{
  Event_base<NameRecord> rs( EV_EDS_NM_LIST );

  rs.dest( rq.src() );

  MT_REENTRANT( manager()->_lock_heap, _1 );
  EvManager::heap_type::iterator i = manager()->heap.begin();
  while ( i != manager()->heap.end() ) {
    if ( ((*i).first & extbit) == 0 ) { // only local...
      rs.value().addr = (*i).first;
      rs.value().record = (*i).second.info;

      Send( Event_convert<NameRecord>()( rs ) );
    }
    ++i;
  }

  // end of table
  rs.value().addr = badaddr;
  rs.value().record.clear();
  Send( Event_convert<NameRecord>()( rs ) );
}

void __PG_DECLSPEC Names::get_ext_list( const Event& rq )
{
  Event_base<NameRecord> rs( EV_EDS_NM_LIST );

  rs.dest( rq.src() );

  MT_REENTRANT( manager()->_lock_heap, _1 );
  EvManager::heap_type::iterator i = manager()->heap.begin();
  while ( i != manager()->heap.end() ) {
    if ( ((*i).first & extbit) != 0 ) { // only external...
      rs.value().addr = (*i).first;
      rs.value().record = (*i).second.info;

      Send( Event_convert<NameRecord>()( rs ) );
    }
    ++i;
  }

  // end of table
  rs.value().addr = badaddr;
  rs.value().record.clear();
  Send( Event_convert<NameRecord>()( rs ) );
}

void __PG_DECLSPEC Names::get_by_name( const Event& rq )
{
  Event_base<NameRecord> rs( EV_EDS_NS_ADDR );

  rs.dest( rq.src() );

  MT_REENTRANT( manager()->_lock_heap, _1 );
  EvManager::heap_type::iterator i = manager()->heap.begin();
  while ( i != manager()->heap.end() ) {
    if ( ((*i).first & extbit) == 0 && (*i).second.info == rq.value() ) { // only local...
      rs.value().addr = (*i).first;
      rs.value().record = (*i).second.info;

      Send( Event_convert<NameRecord>()( rs ) );
      return;
    }
    ++i;
  }

  // end of table
  rs.value().addr = badaddr;
  rs.value().record.clear();
  Send( Event_convert<NameRecord>()( rs ) );
}

__PG_DECLSPEC
void NameRecord::pack( __STD::ostream& s ) const
{
  __pack( s, addr );
  __pack( s, record );
}

__PG_DECLSPEC
void NameRecord::net_pack( __STD::ostream& s ) const
{
  __net_pack( s, addr );
  __net_pack( s, record );
}

__PG_DECLSPEC
void NameRecord::unpack( __STD::istream& s )
{
  __unpack( s, addr );
  __unpack( s, record );
}

__PG_DECLSPEC
void NameRecord::net_unpack( __STD::istream& s )
{
  __net_unpack( s, addr );
  __net_unpack( s, record );
}

DEFINE_RESPONSE_TABLE( Names )
  EV_EDS(ST_NULL,EV_EDS_RQ_ADDR_LIST,get_list)
  EV_EDS(ST_NULL,EV_EDS_RQ_EXT_ADDR_LIST,get_ext_list)
  EV_EDS(ST_NULL,EV_EDS_RQ_ADDR_BY_NAME,get_by_name)
END_RESPONSE_TABLE

} // namespace EDS
