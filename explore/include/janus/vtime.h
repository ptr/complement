// -*- C++ -*- Time-stamp: <09/09/11 17:14:14 ptr>

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
extern const addr_type& nil_addr;

} // namespace janus

namespace janus {

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
      { vt = _vt.vt; }

    // bool operator ==( const vtime& r ) const
    //   { return vt == r.vt; }

    bool operator <=( const vtime& r ) const;

    bool operator >=( const vtime& r ) const
      { return r <= *this; }

    vtime operator +( const vtime& r ) const;
    vtime operator -( const vtime& r ) const;

    vtime& operator +=( const vtime& t );
    vtime& operator +=( const vtime_type::value_type& );

    vtime& sup( const vtime& r );

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

vtime chg( const vtime&, const vtime& );

#ifdef __USE_STLPORT_HASH
typedef std::hash_map<addr_type,vtime> vtime_matrix_type;
#endif
#ifdef __USE_STD_HASH
typedef __gnu_cxx::hash_map<addr_type,vtime> vtime_matrix_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
typedef std::tr1::unordered_map<addr_type,vtime> vtime_matrix_type;
#endif

struct basic_event :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const
      { vt.pack( s ); }

    void unpack( std::istream& s )
      { vt.unpack( s ); }

    basic_event()
      { }
    basic_event( const basic_event& vt_ ) :
        vt( vt_.vt )
      { }

    void swap( basic_event& r )
      { std::swap( vt, r.vt ); }

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

struct VT_sync :
    public basic_event
{
    VT_sync()
      { }
    VT_sync( const VT_sync& vt_ ) :
        basic_event( vt_ )
      { }
};

// vtime max( const vtime& l, const vtime& r );

// typedef std::pair<group_type, vtime> vtime_group_type;
// typedef std::list<vtime_group_type> gvtime_type;

struct gvtime :
    public stem::__pack_base
{
#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<gid_type, vtime> gvtime_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<gid_type, vtime> gvtime_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<gid_type, vtime> gvtime_type;
#endif
    void pack( std::ostream& s ) const;
    void unpack( std::istream& s );

    gvtime()
      { }
    gvtime( const gvtime& _gvt ) :
        gvt( _gvt.gvt.begin(), _gvt.gvt.end() )
      { }
    template <class C>
    gvtime( const C& first, const C& last ) :
        gvt( first, last )
      { }

    gvtime& operator =( const gvtime& _gvt )
      { gvt = _gvt.gvt; }
    gvtime& operator =( const gvtime_type& _gvt )
      { gvt = _gvt; }

    gvtime operator -( const gvtime& ) const;

    gvtime& operator +=( const gvtime_type::value_type& );
    gvtime& operator +=( const gvtime& );

    gvtime_type::mapped_type& operator[]( const gvtime_type::key_type k )
      { return gvt[k]; }
    const gvtime_type::mapped_type& operator[]( const gvtime_type::key_type k ) const
      { return gvt[k]; }

    mutable gvtime_type gvt;
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

class basic_vs :
    public stem::EventHandler
{
  private:
    typedef std::set<stem::addr_type> group_members_type;

    static const stem::code_type VS_GROUP_R1;
    static const stem::code_type VS_GROUP_R2;
    static const stem::code_type VS_EVENT;

  public:
    basic_vs();
    basic_vs( stem::addr_type id );
    basic_vs( stem::addr_type id, const char *info );
    ~basic_vs();

    template <class InputIter>
    void VTjoin( InputIter first, InputIter last )
      {
        // super is ordered container
        group_members_type super( first, last );
        
        super.insert( self_id() );

        group_members_type::const_iterator i = super.find( self_id() );

        if ( ++i == super.end() ) {
          i = super.begin();
        }

        if ( i != super.end() && *i != self_id() ) {
          stem::Event_base<VT_sync> ev( VS_GROUP_R1 );

          ev.dest( *i );
          ev.value().vt = vt[self_id()];

          Send( ev );
        }
      }

    void vs( const stem::Event& );

    template <class D>
    void vs( const stem::Event_base<D>& e )
      { basic_vs::vs( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }

  protected:
    vtime_matrix_type vt;

    virtual void round2_pass() = 0;

  private:
    void new_member_round1( const stem::Event_base<VT_sync>& );
    void new_member_round2( const stem::Event_base<VT_sync>& );
    void vt_process( const stem::Event_base<vs_event>& );

    typedef std::list<stem::Event_base<vs_event> > delay_container_type;

    delay_container_type dc;

    DECLARE_RESPONSE_TABLE( basic_vs, stem::EventHandler );
};

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
