// -*- C++ -*- Time-stamp: <10/06/10 20:34:00 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __leader_recovery_h
#define __leader_recovery_h

#include <janus/torder.h>
#include <fstream>
#include <string>

namespace janus {

class VT_with_leader_recovery :
        public torder_vs
{
  public:
    VT_with_leader_recovery( const char* );
    ~VT_with_leader_recovery();

    template <class Duration>
    bool wait_group_size( const Duration& rel_time, int _gsize )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );
        
        gsize = _gsize;
        
        return cnd.timed_wait( lk, rel_time, gs_status );
      }

    template <class Duration>
    bool wait_msg( const Duration& rel_time, int _n_msg )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );
        
        n_msg = _n_msg;

        return cnd.timed_wait( lk, rel_time, msg_status );
      }

    template <class Duration>
    bool wait_flush( const Duration& rel_time, int _n_flush )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );

        n_flush = _n_flush;

        return cnd.timed_wait( lk, rel_time, flush_status );
      }

    virtual xmt::uuid_type vs_pub_recover( bool is_founder );
    virtual void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    virtual void vs_pub_view_update();
    virtual void vs_pub_rec( const stem::Event& );
    virtual void vs_pub_flush();
    virtual void vs_pub_tord_rec( const stem::Event& );
    virtual std::tr2::milliseconds vs_pub_lock_timeout() const;

    void reset_msg()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); msg = 0; }

    void reset_flush()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); flush = 0; }


    int msg;
  private:
    void message( const stem::Event& );
    void sync_message( const stem::Event& );

    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;
    int n_msg;
    int gsize;
    int flush;
    int n_flush;

    std::fstream history;

    struct _gs_status
    {
        _gs_status( VT_with_leader_recovery& m ) :
            me( m )
          { }

        bool operator()() const;

        VT_with_leader_recovery& me;
    } gs_status;    

    struct _msg_status
    {
        _msg_status( VT_with_leader_recovery& m ) :
            me( m )
          { }

        bool operator()() const;

        VT_with_leader_recovery& me;
    } msg_status;

    struct _flush_status
    {
        _flush_status( VT_with_leader_recovery& m ) :
            me( m )
          { }

        bool operator()() const;

        VT_with_leader_recovery& me;
    } flush_status;

    DECLARE_RESPONSE_TABLE( VT_with_leader_recovery, janus::torder_vs );
};

} // namespace janus

#endif // __leader_recovery_h
