// -*- C++ -*- Time-stamp: <00/09/10 15:36:33 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
 * ParallelGraphics Ltd.
 * 
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

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

namespace EDS {

class Names :
    public EventHandler
{
  public:
    __PG_DECLSPEC Names();
    explicit __PG_DECLSPEC Names( const char * );
    explicit __PG_DECLSPEC Names( addr_type id, const char *info = 0 );
    __PG_DECLSPEC ~Names();

    __PG_DECLSPEC void get_list( const Event& );
    __PG_DECLSPEC void get_ext_list( const Event& );
    __PG_DECLSPEC void get_by_name( const Event& );

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
    __STD::string  record;

    virtual __PG_DECLSPEC void pack( __STD::ostream& s ) const;
    virtual __PG_DECLSPEC void net_pack( __STD::ostream& s ) const;
    virtual __PG_DECLSPEC void unpack( __STD::istream& s );
    virtual __PG_DECLSPEC void net_unpack( __STD::istream& s );
};

} // namespace EDS

#endif // __Names_h
