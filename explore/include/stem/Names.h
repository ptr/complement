// -*- C++ -*- Time-stamp: <05/12/30 00:15:36 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003, 2005
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 2.1
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

#ifndef __stem_Names_h
#define __stem_Names_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>

#ifndef __IOSFWD__
#include <iosfwd>
#endif

#ifndef __stem_EventHandler_h
#include <stem/EventHandler.h>
#endif

#ifndef __stem_EvPack_h
#include <stem/EvPack.h>
#endif

namespace stem {

class Names :
    public EventHandler
{
  public:
    __FIT_DECLSPEC Names();
    explicit __FIT_DECLSPEC Names( const char * );
    explicit __FIT_DECLSPEC Names( addr_type id, const char *info = 0 );
    __FIT_DECLSPEC ~Names();

    __FIT_DECLSPEC void get_list( const Event& );
    __FIT_DECLSPEC void get_ext_list( const Event& );
    __FIT_DECLSPEC void get_by_name( const Event& );

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

    virtual __FIT_DECLSPEC void pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void net_pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void unpack( std::istream& s );
    virtual __FIT_DECLSPEC void net_unpack( std::istream& s );
};

} // namespace stem

#endif // __Names_h
