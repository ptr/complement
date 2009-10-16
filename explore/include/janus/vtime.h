// -*- C++ -*- Time-stamp: <09/10/16 15:28:03 ptr>

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

class basic_vs :
    public stem::EventHandler
{
  private:
    typedef std::set<stem::addr_type> group_members_type;

    static const stem::code_type VS_EVENT;
    static const stem::code_type VS_LEAVE;
    static const stem::code_type VS_JOIN_RQ;
    static const stem::code_type VS_JOIN_RS;
    static const stem::code_type VS_LOCK_VIEW;
    static const stem::code_type VS_LOCK_VIEW_ACK;
    static const stem::code_type VS_LOCK_VIEW_NAK;
    static const stem::code_type VS_UPDATE_VIEW;
    static const stem::code_type VS_FLUSH_LOCK_VIEW;
    static const stem::code_type VS_FLUSH_LOCK_VIEW_ACK;
    static const stem::code_type VS_FLUSH_LOCK_VIEW_NAK;

  protected:
    static const stem::code_type VS_FLUSH_VIEW;
    static const stem::code_type VS_FLUSH_VIEW_JOIN;

  protected:
    static const stem::state_type VS_ST_LOCKED;

  public:
    basic_vs();
    basic_vs( stem::addr_type id );
    basic_vs( stem::addr_type id, const char* info );
    basic_vs( const char* info );
    ~basic_vs();

    void vs_tcp_point( uint32_t, int );
    void vs_tcp_point( const sockaddr_in& );
    void vs_join( const stem::addr_type& );
    void vs_join( const stem::addr_type&, const char*, int );
    void vs_join( const char*, int );
    void vs( const stem::Event& );

    template <class D>
    void vs( const stem::Event_base<D>& e )
      { basic_vs::vs( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }
    void vs_send_flush();

  protected:
    vtime_matrix_type vt;

    virtual xmt::uuid_type vs_pub_recover() = 0;
    virtual void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& ) = 0;
    virtual void vs_pub_view_update() = 0;
    virtual void vs_event_origin( const vtime&, const stem::Event& ) = 0;
    virtual void vs_event_derivative( const vtime&, const stem::Event& ) = 0;
    virtual void vs_pub_flush() = 0;

    void replay( const vtime&, const stem::Event& );

  private:
    void vs_lock_view( const stem::EventVoid& );
    void vs_lock_view_lk( const stem::EventVoid& );
    void vs_lock_view_ack( const stem::EventVoid& );
    void vs_lock_view_nak( const stem::EventVoid& );
    void vs_view_update();

    void vs_process( const stem::Event_base<vs_event>& );
    void vs_process_lk( const stem::Event_base<vs_event>& );
    void vs_leave( const stem::Event_base<basic_event>& );
    void vs_join_request( const stem::Event_base<vs_join_rq>& );
    void vs_join_request_lk( const stem::Event_base<vs_join_rq>& );
    void vs_group_points( const stem::Event_base<vs_points>& );

    void vs_flush_lock_view( const stem::EventVoid& );
    void vs_flush_lock_view_lk( const stem::EventVoid& );
    void vs_flush_lock_view_ack( const stem::EventVoid& );
    void vs_flush_lock_view_nak( const stem::EventVoid& );
    void vs_flush( const xmt::uuid_type& );
    void vs_flush_wr( const xmt::uuid_type& );

    void process_delayed();

    typedef std::list<stem::Event_base<vs_event> > delay_container_type;

  protected:
#ifdef __USE_STLPORT_HASH
    typedef std::hash_set<addr_type> lock_rsp_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_set<addr_type> lock_rsp_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_set<addr_type> lock_rsp_type;
#endif

    delay_container_type dc;
    vs_points::points_type points;
    unsigned view;
    stem::addr_type lock_addr;
    lock_rsp_type lock_rsp;
    stem::addr_type group_applicant;

  private:
    typedef std::list<stem::NetTransportMgr*> access_container_type;

    access_container_type remotes_;

    DECLARE_RESPONSE_TABLE( basic_vs, stem::EventHandler );
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
