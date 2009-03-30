// -*- C++ -*- Time-stamp: <09/03/30 16:18:07 ptr>

/*
 *
 * Copyright (c) 1995-1999, 2002, 2003, 2005-2009
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

#include <misc/type_traits.h>
#include <stem/EvPack.h>
#include <mt/uid.h>
#include <mt/thread>

#ifdef STLPORT
#  include <unordered_map>
#  include <unordered_set>
// #  include <hash_map>
// #  include <hash_set>
// #  define __USE_STLPORT_HASH
#  define __USE_STLPORT_TR1
#else
#  if defined(__GNUC__) && (__GNUC__ < 4)
#    include <ext/hash_map>
#    include <ext/hash_set>
#    define __USE_STD_HASH
#  else
#    include <tr1/unordered_map>
#    include <tr1/unordered_set>
#    define __USE_STD_TR1
#  endif
#endif

namespace stem {

typedef uint32_t addr_type;
typedef uint32_t code_type;

extern const addr_type badaddr;
extern const addr_type extbit;
extern const addr_type default_addr;
extern const addr_type ns_addr;
extern const addr_type janus_addr;
extern const code_type badcode;

struct gaddr_type :
        public __pack_base
{
    gaddr_type() :
        hid(),
        pid( 0xffffffff ),
        addr( badaddr )
      { }

    explicit gaddr_type( const xmt::uuid_type& _hid, pid_t _pid, stem::addr_type _addr ) :
        hid( _hid ),
        pid( _pid ),
        addr( _addr )
      { }

    explicit gaddr_type( pid_t _pid, stem::addr_type _addr ) :
        hid( xmt::hostid() ),
        pid( _pid ),
        addr( _addr )
      { }

    explicit gaddr_type( stem::addr_type _addr ) :
        hid( xmt::hostid() ),
        pid( std::tr2::getpid() ),
        addr( _addr )
      { }

    gaddr_type( const gaddr_type& g ) :
        hid( g.hid ),
        pid( g.pid ),
        addr( g.addr )
      { }

    xmt::uuid_type  hid;
    // int64_t         pid; // pid_t defined as int, so it may be int64_t
    // most systems has max(pid) < 2^15, 64-bit Linuxes max(pid) < 4 *1024 *1024
    // but some other unixes may has larger numbers... check it
    uint32_t         pid;
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
        _src( badaddr ),
        _flags( 0 )
      { }

    explicit __Event_Base( code_type c ) :
        _code( c ),
        _dst( badaddr ),
        _src( badaddr ),
        _flags( 0 )
      { }

    /* explicit */ __Event_Base( const __Event_Base& e ) :
        _code( e._code ),
        _dst( e._dst ),
        _src( e._src ),
        _flags( e._flags )
      { }

    code_type code() const
      { return _code; }
    addr_type dest() const
      { return _dst; }
    addr_type src() const
      { return _src; }
    uint32_t flags() const
      { return _flags; }
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

    void setf( uint32_t f ) const
      { _flags |= f; }
    void unsetf( uint32_t f ) const
      { _flags &= (0xffffffff & ~f); }
    void resetf( uint32_t f ) const
      { _flags = f; }
    void cleanf() const
      { _flags = 0; }

    enum {
      conv = 1,
      expand = 2
    };

  protected:
    mutable code_type  _code; // event code
    mutable addr_type  _dst;  // destination
    mutable addr_type  _src;  // source
    mutable uint32_t   _flags;
};

// Forward declarations

template <class D, class POD > class __Event_base_aux;
template <class D> class __Event_base_aux<D,std::tr1::true_type>;
template <class D> class __Event_base_aux<D,std::tr1::false_type>;

template <class D> class Event_base;

template <> class Event_base<std::string>;
template <> class Event_base<void>;

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

template <class D, class POD >
class __Event_base_aux :
        public __Event_Base
{
};

template <class D>
class __Event_base_aux<D,std::tr1::true_type> :
        public __Event_Base
{
  public:
    typedef D         value_type;
    typedef D&        reference;
    typedef const D&  const_reference;
    typedef D *       pointer;
    typedef const D * const_pointer;

    __Event_base_aux() :
        __Event_Base(),
        _data()
      { }

    explicit __Event_base_aux( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    __Event_base_aux( code_type c, const D& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    __Event_base_aux( const __Event_Base& e, const D& d ) :
        __Event_Base( e ),
        _data( d )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    size_type value_size() const
      { return sizeof(_data); }

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

template <class D>
class __Event_base_aux<D,std::tr1::false_type> :
        public __Event_Base
{
  public:
    typedef D         value_type;
    typedef D&        reference;
    typedef const D&  const_reference;
    typedef D *       pointer;
    typedef const D * const_pointer;

    __Event_base_aux() :
        __Event_Base(),
        _data()
      { }

    explicit __Event_base_aux( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    __Event_base_aux( code_type c, const D& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    __Event_base_aux( const __Event_Base& e, const D& d ) :
        __Event_Base( e ),
        _data( d )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    size_type value_size() const
      { return sizeof(_data); }

    void pack( std::ostream& __s ) const
      { _data.pack( __s ); }
    void unpack( std::istream& __s )
      { _data.unpack( __s ); }
    void net_pack( std::ostream& __s ) const
      { _data.net_pack( __s ); }
    void net_unpack( std::istream& __s )
      { _data.net_unpack( __s ); }

  protected:
    value_type _data;
};

template <>
class __Event_base_aux<std::string,std::tr1::false_type> :
        public __Event_Base
{
  public:
    typedef std::string         value_type;
    typedef std::string&        reference;
    typedef const std::string&  const_reference;
    typedef std::string *       pointer;
    typedef const std::string * const_pointer;

    __Event_base_aux() :
        __Event_Base(),
        _data()
      { }

    explicit __Event_base_aux( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    __Event_base_aux( code_type c, const std::string& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    __Event_base_aux( const __Event_Base& e, const std::string& d ) :
        __Event_Base( e ),
        _data( d )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    size_type value_size() const
      { return sizeof(_data); }

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

template <>
class __Event_base_aux<xmt::uuid_type,std::tr1::false_type> :
        public __Event_Base
{
  public:
    typedef xmt::uuid_type         value_type;
    typedef xmt::uuid_type&        reference;
    typedef const xmt::uuid_type&  const_reference;
    typedef xmt::uuid_type*        pointer;
    typedef const xmt::uuid_type*  const_pointer;

    __Event_base_aux() :
        __Event_Base(),
        _data()
      { }

    explicit __Event_base_aux( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    __Event_base_aux( code_type c, const xmt::uuid_type& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    __Event_base_aux( const __Event_Base& e, const xmt::uuid_type& d ) :
        __Event_Base( e ),
        _data( d )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    size_type value_size() const
      { return sizeof(_data); }

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

template <>
class __Event_base_aux<void,std::tr1::true_type> :
        public __Event_Base
{
  public:
    typedef void         value_type;
    typedef void *       pointer;
    typedef size_t       size_type;
    typedef const void * const_pointer;

    __Event_base_aux() :
        __Event_Base()
      { }

    explicit __Event_base_aux( code_type c ) :
        __Event_Base( c )
      { }

    __Event_base_aux( const __Event_Base& e ) :
        __Event_Base( e )
      { }

    size_type value_size() const
      { return 0; }

    void pack( std::ostream& __s ) const
      { }
    void unpack( std::istream& __s )
      { }
    void net_pack( std::ostream& __s ) const
      { }
    void net_unpack( std::istream& __s )
      { }
};

template <class D>
class Event_base :
    public __Event_base_aux<D,typename std::tr1::is_pod<D>::type>
{
  private:
    typedef __Event_base_aux<D,typename std::tr1::is_pod<D>::type> _Base;

  public:
    Event_base() :
        __Event_base_aux<D,typename std::tr1::is_pod<D>::type>()
      { }

    explicit Event_base( code_type c ) :
        __Event_base_aux<D,typename std::tr1::is_pod<D>::type>( c )
      { }

    Event_base( code_type c, const D& d ) :
        __Event_base_aux<D,typename std::tr1::is_pod<D>::type>( c, d )
      { }

    Event_base( const Event_base& e ) :
        __Event_base_aux<D,typename std::tr1::is_pod<D>::type>( e, e._data )
      { }

    void net_pack( Event& s ) const;
    void net_unpack( const Event& s );
    void pack( Event& s ) const;
    void unpack( const Event& s );
};


template <>
class Event_base<std::string> :
    public __Event_base_aux<std::string,std::tr1::false_type>
{
  private:
    typedef __Event_base_aux<std::string,std::tr1::false_type> _Base;

  public:
    Event_base() :
        __Event_base_aux<std::string,std::tr1::false_type>()
      { }

    explicit Event_base( code_type c ) :
        __Event_base_aux<std::string,std::tr1::false_type>( c )
      { }

    Event_base( code_type c, const std::string& d ) :
        __Event_base_aux<std::string,std::tr1::false_type>( c, d )
      { }

    Event_base( const Event_base& e ) :
        __Event_base_aux<std::string,std::tr1::false_type>( e, e._data )
      { }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.resetf( _flags );
        s.value() = _data;
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _flags = s.flags();
        _data = s.value();
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.resetf( _flags );
        s.value() = _data;
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _flags = s.flags();
        _data = s.value();
      }
};

template <class D>
void Event_base<D>::net_pack( Event& s ) const
{
  s.code( _Base::_code );
  s.dest( _Base::_dst );
  s.src( _Base::_src );
  s.resetf( _Base::_flags | (__Event_Base::conv | __Event_Base::expand) );
  std::ostringstream ss;
  _Base::net_pack( ss );
  s.value() = ss.str();  
}

template <class D>
void Event_base<D>::net_unpack( const Event& s )
{
  _Base::_code = s.code();
  _Base::_dst  = s.dest();
  _Base::_src  = s.src();
  _Base::_flags = s.flags() & ~(__Event_Base::conv | __Event_Base::expand);
  std::istringstream ss( s.value() );
  _Base::net_unpack( ss );
}

template <class D>
void Event_base<D>::pack( Event& s ) const
{
  s.code( _Base::_code );
  s.dest( _Base::_dst );
  s.src( _Base::_src );
  s.resetf( _Base::_flags | __Event_Base::expand & ~__Event_Base::conv );
  // s.unsetf( __Event_Base::conv );
  std::ostringstream ss;
  _Base::pack( ss );
  s.value() = ss.str();
}

template <class D>
void Event_base<D>::unpack( const Event& s )
{
  _Base::_code = s.code();
  _Base::_dst  = s.dest();
  _Base::_src  = s.src();
  _Base::_flags = s.flags() & ~__Event_Base::expand;
  // _Base::unsetf( __Event_Base::expand );
  std::istringstream ss( s.value() );
  _Base::unpack( ss );
}

template <>
class Event_base<void> :
    public __Event_base_aux<void,std::tr1::true_type>
{
  private:
    typedef __Event_base_aux<void,std::tr1::true_type> _Base;

  public:

    Event_base() :
        __Event_base_aux<void,std::tr1::true_type>()
      { }

    explicit Event_base( code_type c ) :
        __Event_base_aux<void,std::tr1::true_type>( c )
      { }

    Event_base( const Event_base& e ) :
        __Event_base_aux<void,std::tr1::true_type>( e )
      { }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.resetf( _Base::_flags | __Event_Base::conv | __Event_Base::expand );
        s.value().erase();
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _flags = s.flags() & ~(__Event_Base::conv | __Event_Base::expand);
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.resetf( _Base::_flags & ~__Event_Base::conv | __Event_Base::expand );
        s.value().erase();
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _flags = s.flags() & ~__Event_Base::expand;
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

} // namespace stem

namespace std {

ostream& operator <<( ostream& o, const stem::gaddr_type& g );

} // namespace std

namespace EDS = stem;

#if defined(__USE_STLPORT_HASH) || defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
#  define __HASH_NAMESPACE std
#endif
#if defined(__USE_STD_HASH)
#  define __HASH_NAMESPACE __gnu_cxx
#endif

namespace __HASH_NAMESPACE {

#ifdef __USE_STD_TR1
namespace tr1 {
#endif

template <>
struct hash<stem::gaddr_type>
{
    size_t operator()(const stem::gaddr_type& __x) const
      { return __x.addr | (__x.pid << 23); }
};

#ifdef __USE_STD_TR1
}
#endif

#if defined(__GNUC__) && (__GNUC__ < 4) && !defined(HASH_VOID_PTR_DEFINED)
template<>
struct hash<void *>
{
   size_t operator()(const void *__x) const
     { return reinterpret_cast<size_t>(__x); }
};

#define HASH_VOID_PTR_DEFINED

#endif // __GNUC__ < 4

} // namespace __HASH_NAMESPACE

#undef __HASH_NAMESPACE

#ifdef __USE_STLPORT_HASH
#  undef __USE_STLPORT_HASH
#endif
#ifdef __USE_STD_HASH
#  undef __USE_STD_HASH
#endif
#ifdef __USE_STLPORT_TR1
#  undef __USE_STLPORT_TR1
#endif
#ifdef __USE_STD_TR1
#  undef __USE_STD_TR1
#endif

#endif // __stem_Event_h
