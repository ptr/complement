// -*- C++ -*- Time-stamp: <99/03/24 18:20:20 ptr>
#ifndef __EDS_Event_h
#define __EDS_Event_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <string>
#include <istream>
#include <ostream>
#include <type_traits.h>
#include <sstream>

#include <EvPack.h>

namespace EDS {

class __Event_Base
{
  public:
    typedef unsigned code_type;
    typedef unsigned key_type;
    typedef unsigned sq_type;
    typedef size_t   size_type;
    
    enum {
      extbit  = 0x80000000
    };

    __Event_Base() :
        _code( 0 ),
        _dst( 0 ),
        _src( 0 ),
        _sid( 0 ),
        _sqn( 0 ),
        _rsqn( 0 )
      { }

    explicit __Event_Base( code_type c ) :
        _code( c ),
        _dst( 0 ),
        _src( 0 ),
        _sid( 0 ),
        _sqn( 0 ),
        _rsqn( 0 )        
      { }

    explicit __Event_Base( const __Event_Base& e ) :
        _code( e._code ),
        _dst( e._dst ),
        _src( e._src ),
        _sid( e._sid ),
        _sqn( e._sqn ),
        _rsqn( e._rsqn ) // ! not _rsqn
      { }

    code_type code() const
      { return _code; }
    key_type dest() const
      { return _dst; }
    key_type src() const
      { return _src; }
    key_type sid() const
      { return _sid; }
    sq_type seq() const
      { return _sqn; }
    sq_type responce() const
      { return _rsqn; }
    bool is_responce() const
      { return _rsqn != 0; }
    bool is_from_foreign() const
      { return (_src & extbit) != 0; }
    bool is_to_foreign() const
      { return (_dst & extbit) != 0; }

    void code( code_type c )
      { _code = c; }
    void dest( key_type c )
      { _dst = c; }
    void src( key_type c )
      { _src = c; }
    void seq( sq_type c )
      { _sqn = c; }
    void responce( sq_type c )
      { _rsqn = c; }
    void sid( key_type c )
      { _sid = c; }

  protected:
    code_type _code; // event code
    key_type  _dst;  // destination
    key_type  _src;  // source
    key_type  _sid;  // session ID
    sq_type   _sqn;  // sequential number / responce to number
    sq_type   _rsqn; // responce to number

    friend class NetTransport;
};

// Forward declarations

template <class D> class Event_base;
// VC 5.0 to be very huffy on typedefed std::string...
#ifndef _MSC_VER
template <> class Event_base<std::string>;
#else
template <> 
class Event_base<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >;
#endif
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

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const D& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { pack( __s, __type_traits<D>::is_POD_type() ); }
    void unpack( std::istream& __s )
      { unpack( __s, __type_traits<D>::is_POD_type() ); }
    void net_pack( std::ostream& __s ) const
      { net_pack( __s, __type_traits<D>::is_POD_type() ); }
    void net_unpack( std::istream& __s )
      { net_unpack( __s, __type_traits<D>::is_POD_type() ); }

  protected:
    value_type _data;

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
#ifndef _MSC_VER
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
#endif
      }
    void net_pack( std::ostream& __s, __false_type ) const
      { _data.net_pack( __s ); }
    void net_unpack( std::istream& __s, __true_type )
      {
#ifndef _MSC_VER
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
#endif
      }
    void net_unpack( std::istream& __s, __false_type )
      { _data.net_unpack( __s ); }
};

// VC 5.0 to be very huffy on typedefed std::string...
__STL_TEMPLATE_NULL
#ifndef _MSC_VER
class Event_base<std::string> :
#else
class Event_base<std::basic_string<char, std::char_traits<char>, std::allocator<char> > > :
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

    explicit Event_base( code_type c, const std::string& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const std::string& d )
      { _data = d; }
    size_type value_size() const
      { return _data.size(); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        s.value( _data );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        _data = s.value();
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        s.value( _data );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
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
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        s.value( std::string() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        s.value( std::string() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
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

#if 0
class GENERIC;
class EventsCore;

template <class T> class EDSCallbackObject;
typedef EDSEventT<EDSEventsCore,EDSCallbackObject<GENERIC> > EDSEventCb;

template <class T>
class EDSCallbackObject
{
  public:
    typedef void (T::*PMF)();
    EDSCallbackObject( T *o, PMF pmf )
      { object = o; Pmf = pmf; }
    EDSCallbackObject()
      { object = 0; Pmf = 0; }
    operator EDSCallbackObject<GENERIC>&()
      { return *((EDSCallbackObject<GENERIC> *)this); }
    operator const EDSCallbackObject<GENERIC>&() const
      { return *((EDSCallbackObject<GENERIC> *)this); }
    T *object;
    PMF Pmf;
};
#endif

} // namespace EDS

#endif
