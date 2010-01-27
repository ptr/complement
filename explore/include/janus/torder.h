// -*- C++ -*- Time-stamp: <10/01/27 20:25:26 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __janus_torder_h
#define __janus_torder_h

#include <janus/vtime.h>
#include <list>

#ifdef STLPORT
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

class torder_vs :
    public basic_vs
{
  private:
    static const stem::code_type VS_EVENT_TORDER;
    static const stem::code_type VS_ORDER_CONF;

  public:
    torder_vs();
    torder_vs( const char* info );
    ~torder_vs();

    int vs_torder( const stem::Event& );

    template <class D>
    int vs_torder( const stem::Event_base<D>& e )
      { return torder_vs::vs_torder( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }
    // void vs_send_flush();

  protected:
    int vs_torder_aux( const stem::Event& );

    template <class D>
    int vs_torder_aux( const stem::Event_base<D>& e )
      { return torder_vs::vs_torder_aux( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }

  public:
    void leader();

    bool is_leader() const
      { return is_leader_; }

  protected:
    virtual void vs_pub_join();

  private:
#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<vs_event_total_order::id_type,stem::Event> conf_cnt_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<vs_event_total_order::id_type,stem::Event> conf_cnt_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<vs_event_total_order::id_type,stem::Event> conf_cnt_type;
#endif

    conf_cnt_type conform_container_;
    stem::addr_type leader_;
    bool is_leader_;

    void vs_process_torder( const stem::Event_base<vs_event_total_order>& );

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
