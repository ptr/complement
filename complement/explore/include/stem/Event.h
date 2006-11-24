// -*- C++ -*- Time-stamp: <06/11/24 20:57:55 ptr>

/*
 *
 * Copyright (c) 1995-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __stem_Event_h
#define __stem_Event_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <istream>
#include <ostream>
#include <utility>
#include <sstream>
#include <stdint.h>

#include <stem/EvPack.h>
#include <mt/uid.h>

#ifndef STLPORT
#include <bits/cpp_type_traits.h>

// libstdc++ v3, timestamp 20050519 (3.4.4) has __type_traits,
// libstdc++ v3, timestamp 20060306 (3.4.6) has __type_traits,
// while libstdc++ v3, 20050921 (4.0.2) not; use boost's staff instead
# if !defined(__GLIBCXX__) || (defined(__GNUC__) && (__GNUC__ > 3))
#include <boost/type_traits.hpp>

//bool to type
template <int _Is>
struct __bool2type
{ typedef __true_type _Ret; };

template <>
struct __bool2type<1> { typedef __true_type _Ret; };

template <>
struct __bool2type<0> { typedef __false_type _Ret; };

template <class _Tp>
struct __type_traits {
  enum { trivial_constructor = ::boost::has_trivial_constructor<_Tp>::value };
  typedef typename __bool2type<trivial_constructor>::_Ret has_trivial_default_constructor;

  enum { trivial_copy = ::boost::has_trivial_copy<_Tp>::value };
  typedef typename __bool2type<trivial_copy>::_Ret has_trivial_copy_constructor;

  enum { trivial_assign = ::boost::has_trivial_assign<_Tp>::value };
  typedef typename __bool2type<trivial_assign>::_Ret has_trivial_assignment_operator;

  enum { trivial_destructor = ::boost::has_trivial_destructor<_Tp>::value };
  typedef typename __bool2type<trivial_destructor>::_Ret has_trivial_destructor;

  enum { pod = ::boost::is_pod<_Tp>::value };
  typedef typename __bool2type<pod>::_Ret is_POD_type;
};
# else
#  include <bits/type_traits.h>
# endif // __GLIBCXX__

#define _STLP_TEMPLATE_NULL template <>

#endif

namespace stem {

typedef uint32_t addr_type;
typedef uint32_t code_type;
typedef uint32_t key_type;

#ifndef WIN32
extern const addr_type badaddr;
extern const addr_type extbit;
extern const addr_type ns_addr;
extern const key_type  badkey;
extern const code_type badcode;
#endif

#ifdef WIN32
extern __PG_DECLSPEC addr_type badaddr;
extern __PG_DECLSPEC addr_type extbit;
extern __PG_DECLSPEC addr_type ns_addr;
extern __PG_DECLSPEC key_type  badkey;
extern __PG_DECLSPEC code_type badcode;
#endif

#ifdef STLPORT
using std::__true_type;
using std::__false_type;
using std::__type_traits;
#else
using ::__true_type;
using ::__false_type;
using ::__type_traits;
#endif

struct gaddr_type :
        public __pack_base
{
    gaddr_type() :
        hid(),
        pid( -1 ),
        addr( badaddr )
      { }

    gaddr_type( const xmt::uuid_type& _hid, pid_t _pid, stem::addr_type _addr ) :
        hid( _hid ),
        pid( _pid ),
        addr( _addr )
      { }

    gaddr_type( const gaddr_type& g ) :
        hid( g.hid ),
        pid( g.pid ),
        addr( g.addr )
      { }

    xmt::uuid_type  hid;
    int64_t         pid; // pid_t defined as int, so it may be int64_t
    stem::addr_type addr;

    __FIT_DECLSPEC virtual void pack( std::ostream& ) const;
    __FIT_DECLSPEC virtual void unpack( std::istream& );
    __FIT_DECLSPEC virtual void net_pack( std::ostream& ) const;
    __FIT_DECLSPEC virtual void net_unpack( std::istream& );

    gaddr_type& operator =( const gaddr_type& g )
      { hid = g.hid; pid = g.pid; addr = g.addr; return *this; }

    bool operator ==( const gaddr_type& ga ) const
      { return hid == ga.hid && pid == ga.pid && addr == ga.addr; }
    bool operator !=( const gaddr_type& ga ) const
      { return hid != ga.hid || pid != ga.pid || addr != ga.addr; }
    __FIT_DECLSPEC bool operator <( const gaddr_type& ga ) const;

    
    __FIT_DECLSPEC void _xnet_pack( char *buf ) const;
    __FIT_DECLSPEC void _xnet_unpack( const char *buf );
};

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
} // namespace stem
typedef stem::__Event_Base __Event_Base;
namespace stem {
#endif

// Forward declarations

template <class D> class Event_base;
// VC 5.0 to be very huffy on typedefed std::string...
#ifndef _MSC_VER
_STLP_TEMPLATE_NULL class Event_base<std::string>;
#else
_STLP_TEMPLATE_NULL
class Event_base<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >;
#endif
_STLP_TEMPLATE_NULL class Event_base<void>;

// Typedefs:

typedef Event_base<void>        EventVoid;
typedef Event_base<std::string> EventStr;
      // Today same, the basic of Event transport/conversions:
typedef Event_base<std::string> Event;

/* ******************************************** *\
   Any class to be passed as parameter
   to Event_base should be like:

   struct auth_resp :
     public EDS::__pack_base
   {
      // unsigned result;
      // unsigned sessionID;
      // unsigned flags;

      virtual void pack( std::ostream& s ) const;
      virtual void net_pack( std::ostream& s ) const;
      virtual void unpack( std::istream& s );
      virtual void net_unpack( std::istream& s );
   };

   Of cause, if it not a POD type, std::string
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

    Event_base( code_type c, const D& d ) :
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

//#ifndef __FIT_TEMPLATE_FORWARD_BUG
#if 0
    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        std::ostringstream ss;
        net_pack( ss );
        s.value() = ss.str();
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        std::stringstream ss( s.value() );
        net_unpack( ss );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        std::stringstream ss;
        pack( ss );
        s.value() = ss.str();
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        std::stringstream ss( s.value() );
        unpack( ss );
      }
#else // __FIT_TEMPLATE_FORWARD_BUG
    void net_pack( Event& s ) const;
    void net_unpack( const Event& s );
    void pack( Event& s ) const;
    void unpack( const Event& s );
#endif

#ifndef _MSC_VER
    void pack( std::ostream& __s ) const
      { pack( __s, typename __type_traits<D>::is_POD_type() ); }
    void unpack( std::istream& __s )
      { unpack( __s, typename __type_traits<D>::is_POD_type() ); }
    void net_pack( std::ostream& __s ) const
      { net_pack( __s, typename __type_traits<D>::is_POD_type() ); }
    void net_unpack( std::istream& __s )
      { net_unpack( __s, typename __type_traits<D>::is_POD_type() ); }
#else
// VC instantiate only whole class, so I need stupid specializaton for it,
// and this template can be compiled for non-POD classes only
// (specialization for integral types in separate file, included below)
    void pack( std::ostream& __s ) const
      { _data.pack( __s ); }
    void unpack( std::istream& __s )
      { _data.unpack( __s ); }
    void net_pack( std::ostream& __s ) const
      { _data.net_pack( __s ); }
    void net_unpack( std::istream& __s )
      { _data.net_unpack( __s ); }
#endif

  protected:
    value_type _data;

#ifndef _MSC_VER
    void pack( std::ostream& __s, __true_type ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }

    void pack( std::ostream& __s, __false_type ) const
      { _data.pack( __s ); }

    void unpack( std::istream& __s, __true_type )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s, __false_type )
      { _data.unpack( __s ); }

    void net_pack( std::ostream& __s, __true_type ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_pack( std::ostream& __s, __false_type ) const
      { _data.net_pack( __s ); }
    void net_unpack( std::istream& __s, __true_type )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }
    void net_unpack( std::istream& __s, __false_type )
      { _data.net_unpack( __s ); }
#endif
};


// VC 5.0 to be very huffy on typedefed std::string...
_STLP_TEMPLATE_NULL
#ifndef _MSC_VER
class Event_base<std::string> :
#else
class Event_base<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > :
#endif
        public __Event_Base
{
  public:
    typedef std::string         value_type;
    typedef std::string&        reference;
    typedef const std::string&  const_reference;
    typedef std::string *       pointer;
    typedef const std::string * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    Event_base( code_type c, const std::string& d ) :
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

    void pack( std::ostream& __s ) const
      { __pack_base::__pack( __s, _data ); }
    void unpack( std::istream& __s )
      { __pack_base::__unpack( __s, _data ); }
    void net_pack( std::ostream& __s ) const
      { __pack_base::__net_pack( __s, _data ); }
    void net_unpack( std::istream& __s )
      { __pack_base::__net_unpack( __s, _data ); }

  protected:
    value_type _data;
};

//#ifdef __FIT_TEMPLATE_FORWARD_BUG
template <class D>
void Event_base<D>::net_pack( Event& s ) const
{
  s.code( _code );
  s.dest( _dst );
  s.src( _src );
  std::ostringstream ss;
  net_pack( ss );
  s.value() = ss.str();
}

template <class D>
void Event_base<D>::net_unpack( const Event& s )
{
  _code = s.code();
  _dst  = s.dest();
  _src  = s.src();
  std::istringstream ss( s.value() );
  net_unpack( ss );
}

template <class D>
void Event_base<D>::pack( Event& s ) const
{
  s.code( _code );
  s.dest( _dst );
  s.src( _src );
  std::ostringstream ss;
  pack( ss );
  s.value() = ss.str();
}

template <class D>
void Event_base<D>::unpack( const Event& s )
{
  _code = s.code();
  _dst  = s.dest();
  _src  = s.src();
  std::istringstream ss( s.value() );
  unpack( ss );
}
//#endif // __FIT_TEMPLATE_FORWARD_BUG

_STLP_TEMPLATE_NULL
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

    __FIT_EXPLICIT Event_base( const Event_base& e ) :
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

    void pack( std::ostream& ) const
      { }
    void unpack( std::istream& )
      { }
    void net_pack( std::ostream& ) const
      { }
    void net_unpack( std::istream& )
      { }
};

// VC instantiate only whole class, so I need stupid specializaton for it:
#ifdef _MSC_VER
#include <EDS/EventSpec.h>
#endif

} // namespace stem

namespace EDS = stem;

#endif
