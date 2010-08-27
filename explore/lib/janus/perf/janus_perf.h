// -*- C++ -*- Time-stamp: <09/07/31 13:48:26 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __janus_perf_h
#define __janus_perf_h

#include <exam/suite.h>
#include <string>
#include <fstream>
#include <sstream>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

#include <janus/torder.h>

class janus_perf
{
  public:
    template <int N_user, int N_msg>
    int EXAM_DECL(group_send_mt);
  private:
    static int n_obj;
    static int n_msg;
    static std::vector< std::tr2::thread* > thr;
    static std::vector< std::string > names;
    static std::vector< int > res;
    static stem::addr_type addr;

    static void run(int i);
};

#define EV_EXT_EV_SAMPLE      0x9010
#define EV_VS_EV_SAMPLE       0x9011
#define EV_VS_EV_SAMPLE2      0x9012

#ifndef VS_FLUSH_RQ
# define VS_FLUSH_RQ 0x307 // see casual.cc
#endif

class VT_with_leader_recovery :
        public janus::torder_vs
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

    xmt::uuid_type vs_pub_recover();
    void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    void vs_pub_view_update();
    void vs_pub_rec( const stem::Event& );
    void vs_pub_flush();
    virtual void vs_pub_tord_rec( const stem::Event& );
    virtual std::tr2::milliseconds vs_pub_lock_timeout() const;

    void reset_msg()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); msg = 0; }

    void reset_flush()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); flush = 0; }


    int msg;
    int flush;
  private:
    void message( const stem::Event& );
    void sync_message( const stem::Event& );

    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;
    int n_msg;
    int gsize;
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

template <int N_user, int N_msg>
int EXAM_IMPL( janus_perf::group_send_mt )
{
  n_obj = N_user - 1;
  n_msg = N_msg;
  thr.resize( n_obj );
  names.resize( n_obj );
  res.resize( n_obj );

  std::string name = std::string("/tmp/janus.") + xmt::uid_str();
  VT_with_leader_recovery a( name.c_str() );
  addr = a.self_id();

  a.vs_join( stem::badaddr );

  for (int i = 0;i < n_obj;++i) {
    names[i] = std::string("/tmp/janus.") + xmt::uid_str();
    thr[i] = new std::tr2::thread( run, i );
    res[i] = 0;
  }

  EXAM_CHECK( a.wait_group_size( std::tr2::milliseconds(N_user * 2000), N_user ) );

  {
    stem::Event ev( EV_EXT_EV_SAMPLE );
    ev.dest( a.self_id() );

    for ( int j = 0; j < N_msg; ++j ) {
      std::stringstream v;
      v << j;
      ev.value() = v.str();
      a.Send( ev );
    }

    a.vs_send_flush();
  }

  EXAM_CHECK( a.wait_msg( std::tr2::milliseconds(N_user * N_msg * 100), N_user * N_msg ) );
  EXAM_CHECK( a.wait_flush( std::tr2::milliseconds(N_user * 2000), N_user ) );

  for (int i = 0;i < n_obj;++i) {
    thr[i]->join();
    delete thr[i];
    unlink( names[i].c_str() );
    EXAM_CHECK( res[i] == 0 );
  }

  unlink( name.c_str() );

  return EXAM_RESULT;
}

#endif // __janus_perf_h
