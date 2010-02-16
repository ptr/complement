// -*- C++ -*- Time-stamp: <10/02/16 13:12:43 ptr>

/*
 *
 * Copyright (c) 2008-2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __casual_h
#define __casual_h

#include <janus/vtime.h>

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

namespace stem {
class Cron;
}

namespace janus {

extern const addr_type& nil_addr;
extern const gid_type& nil_gid;

class basic_vs :
    public stem::EventHandler
{
  public:
    typedef vtime::vtime_type::size_type size_type;

  private:
    typedef std::set<stem::addr_type> group_members_type;

    static const stem::code_type VS_EVENT;
    static const stem::code_type VS_JOIN_RQ;
    static const stem::code_type VS_JOIN_RS;
    static const stem::code_type VS_LOCK_VIEW;
    static const stem::code_type VS_LOCK_VIEW_ACK;
    static const stem::code_type VS_LOCK_VIEW_NAK;
    static const stem::code_type VS_UPDATE_VIEW;
    static const stem::code_type VS_FLUSH_LOCK_VIEW;
    static const stem::code_type VS_FLUSH_LOCK_VIEW_ACK;
    static const stem::code_type VS_FLUSH_LOCK_VIEW_NAK;
    static const stem::code_type VS_LOCK_SAFETY; // from cron, timeout
    static const stem::code_type VS_LAST_WILL;

  protected:
    static const stem::code_type VS_FLUSH_VIEW;
    static const stem::code_type VS_FLUSH_VIEW_JOIN;

  protected:
    static const stem::state_type VS_ST_LOCKED;

  private:
    class Init
    {
      public:
        Init();
        ~Init();
      private:
        static void _guard( int );
        static void __at_fork_prepare();
        static void __at_fork_child();
        static void __at_fork_parent();
    };

  public:
    basic_vs();
    basic_vs( const char* info );
    ~basic_vs();

    void vs_tcp_point( uint32_t, int );
    void vs_tcp_point( const sockaddr_in& );
    void vs_copy_tcp_points( const basic_vs& );
    int vs_join( const stem::addr_type& );
    int vs_join( const stem::addr_type&, const char*, int );
    int vs_join( const stem::addr_type&, const sockaddr_in& );
    int vs_join( const char*, int );
    int vs_join( const sockaddr_in& );
    int vs( const stem::Event& );

    template <class D>
    int vs( const stem::Event_base<D>& e )
      { return basic_vs::vs( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }

  protected:
    int vs_aux( const stem::Event& );

    template <class D>
    int vs_aux( const stem::Event_base<D>& e )
      { return basic_vs::vs_aux( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }

  public:
    void vs_send_flush();
    size_type vs_group_size() const;

    void send_to_vsg( const stem::Event& ); // not VS!

    template <class D>
    void send_to_vsg( const stem::Event_base<D>& e ) // not VS!
      { basic_vs::send_to_vsg( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }

    void forward_to_vsg( const stem::Event& ); // not VS!

    template <class D>
    void forward_to_vsg( const stem::Event_base<D>& e ) // not VS!
      { basic_vs::forward_to_vsg( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }
		
  protected:
    void check_remotes();

    vtime vt;

    virtual xmt::uuid_type vs_pub_recover() = 0;
    virtual void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& ) = 0;
    virtual void vs_pub_view_update() = 0;
    virtual void vs_pub_rec( const stem::Event& ) = 0;
    virtual void vs_pub_flush() = 0;
    virtual void vs_pub_join();

    void replay( const stem::Event& );

  private:
    void vs_lock_view( const stem::EventVoid& );
    void vs_lock_view_lk( const stem::EventVoid& );
    void vs_lock_view_ack( const stem::EventVoid& );
    void vs_lock_view_nak( const stem::EventVoid& );

    void vs_process( const stem::Event_base<vs_event>& );
    void vs_process_lk( const stem::Event_base<vs_event>& );
    void vs_join_request( const stem::Event_base<vs_join_rq>& );
    void vs_join_request_lk( const stem::Event_base<vs_join_rq>& );
    void vs_group_points( const stem::Event_base<vs_points>& );

    void vs_flush_lock_view( const stem::EventVoid& );
    void vs_flush_lock_view_lk( const stem::EventVoid& );
    void vs_flush_lock_view_ack( const stem::EventVoid& );
    void vs_flush_lock_view_nak( const stem::EventVoid& );
    void vs_flush( const xmt::uuid_type& );
    void vs_flush_wr( const xmt::uuid_type& );

    void vs_lock_safety( const stem::EventVoid& ev );

    void process_delayed();

    void add_lock_safety();
    void rm_lock_safety();

    void process_last_will( const stem::Event_base<janus::addr_type>& );
    void process_last_will_lk( const stem::Event_base<janus::addr_type>& );

    // vs order violation events
    typedef std::list<stem::Event_base<vs_event> > ove_container_type;
    typedef std::list<stem::Event> delayed_container_type;

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

    ove_container_type ove;
    delayed_container_type de;
    vs_points::points_type points;
    unsigned view;
    stem::addr_type lock_addr;
    lock_rsp_type lock_rsp;
    stem::addr_type group_applicant;
    stem::addr_type group_applicant_ref;

  private:
    typedef std::list<stem::NetTransportMgr*> access_container_type;

    access_container_type remotes_;

    static class stem::Cron* _cron;

    DECLARE_RESPONSE_TABLE( basic_vs, stem::EventHandler );

    friend class Init;
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

#endif // __casual_h
