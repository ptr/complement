// -*- C++ -*- Time-stamp: <09/02/04 19:23:47 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __stem_perf_h
#define __stem_perf_h

#include <exam/suite.h>
#include <string>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

#include <stem/EventHandler.h>

class Tester :
    public stem::EventHandler
{
  public:
    Tester();
    Tester( stem::addr_type id );
    Tester( stem::addr_type id, const char* );

    void handler1( const stem::Event& );
    void handler2( const stem::Event& );
    // void regme( const stem::Event& );

    // std::tr2::condition_event cnd;

    bool wait_peer();
    void send();

  public:
    static bool n_cnt();

    static std::tr2::mutex lock;
    static std::tr2::condition_variable cnd;
    static int visits;

  private:
    class pred
    {
      public:
        pred( Tester& t ) :
            tester( &t )
          { }

        bool operator()() const
          { return tester->peer != stem::badaddr; }

      private:
        Tester* tester;
    };

    std::tr2::mutex peer_lock;
    std::tr2::condition_variable peer_cnd;

    stem::addr_type peer;

    friend class pred;

    DECLARE_RESPONSE_TABLE( Tester, stem::EventHandler );
};

class stem_perf
{
  public:
    stem_perf();
    ~stem_perf();

    int EXAM_DECL(local);
    int EXAM_DECL(local_too);
    int EXAM_DECL(net_loopback);
    int EXAM_DECL(net_loopback_inv);
    int EXAM_DECL(net_loopback_inv2);

  private:
    // Tester tester;
};

#endif // __stem_perf_h
