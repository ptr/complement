// -*- C++ -*- Time-stamp: <09/02/03 16:32:16 ptr>

/*
 *
 * Copyright (c) 2007, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_perf.h"
#include <exam/suite.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

using namespace std;
using namespace std::tr2;

sockios_perf_SrvR::sockios_perf_SrvR()
{
}

sockios_perf_SrvR::~sockios_perf_SrvR()
{
}

sockios_perf_SrvW::sockios_perf_SrvW()
{
}

sockios_perf_SrvW::~sockios_perf_SrvW()
{
}

sockios_perf_SrvRW::sockios_perf_SrvRW()
{
}

sockios_perf_SrvRW::~sockios_perf_SrvRW()
{
}

