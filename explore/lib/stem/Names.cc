// -*- C++ -*- Time-stamp: <99/09/08 14:39:32 ptr>
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

#include <Names.h>
#include <EvManager.h>
#include <EDSEv.h>

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
END_RESPONSE_TABLE

} // namespace EDS
