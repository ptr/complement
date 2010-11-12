// -*- C++ -*- Time-stamp: <2010-11-11 14:43:55 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "syslog_perf.h"
#include <exam/suite.h>

using namespace std;
using namespace std::tr2;

int sockios_syslog_perf::message_count = 0;

void sockios_syslog_perf::syslog_dgram_worker()
{
  // std::string res(message_size, '0');
  // std::ostream& l = misc::use_syslog<LOG_ERR,LOG_USER>();

  for ( int i = 0; i < message_count; ++i ) {
    misc::use_syslog<LOG_ERR,LOG_USER>() << /* res */ 0 << std::endl;
  }
}

extern "C" char* __progname; // from crt0, linux

void sockios_syslog_perf::syslog_classic_worker()
{
  // std::string res(message_size, '0');
  
  openlog( __progname, LOG_PID, LOG_USER );

  for (int i = 0; i < message_count; ++i) {
    syslog( LOG_ERR, "%d" /* res.c_str() */, 0 );
  }

  closelog();
}
