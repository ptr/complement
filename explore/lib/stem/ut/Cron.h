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
#include <string>

class CronClient :
    public stem::EventHandler
{
  public:
    CronClient();

    void cron_event( /* std::string arg */ const stem::Event& );

    std::tr2::mutex m;
    std::tr2::condition_variable cnd;

    std::string see;
    int visited;

  private:
    DECLARE_RESPONSE_TABLE( CronClient, stem::EventHandler );
};

#define TEST_EV_CRON 0x920

#endif // __Cron_h
