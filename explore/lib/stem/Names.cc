// -*- C++ -*- Time-stamp: <09/04/30 11:20:43 ptr>

/*
 * Copyright (c) 1997-1999, 2002-2003, 2005-2006, 2008-2009
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

#include <unistd.h>

#include "stem/Names.h"
#include "stem/EvManager.h"
#include "stem/EDSEv.h"

#include <list>
#include <iostream>

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

void __FIT_DECLSPEC Names::ns_list( const Event& rq )
{
  typedef NameRecords<addr_type,string> Seq;
  Event_base<Seq> rs( EV_STEM_NS_LIST );
  Seq::container_type& lst = rs.value().container;

  manager()->_lock_iheap.lock();
  for ( EvManager::info_heap_type::const_iterator i = manager()->iheap.begin(); i != manager()->iheap.end(); ++i ) {
    for ( std::list<addr_type>::const_iterator j = i->second.begin(); j != i->second.end(); ++j ) {
      lst.push_back( make_pair( *j, i->first ) );
    }
  }
  manager()->_lock_iheap.unlock();

  rs.dest( rq.src() );
  Send( rs );
}

void __FIT_DECLSPEC Names::ns_name( const Event& rq )
{
  typedef NameRecords<addr_type,string> Seq;
  Event_base<Seq> rs( EV_STEM_NS_NAME );
  Seq::container_type& lst = rs.value().container;

  manager()->_lock_iheap.lock();
  EvManager::info_heap_type::const_iterator i = manager()->iheap.find( rq.value() );
  if ( i != manager()->iheap.end() ) {
    for ( std::list<addr_type>::const_iterator j = i->second.begin(); j != i->second.end(); ++j ) {
      lst.push_back( make_pair( *j, i->first ) );
    }
  }
  manager()->_lock_iheap.unlock();

  rs.dest( rq.src() );
  Send( rs );
}

DEFINE_RESPONSE_TABLE( Names )
  EV_EDS(ST_NULL,EV_STEM_GET_NS_LIST,ns_list)
  EV_EDS(ST_NULL,EV_STEM_GET_NS_NAME,ns_name)
END_RESPONSE_TABLE

__FIT_DECLSPEC
void NameRecord::pack( std::ostream& s ) const
{
  __pack( s, addr );
  __pack( s, record );
}

__FIT_DECLSPEC
void NameRecord::unpack( std::istream& s )
{
  __unpack( s, addr );
  __unpack( s, record );
}

} // namespace stem
