// -*- C++ -*- Time-stamp: <06/11/27 18:45:00 ptr>

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

#include <unistd.h>

#include "stem/Names.h"
#include "stem/EvManager.h"
#include "stem/EDSEv.h"

#include <list>
#include <iostream>

#include <mt/xmt.h>

namespace stem {

using namespace std;
using namespace xmt;

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
  typedef NameRecords<gaddr_type,string> Seq;
  Event_base<Seq> rs( EV_STEM_NS_LIST );
  Seq::container_type& lst = rs.value().container;

  manager()->_lock_xheap.lock();
  manager()->_lock_iheap.lock();
  for ( EvManager::uuid_ext_heap_type::const_iterator i = manager()->_ui_heap.begin(); i != manager()->_ui_heap.end(); ++i ) {
    EvManager::info_heap_type::const_iterator j = manager()->iheap.find( i->second );
    if ( j != manager()->iheap.end() ) {
      lst.push_back( make_pair( i->first, j->second ) );
    } else {
      lst.push_back( make_pair( i->first, string() ) );
    }
  }
  manager()->_lock_iheap.unlock();
  manager()->_lock_xheap.unlock();

  rs.dest( rq.src() );
  Send( rs );
}

void __FIT_DECLSPEC Names::ns_name( const Event& rq )
{
  cerr << __FILE__ << ":" << __LINE__ << endl;
  typedef NameRecords<gaddr_type,string> Seq;
  Event_base<Seq> rs( EV_STEM_NS_NAME );
  Seq::container_type& lst = rs.value().container;

  manager()->_lock_iheap.lock();
  for ( EvManager::info_heap_type::const_iterator i = manager()->iheap.begin(); i != manager()->iheap.end(); ++i ) {    
    if ( i->second == rq.value() ) {
      if ( /* i->first & extbit */ true ) {
        Locker lk( manager()->_lock_xheap );
        EvManager::ext_uuid_heap_type::const_iterator j = manager()->_ex_heap.find( i->first );
        if ( j != manager()->_ex_heap.end() ) {
          lst.push_back( make_pair( j->second, i->second ) );
        }
      } else {
        Locker lk( manager()->_lock_heap );
        EvManager::local_heap_type::const_iterator j = manager()->heap.find( i->first );
        if ( j != manager()->heap.end() ) {
          gaddr_type gaddr;
          gaddr.hid = xmt::hostid();
          gaddr.pid = getpid();
          gaddr.addr = j->first;
          lst.push_back( make_pair( gaddr, i->second ) );
        }
      }
    }
  }
  manager()->_lock_iheap.unlock();

  cerr << __FILE__ << ":" << __LINE__ << endl;
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
  addr.pack( s );
  __pack( s, record );
}

__FIT_DECLSPEC
void NameRecord::net_pack( std::ostream& s ) const
{
  addr.net_pack( s );
  __net_pack( s, record );
}

__FIT_DECLSPEC
void NameRecord::unpack( std::istream& s )
{
  addr.unpack( s );
  __unpack( s, record );
}

__FIT_DECLSPEC
void NameRecord::net_unpack( std::istream& s )
{
  addr.net_unpack( s );
  __net_unpack( s, record );
}

} // namespace stem
