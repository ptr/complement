// -*- C++ -*- Time-stamp: <2012-02-08 14:50:40 ptr>

/*
 *
 * Copyright (c) 2010-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __janus_torder_h
#define __janus_torder_h

#include <janus/vtime.h>
#include <janus/causal.h>

#include <list>

#if defined(STLPORT) || defined(__FIT_CPP_0X)
#  include <unordered_map>
#  define __USE_STLPORT_TR1
#else
#  if defined(__GNUC__) && (__GNUC__ < 4)
#    include <ext/hash_map>
#    define __USE_STD_HASH
#  else
#    include <tr1/unordered_map>
#    define __USE_STD_TR1
#  endif
#endif

namespace janus {

class torder_vs :
    public basic_vs
{
  public:
    torder_vs();
    torder_vs( const char* info );

    int vs_torder( const stem::Event& );

    template <class D>
    int vs_torder( const stem::Event_base<D>& e )
      { return torder_vs::vs_torder( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }

    bool is_leader() const
      { return is_leader_; }

  protected:
    virtual void vs_pub_view_update();
    virtual void vs_pub_flush();
    virtual void vs_resend_from( const xmt::uuid_type&, const stem::ext_addr_type& );
    virtual void vs_pub_tord_rec( const stem::Event& ) = 0;

  protected:
    void vs_process_torder( const stem::Event_base<vs_event_total_order>& );
    void vs_torder_conf( const stem::Event_base<vs_event_total_order::id_type>& );

    void vs_leader( const stem::EventVoid& );
    void check_leader();

#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<vs_event_total_order::id_type,stem::Event> conf_cnt_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<vs_event_total_order::id_type,stem::Event> conf_cnt_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::unordered_map<vs_event_total_order::id_type,stem::Event> conf_cnt_type;
#endif
    typedef std::list<vs_event_total_order::id_type> orig_order_cnt_type;

    conf_cnt_type conform_container_;
    orig_order_cnt_type orig_order_container_;
    bool is_leader_;

    DECLARE_RESPONSE_TABLE( torder_vs, basic_vs );
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

#endif // __janus_torder_h
