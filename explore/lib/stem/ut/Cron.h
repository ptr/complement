// -*- C++ -*-

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
#include <mutex>
#include <condition_variable>
#include <string>

class CronClient :
    public stem::EventHandler
{
  public:
    CronClient();

    void cron_event( /* std::string arg */ const stem::Event& );

    std::mutex m;
    std::condition_variable cnd;

    std::string see;
    int visited;

  private:
    DECLARE_RESPONSE_TABLE( CronClient, stem::EventHandler );
};

#define TEST_EV_CRON 0x920

#endif // __Cron_h
