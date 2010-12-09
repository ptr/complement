// -*- C++ -*- Time-stamp: <10/05/25 11:37:07 ptr>

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

#include <config/feature.h>

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

typedef xmt::uuid_type addr_type;
typedef uint32_t code_type;

extern const addr_type& badaddr;
extern const code_type badcode;

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

    __Event_Base( const __Event_Base& e ) :
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
      conv    = 0x1,
      expand  = 0x2,
      vs      = 0x4,
      vs_join = 0x8
    };

    void swap( __Event_Base& l )
      {
        std::swap( _code, l._code );
        std::swap( _dst, l._dst );
        std::swap( _src, l._src );
        std::swap( _flags, l._flags );
      }

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
      virtual void unpack( std::istream& s );
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

    void swap( __Event_base_aux<D,std::tr1::true_type>& l )
      {
        __Event_Base::swap( static_cast<__Event_Base&>(l) );
        std::swap( _data, l._data );
      }

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

    void swap( __Event_base_aux<D,std::tr1::false_type>& l )
      {
        __Event_Base::swap( static_cast<__Event_Base&>(l) );
        std::swap( _data, l._data );
      }

  protected:
    value_type _data;
};

template <class F, class S>
class __Event_base_aux<std::pair<F,S>,std::tr1::false_type> :
        public __Event_Base
{
  public:
    typedef std::pair<F,S>         value_type;
    typedef std::pair<F,S>&        reference;
    typedef const std::pair<F,S>&  const_reference;
    typedef std::pair<F,S>*        pointer;
    typedef const std::pair<F,S>*  const_pointer;

    __Event_base_aux() :
        __Event_Base(),
        _data()
      { }

    explicit __Event_base_aux( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    __Event_base_aux( code_type c, const_reference d ) :
        __Event_Base( c ),
        _data( d )
      { }

    __Event_base_aux( const __Event_Base& e, const_reference d ) :
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
      {
        __pack_base::__pack( __s, _data.first );
        __pack_base::__pack( __s, _data.second );
      }
    void unpack( std::istream& __s )
      {
        __pack_base::__unpack( __s, _data.first );
        __pack_base::__unpack( __s, _data.second );
      }

    void swap( __Event_base_aux<value_type,std::tr1::false_type>& l )
      {
        __Event_Base::swap( static_cast<__Event_Base&>(l) );
        std::swap( _data.first, l._data.first );
        std::swap( _data.second, l._data.second );
      }

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

    void swap( __Event_base_aux<std::string,std::tr1::false_type>& l )
      {
        __Event_Base::swap( static_cast<__Event_Base&>(l) );
        std::swap( _data, l._data );
      }

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

    void swap( __Event_base_aux<xmt::uuid_type,std::tr1::false_type>& l )
      {
        __Event_Base::swap( static_cast<__Event_Base&>(l) );
        std::swap( _data, l._data );
      }

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
void Event_base<D>::pack( Event& s ) const
{
  s.code( _Base::_code );
  s.dest( _Base::_dst );
  s.src( _Base::_src );
  s.resetf( _Base::_flags | (__Event_Base::conv | __Event_Base::expand) );
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
  _Base::_flags = s.flags() & ~(__Event_Base::conv | __Event_Base::expand);
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

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.resetf( _Base::_flags | __Event_Base::conv | __Event_Base::expand );
        s.value().erase();
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _flags = s.flags() & ~(__Event_Base::conv | __Event_Base::expand);
      }

    void pack( std::ostream& ) const
      { }
    void unpack( std::istream& )
      { }
};

} // namespace stem

namespace std {

template <class T>
inline void swap( stem::Event_base<T>& l, stem::Event_base<T>& r )
{ l.swap(r); }

} // namespace std

namespace EDS = stem;

#endif // __stem_Event_h
