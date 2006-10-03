// -*- C++ -*- Time-stamp: <06/10/03 10:55:31 ptr>

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

#ifndef __stem_Names_h
#define __stem_Names_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <iosfwd>
#include <functional>

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

    NameRecord( addr_type a, const std::string& r ) :
        addr( a ),
        record( r )
      { }

    addr_type    addr;
    std::string  record;

    virtual __FIT_DECLSPEC void pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void net_pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void unpack( std::istream& s );
    virtual __FIT_DECLSPEC void net_unpack( std::istream& s );
};

inline bool operator == ( const NameRecord& nr, const std::string& s )
{ return nr.record == s; }

// bool operator == ( const std::string& s, const NameRecord& nr )
// { return nr.record == s; }

inline bool operator == ( const NameRecord& nr, addr_type a )
{ return nr.addr == a; }

// bool operator == ( addr_type a, const NameRecord& nr )
// { return nr.addr == a; }

} // namespace stem

#endif // __Names_h
