// -*- C++ -*- Time-stamp: <10/02/16 16:27:30 ptr>

/*
 *
 * Copyright (c) 2008-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __vtime_h
#define __vtime_h

#include <algorithm>
#include <list>
#include <vector>
#include <iterator>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <set>

#include <stem/Event.h>
#include <stem/EventHandler.h>
#include <stem/NetTransport.h>

#include <mt/time.h>

#include <mt/uid.h>
#include <mt/uidhash.h>

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

namespace janus {

typedef stem::addr_type addr_type;
typedef uint32_t vtime_unit_type;
typedef xmt::uuid_type gid_type; // required, used in VTSend

extern const gid_type& nil_gid;

struct vtime :
    public stem::__pack_base
{
#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<addr_type, vtime_unit_type> vtime_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<addr_type, vtime_unit_type> vtime_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<addr_type, vtime_unit_type> vtime_type;
#endif

    void pack( std::ostream& s ) const;
    void unpack( std::istream& s );

    vtime()
      { }
    vtime( const vtime& _vt ) :
        vt( _vt.vt.begin(), _vt.vt.end() )
      { }
    vtime( const vtime_type& _vt ) :
        vt( _vt.begin(), _vt.end() )
      { }

    template <class C>
    vtime( const C& first, const C& last ) :
        vt( first, last )
      { }

    vtime& operator =( const vtime& _vt )
      { vt = _vt.vt; return *this; }

    vtime_type::mapped_type& operator[]( const vtime_type::key_type& k )
      { return vt[k]; }
    const vtime_type::mapped_type& operator[]( const vtime_type::key_type& k ) const
      { return vt[k]; }

    void swap( vtime& _vt )
      { std::swap( vt, _vt.vt ); }
    void clear()
      { vt.clear(); }
    
    mutable vtime_type vt;
};

struct basic_event :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const
      { __pack( s, view ); vt.pack( s ); }

    void unpack( std::istream& s )
      { __unpack( s, view ); vt.unpack( s ); }

    basic_event()
      { }
    basic_event( const basic_event& vt_ ) :
        view( vt_.view ),
        vt( vt_.vt )
      { }

    void swap( basic_event& r )
      { std::swap( view, r.view ); std::swap( vt, r.vt ); }

    uint32_t view;
    vtime vt;
};

struct vs_event :
    public basic_event
{
    vs_event()
      { }
    vs_event( const vs_event& e ) :
        basic_event( e ),
        ev( e.ev )
      { }

    virtual void pack( std::ostream& s ) const;
    virtual void unpack( std::istream& s );

    void swap( vs_event& );

    stem::Event ev;
};

struct vs_points :
    public basic_event
{
    vs_points()
      { }
    vs_points( const vs_points& vt_ ) :
        basic_event( vt_ ),
        points( vt_.points )
      { }

    virtual void pack( std::ostream& s ) const;
    virtual void unpack( std::istream& s );

    void swap( vs_points& );

    struct access_t {
        xmt::uuid_type hostid;
        uint8_t family;
        uint8_t type;
        std::string data;
    };

#ifdef __USE_STLPORT_HASH
    typedef std::hash_multimap<addr_type,access_t> points_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_multimap<addr_type,access_t> points_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_multimap<addr_type,access_t> points_type;
#endif
    points_type points;
};

namespace detail {

struct access_points :
        public stem::__pack_base
{
    access_points()
      { }

    access_points( const access_points& r ) :
        points( r.points )
      { }

    virtual void pack( std::ostream& s ) const;
    virtual void unpack( std::istream& s );

    void swap( access_points& );

    janus::vs_points::points_type points;
};

} // namespace detail

struct vs_join_rq :
    public vs_points
{
    vs_join_rq()
      { }
    vs_join_rq( const vs_join_rq& vt_ ) :
        vs_points( vt_ )
      { }

    virtual void pack( std::ostream& s ) const;
    virtual void unpack( std::istream& s );

    void swap( vs_join_rq& );

    xmt::uuid_type reference;
};

struct vs_event_total_order :
        public stem::__pack_base
{
    typedef xmt::uuid_type id_type;

    vs_event_total_order()
      { }
    vs_event_total_order( const vs_event_total_order& e ) :
        id( e.id ),
        conform( e.conform ),
        ev( e.ev )
      { }

    virtual void pack( std::ostream& s ) const;
    virtual void unpack( std::istream& s );

    void swap( vs_event_total_order& );

    id_type id;
    std::list<id_type> conform;
    stem::Event ev;
};

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

} // namespace janus

namespace std {

template <>
inline void swap( janus::basic_event& l, janus::basic_event& r )
{ l.swap( r ); }

template <>
inline void swap( janus::vs_event& l, janus::vs_event& r )
{ l.swap( r ); }

} // namespace std

#endif // __vtime_h
