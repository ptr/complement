// -*- C++ -*- Time-stamp: <2010-11-09 16:26:38 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __syslog_perf_h
#define __syslog_perf_h

#include <exam/suite.h>
#include <string>
#include <vector>

//#include <sockios/sockstream>
//#include <sockios/sockmgr.h>

//#include <mt/shm.h>
//#include <sys/wait.h>
//#include <signal.h>
//#include <algorithm>

#include <mt/thread>
//#include <mt/mutex>
//#include <mt/condition_variable>
//#include <mt/date_time>

#include <sockios/syslog.h>

class sockios_syslog_perf
{
  public:
    template <int N, int S, int T, void (*worker)()>
    int EXAM_DECL(syslog_mt);

    static void syslog_dgram_worker();
    static void syslog_classic_worker();

  private:
    static int message_count;
    static int message_size;
};

template <int N, int S, int T, void (*worker)()>
int EXAM_IMPL(sockios_syslog_perf::syslog_mt)
{
  std::vector< std::tr2::thread* > threads(T);

  message_count = N;
  message_size = S;

  for (int thread_index = 0; thread_index < T; ++thread_index) {
    threads[thread_index] = new std::tr2::thread( worker );
  }

  for (int thread_index = 0; thread_index < T; ++thread_index) {
    threads[thread_index]->join();
    delete threads[thread_index];
  }
  
  return EXAM_RESULT;
}

#endif // __syslog_perf_h
