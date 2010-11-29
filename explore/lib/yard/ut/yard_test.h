// -*- C++ -*- Time-stamp: <2010-11-29 20:02:01 ptr>

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

    // initial yard
    int EXAM_DECL(create);
    int EXAM_DECL(put);
    int EXAM_DECL(put_object);

    int EXAM_DECL(manifest);

    // int EXAM_DECL(manifest_check);
};
