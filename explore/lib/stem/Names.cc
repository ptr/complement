// -*- C++ -*- Time-stamp: <99/10/20 11:24:21 ptr>

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

#ifdef WIN32
#  ifdef _DLL
#    define __EDS_DLL __declspec( dllexport )
#  else
#    define __EDS_DLL
#  endif
#else
#  define __EDS_DLL
#endif

#include <config/feature.h>
#include "EDS/Names.h"
#include "EDS/EvManager.h"
#include "EDS/EDSEv.h"

#if defined( WIN32 ) && defined( _MSC_VER )
#  undef __EDS_DLL_EXPORT
#  define __EDS_DLL_EXPORT __EDS_DLL
#endif

namespace EDS {

__EDS_DLL Names::Names() :
    EventHandler()
{
}

__EDS_DLL Names::Names( const char *info ) :
    EventHandler( info )
{
}

__EDS_DLL Names::Names( addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

void __EDS_DLL Names::get_list( const Event& rq )
{
  Event_base<NameRecord> rs( EV_EDS_NM_LIST );

  rs.dest( rq.src() );

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

void __EDS_DLL Names::get_ext_list( const Event& rq )
{
  Event_base<NameRecord> rs( EV_EDS_NM_LIST );

  rs.dest( rq.src() );

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

void __EDS_DLL Names::get_by_name( const Event& rq )
{
  Event_base<NameRecord> rs( EV_EDS_NS_ADDR );

  rs.dest( rq.src() );

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

__EDS_DLL
void NameRecord::pack( std::ostream& s ) const
{
  __pack( s, addr );
  __pack( s, record );
}

__EDS_DLL
void NameRecord::net_pack( std::ostream& s ) const
{
  __net_pack( s, addr );
  __net_pack( s, record );
}

__EDS_DLL
void NameRecord::unpack( std::istream& s )
{
  __unpack( s, addr );
  __unpack( s, record );
}

__EDS_DLL
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

} // namespace EDS
