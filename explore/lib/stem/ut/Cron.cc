// -*- C++ -*- Time-stamp: <08/12/01 23:13:51 ptr>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "Cron.h"

#include <stem/Cron.h>
#include <mt/mutex>

using namespace std;
using namespace stem;
using namespace std::tr2;

CronClient::CronClient() :
    see(),
    visited( 0 )
{
}

void CronClient::cron_event( /* string arg */ const stem::Event& ev )
{
  unique_lock<mutex> lk( m );

  see = /* arg */ ev.value();

  ++visited;

  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( CronClient )
  // EV_T_( ST_NULL, TEST_EV_CRON, cron_event, string )
  EV_EDS( ST_NULL, TEST_EV_CRON, cron_event )
END_RESPONSE_TABLE
