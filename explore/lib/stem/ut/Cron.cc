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
    see( 0 ),
    visited( 0 )
{
}

void CronClient::cron_event( const uint32_t& arg )
{
  unique_lock<mutex> lk( m );

  see = arg;
  ++visited;

  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( CronClient )
  EV_T_( ST_NULL, TEST_EV_CRON, cron_event, uint32_t )
END_RESPONSE_TABLE
