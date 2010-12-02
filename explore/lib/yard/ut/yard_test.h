// -*- C++ -*- Time-stamp: <2010-12-02 20:00:01 ptr>

/*
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <exam/suite.h>

class yard_test
{
  public:
    // yard ng
    int EXAM_DECL(revision_in_memory);
    int EXAM_DECL(access);
    int EXAM_DECL(linear_commits);
    int EXAM_DECL(linear_commits_neg);
    int EXAM_DECL(diff);
    int EXAM_DECL(fast_merge1);
    int EXAM_DECL(fast_merge2);
    int EXAM_DECL(fast_merge3);
    int EXAM_DECL(fast_merge4);
    int EXAM_DECL(fast_merge_conflict1);

    // initial yard
    int EXAM_DECL(create);
    int EXAM_DECL(put);
    int EXAM_DECL(put_object);

    int EXAM_DECL(manifest);

    // int EXAM_DECL(manifest_check);
};
