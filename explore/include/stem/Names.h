// -*- C++ -*- Time-stamp: <99/10/13 17:35:55 ptr>

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

#ifndef __Names_h
#define __Names_h

#ident "$SunId$ %Q%"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>

#ifndef __IOSFWD__
#include <iosfwd>
#endif

#ifndef __EventHandler_h
#include <EDS/EventHandler.h>
#endif

#ifndef __EvPack_h
#include <EDS/EvPack.h>
#endif

#ifndef __EDS_DLL
#  if defined( WIN32 ) && defined( _MSC_VER )
#    define __EDS_DLL __declspec( dllimport )
#  else
#    define __EDS_DLL
#  endif
#endif

#if defined( WIN32 ) && defined( _MSC_VER )
#  undef __EDS_DLL_EXPORT
#  define __EDS_DLL_EXPORT __EDS_DLL
#endif

namespace EDS {

class Names :
    public EventHandler
{
  public:
    __EDS_DLL Names();
    explicit __EDS_DLL Names( const char * );
    explicit __EDS_DLL Names( addr_type id, const char *info = 0 );

    __EDS_DLL void get_list( const Event& );
    __EDS_DLL void get_ext_list( const Event& );
    __EDS_DLL void get_by_name( const Event& );

  private:
    DECLARE_RESPONSE_TABLE( Names, EventHandler );
};

struct NameRecord :
   public __pack_base
{
    NameRecord() :
        addr( badaddr )
      { }

    NameRecord( const NameRecord& nr ) :
        addr( nr.addr ),
        record( nr.record )
      { }

    addr_type    addr;
    std::string  record;

    virtual __EDS_DLL void pack( std::ostream& s ) const;
    virtual __EDS_DLL void net_pack( std::ostream& s ) const;
    virtual __EDS_DLL void unpack( std::istream& s );
    virtual __EDS_DLL void net_unpack( std::istream& s );
};

} // namespace EDS

#if defined( WIN32 ) && defined( _MSC_VER )
#  undef __EDS_DLL_EXPORT
#  define __EDS_DLL_EXPORT
#endif

#endif // __Names_h
