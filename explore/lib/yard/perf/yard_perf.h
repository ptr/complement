// -*- C++ -*- Time-stamp: <2011-03-04 18:48:26 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __yard_perf_h
#define __yard_perf_h

#include <exam/suite.h>
#include <string>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>
#include <mt/uid.h>

#include <stack>

class yard_perf
{
  public:
    yard_perf();
    ~yard_perf();

    int EXAM_DECL(packing);
    int EXAM_DECL(unpacking);

    int EXAM_DECL(consecutive_insert);
    int EXAM_DECL(consecutive_insert_big);
    int EXAM_DECL(random_insert_big);
    int EXAM_DECL(consecutive_insert_with_data);
    int EXAM_DECL(random_insert_with_data_1000);
    int EXAM_DECL(random_insert_with_data_4000);
    int EXAM_DECL(random_insert_with_data_20000);
    int EXAM_DECL(random_insert_with_data_100000);
    int EXAM_DECL(multiple_files);

    int EXAM_DECL(prepare_lookup_tests);
    int EXAM_DECL(random_lookup);
    int EXAM_DECL(lookup_existed_keys);
    int EXAM_DECL(random_lookup_small);
    int EXAM_DECL(lookup_existed_keys_small);

    int EXAM_DECL(mess);
    int EXAM_DECL(put_revisions);
    int EXAM_DECL(mess_insert);
    int EXAM_DECL(mess_insert_single_commit);

    int EXAM_DECL(insert_1000_transactions);
    int EXAM_DECL(insert_4000_transactions);
    int EXAM_DECL(insert_20000_transactions);

    int EXAM_DECL(drop_caches);

    void gen_random_insert_with_data(int count);
    void gen_insert_with_history(int count);
  private:
    std::map<std::string, std::vector<xmt::uuid_type> > inserted_keys;
    std::vector<std::string> created_files;

    std::stack<std::string> random_lookup_files;
    std::stack<std::string> lookup_existed_files;

    std::stack<std::string> random_lookup_files_small;
    std::stack<std::string> lookup_existed_files_small;
};

#endif // __yard_perf_h
