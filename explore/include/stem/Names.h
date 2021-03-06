// -*- C++ -*- Time-stamp: <09/04/30 10:47:30 ptr>

/*
 * Copyright (c) 1997-1999, 2002-2003, 2005-2006, 2009
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

#include <config/feature.h>

#include <string>
#include <iosfwd>
#include <stdint.h>

#include <stem/EventHandler.h>

#include <stem/EvPack.h>

namespace stem {

class Names :
    public EventHandler
{
  public:
    Names();
    explicit Names( const char * );
    explicit Names( addr_type id, const char *info = 0 );
    ~Names();

    void ns_list( const Event& );
    void ns_name( const Event& );

  private:
    DECLARE_RESPONSE_TABLE( Names, EventHandler );
};

struct NameRecord :
   public __pack_base
{
    NameRecord()
      { }

    NameRecord( const NameRecord& nr ) :
        addr( nr.addr ),
        record( nr.record )
      { }

    NameRecord( const addr_type& a, const std::string& r ) :
        addr( a ),
        record( r )
      { }

    addr_type    addr;
    std::string  record;

    virtual __FIT_DECLSPEC void pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void unpack( std::istream& s );
};

inline bool operator == ( const NameRecord& nr, const std::string& s )
{ return nr.record == s; }

inline bool operator == ( const NameRecord& nr, const addr_type& a )
{ return nr.addr == a; }

template <class Addr, class Info, class Sequence = std::list< std::pair<Addr, Info> > >
struct NameRecords :
   public __pack_base
{
    NameRecords()
      { }

    NameRecords( const NameRecords& nr ) :
        container( nr.container )
      { }

    typedef Sequence container_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    typedef Addr address_type;
    typedef Info name_type;

    container_type container;

    virtual void pack( std::ostream& s ) const;
    virtual void unpack( std::istream& s );
};

template <class Addr, class Info, class Sequence>
void NameRecords<Addr,Info,Sequence>::pack( std::ostream& s ) const
{
  __pack( s, static_cast<uint32_t>(container.size()) );
  for ( const_iterator i = container.begin(); i != container.end(); ++i ) {
    __pack( s, i->first );
    __pack( s, i->second );
  }
}

template <class Addr, class Info, class Sequence>
void NameRecords<Addr,Info,Sequence>::unpack( std::istream& s )
{
  uint32_t sz;
  container.clear();

  __unpack( s, sz );
  Addr a;
  Info i;

  while ( sz-- > 0 ) {
    __unpack( s, a );
    __unpack( s, i );
    container.push_back( std::make_pair(a, i) );
  }
}

} // namespace stem

#endif // __Names_h
