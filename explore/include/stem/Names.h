// -*- C++ -*- Time-stamp: <99/09/10 16:03:21 ptr>
#ifndef __Names_h
#define __Names_h

#ident "$SunId$ %Q%"

#include <string>
#include <iosfwd>

#include <EventHandler.h>
#include <EvPack.h>

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
