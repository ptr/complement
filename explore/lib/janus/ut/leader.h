// -*- C++ -*- Time-stamp: <10/06/10 20:34:00 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __leader_h
#define __leader_h

#include <janus/torder.h>
#include <fstream>
#include <string>

namespace janus {

class VT_with_leader :
        public torder_vs
{
  public:
    VT_with_leader( const char* );
    ~VT_with_leader();

    xmt::uuid_type vs_pub_recover();
    void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    void vs_pub_view_update();
    void vs_pub_rec( const stem::Event& );
    void vs_pub_flush();
    virtual void vs_pub_tord_rec( const stem::Event& );

    bool flushed;
  private:
    void message( const stem::Event& );
    void sync_message( const stem::Event& );

    std::ofstream f;
    std::string name;

    DECLARE_RESPONSE_TABLE( VT_with_leader, janus::torder_vs );
};

class TO_object :
        public torder_vs
{
  public:
    TO_object();
    ~TO_object();

  private:
    xmt::uuid_type vs_pub_recover();
    void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    void vs_pub_view_update();
    void vs_pub_rec( const stem::Event& );
    void vs_pub_flush();
    virtual void vs_pub_tord_rec( const stem::Event& );

  private:
    void message( const stem::Event& );
    void message2( const stem::Event& );

    std::tr2::condition_variable cnd;
    std::tr2::mutex mtx_joined;
    bool joined;
    bool joined2;

    struct _status
    {
        _status( TO_object& m ) :
            me( m )
          { }

        bool operator()() const;

        TO_object& me;
    } status;

    struct _status2
    {
        _status2( TO_object& m ) :
            me( m )
          { }

        bool operator()() const;

        TO_object& me;
    } status2;

  public:

    template <class Duration>
    bool wait( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx_joined );

        return cnd.timed_wait( lk, rel_time, status );
      }

    template <class Duration>
    bool wait2( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx_joined );

        return cnd.timed_wait( lk, rel_time, status2 );
      }

  private:
    DECLARE_RESPONSE_TABLE( TO_object, janus::torder_vs );
};

} // namespace janus

#endif // __leader_h
