// -*- C++ -*- Time-stamp: <08/12/01 23:10:36 ptr>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __Cron_h
#define __Cron_h

#include <stdint.h>
#include <stem/EventHandler.h>
#include <mt/condition_variable>

class CronClient :
    public stem::EventHandler
{
  public:
    CronClient();

    void cron_event( const uint32_t& arg );

    std::tr2::mutex m;
    std::tr2::condition_variable cnd;

    uint32_t see;
    int visited;

  private:
    DECLARE_RESPONSE_TABLE( CronClient, stem::EventHandler );
};

#define TEST_EV_CRON 0x920

#endif // __Cron_h
