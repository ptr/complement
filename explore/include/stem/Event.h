// -*- C++ -*- Time-stamp: <00/05/26 10:56:40 ptr>

/*
 * Copyright (c) 1999-2000
 * ParallelGraphics
 *
 * Copyright (c) 1995-1999
 * Petr Ovchenkov
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

#ifndef __EDS_Event_h
#define __EDS_Event_h

#ident "$SunId$"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <istream>
#include <ostream>

#ifdef __SGI_STL_PORT
#  if __SGI_STL_PORT < 0x322
#    include <type_traits.h>
#  else
#    include <stl/type_traits.h>
#  endif
#endif

#include <sstream>

#ifndef __EvPack_h
#include <EDS/EvPack.h>
#endif

namespace EDS {

typedef unsigned addr_type;
typedef unsigned code_type;
typedef unsigned key_type;

#ifndef WIN32
extern const addr_type badaddr;
extern const addr_type extbit;
extern const addr_type nsaddr;
extern const key_type  badkey;
extern const code_type badcode;
#endif

#ifdef WIN32
extern __PG_DECLSPEC addr_type badaddr;
extern __PG_DECLSPEC addr_type extbit;
extern __PG_DECLSPEC addr_type nsaddr;
extern __PG_DECLSPEC key_type  badkey;
extern __PG_DECLSPEC code_type badcode;
#endif

class __Event_Base
{
  public:
    typedef size_t   size_type;

    __Event_Base() :
        _code( badcode ),
        _dst( badaddr ),
        _src( badaddr )
      { }

    explicit __Event_Base( code_type c ) :
        _code( c ),
        _dst( badaddr ),
        _src( badaddr )
      { }

    /* explicit */ __Event_Base( const __Event_Base& e ) :
        _code( e._code ),
        _dst( e._dst ),
        _src( e._src )
      { }

    code_type code() const
      { return _code; }
    key_type dest() const
      { return _dst; }
    key_type src() const
      { return _src; }
    bool is_from_foreign() const
      { return ((_src & extbit) != 0) && (_src != badaddr); }
    bool is_to_foreign() const
      { return ((_dst & extbit) != 0) && (_dst != badaddr); }

    void code( code_type c ) const
      { _code = c; }
    void dest( addr_type c ) const
      { _dst = c; }
    void src( addr_type c ) const
      { _src = c; }

  protected:
    mutable code_type _code; // event code
    mutable addr_type  _dst;  // destination
    mutable addr_type  _src;  // source

    friend class NetTransport_base;
    friend class NetTransportMgr;
};

#if defined(_MSC_VER) && !defined(_DEBUG)  // workaround for VC 5.0 / Release
} // namespace EDS
typedef EDS::__Event_Base __Event_Base;
namespace EDS {
#endif

// Forward declarations

template <class D> class Event_base;
// VC 5.0 to be very huffy on typedefed std::string...
#ifndef _MSC_VER
__STL_TEMPLATE_NULL class Event_base<__STD::string>;
#else
__STL_TEMPLATE_NULL
class Event_base<__STD::basic_string<char, __STD::char_traits<char>, __STD::allocator<char> > >;
#endif
__STL_TEMPLATE_NULL class Event_base<void>;

// Typedefs:

typedef Event_base<void>        EventVoid;
typedef Event_base<__STD::string> EventStr;
      // Today same, the basic of Event transport/conversions:
typedef Event_base<__STD::string> Event;

/* ******************************************** *\
   Any class to be passed as parameter
   to Event_base should be like:

   struct auth_resp :
     public EDS::__pack_base
   {
      // unsigned result;
      // unsigned sessionID;
      // unsigned flags;

      virtual void pack( __STD::ostream& s ) const;
      virtual void net_pack( __STD::ostream& s ) const;
      virtual void unpack( __STD::istream& s );
      virtual void net_unpack( __STD::istream& s );
   };

   Of cause, if it not a POD type, __STD::string
   or void.

   To pack/unpuck POD members, use static functions of
   __pack_base (EvPack.h).

\* ******************************************** */

template <class D>
class Event_base :
        public __Event_Base
{
  public:
    typedef D         value_type;
    typedef D&        reference;
    typedef const D&  const_reference;
    typedef D *       pointer;
    typedef const D * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const D& d ) :
        __Event_Base( c ),
        _data( d )
      { }

// #ifndef __GNUG__ // otherwise gcc can't return structure
//    explicit
// #endif
    Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        __STD::ostringstream ss;
        net_pack( ss );
        s.value() = ss.str();
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        __STD::stringstream ss( s.value() );
        net_unpack( ss );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        __STD::stringstream ss;
        pack( ss );
        s.value() = ss.str();
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        __STD::stringstream ss( s.value() );
        unpack( ss );
      }

#ifndef _MSC_VER
    void pack( __STD::ostream& __s ) const
      { pack( __s, __type_traits<D>::is_POD_type() ); }
    void unpack( __STD::istream& __s )
      { unpack( __s, __type_traits<D>::is_POD_type() ); }
    void net_pack( __STD::ostream& __s ) const
      { net_pack( __s, __type_traits<D>::is_POD_type() ); }
    void net_unpack( __STD::istream& __s )
      { net_unpack( __s, __type_traits<D>::is_POD_type() ); }
#else
// VC instantiate only whole class, so I need stupid specializaton for it,
// and this template can be compiled for non-POD classes only
// (specialization for integral types in separate file, included below)
    void pack( __STD::ostream& __s ) const
      { _data.pack( __s ); }
    void unpack( __STD::istream& __s )
      { _data.unpack( __s ); }
    void net_pack( __STD::ostream& __s ) const
      { _data.net_pack( __s ); }
    void net_unpack( __STD::istream& __s )
      { _data.net_unpack( __s ); }
#endif

  protected:
    value_type _data;

#ifndef _MSC_VER
    void pack( __STD::ostream& __s, __true_type ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }

    void pack( __STD::ostream& __s, __false_type ) const
      { _data.pack( __s ); }

    void unpack( __STD::istream& __s, __true_type )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void unpack( __STD::istream& __s, __false_type )
      { _data.unpack( __s ); }

    void net_pack( __STD::ostream& __s, __true_type ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_pack( __STD::ostream& __s, __false_type ) const
      { _data.net_pack( __s ); }
    void net_unpack( __STD::istream& __s, __true_type )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }
    void net_unpack( __STD::istream& __s, __false_type )
      { _data.net_unpack( __s ); }
#endif
};

// VC 5.0 to be very huffy on typedefed __STD::string...
__STL_TEMPLATE_NULL
#ifndef _MSC_VER
class Event_base<__STD::string> :
#else
class Event_base<__STD::basic_string<char, __STD::char_traits<char>, __STD::allocator<char> > > :
#endif
        public __Event_Base
{
  public:
    typedef __STD::string         value_type;
    typedef __STD::string&        reference;
    typedef const __STD::string&  const_reference;
    typedef __STD::string *       pointer;
    typedef const __STD::string * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const __STD::string& d ) :
        __Event_Base( c ),
        _data( d )
      { }

#if !defined( __GNUG__ ) && !defined( _MSC_VER ) // otherwise gcc can't return structure
//    explicit
#endif
    Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    size_type value_size() const
      { return _data.size(); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.value() = _data;
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _data = s.value();
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.value() = _data;
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _data = s.value();
      }

    void pack( __STD::ostream& __s ) const
      { __pack_base::__pack( __s, _data ); }
    void unpack( __STD::istream& __s )
      { __pack_base::__unpack( __s, _data ); }
    void net_pack( __STD::ostream& __s ) const
      { __pack_base::__net_pack( __s, _data ); }
    void net_unpack( __STD::istream& __s )
      { __pack_base::__net_unpack( __s, _data ); }

  protected:
    value_type _data;
};

__STL_TEMPLATE_NULL
class Event_base<void> :
        public __Event_Base
{
  public:
    typedef void         value_type;
    typedef void *       pointer;
    typedef size_t       size_type;
    typedef const void * const_pointer;

    Event_base() :
        __Event_Base()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e )
      { }

    size_type value_size() const
      { return 0; }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.value().erase();
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.value().erase();
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
      }

    void pack( __STD::ostream& ) const
      { }
    void unpack( __STD::istream& )
      { }
    void net_pack( __STD::ostream& ) const
      { }
    void net_unpack( __STD::istream& )
      { }
};

// VC instantiate only whole class, so I need stupid specializaton for it:
#ifdef _MSC_VER
#include <EDS/EventSpec.h>
#endif

} // namespace EDS

#endif
