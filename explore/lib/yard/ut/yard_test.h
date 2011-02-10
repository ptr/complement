// -*- C++ -*- Time-stamp: <2011-02-10 16:34:02 ptr>

/*
 * Copyright (c) 2010-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <exam/suite.h>

class yard_test
{
  public:
    int EXAM_DECL(pack_test);

    int EXAM_DECL(block_type_lookup);
    int EXAM_DECL(block_type_route);
    int EXAM_DECL(block_type_divide);
    int EXAM_DECL(pack_unpack);

    int EXAM_DECL(btree_basic);
    int EXAM_DECL(btree_random);
    int EXAM_DECL(btree_init_existed);
    // yard interface
    int EXAM_DECL(revision_in_memory);
    int EXAM_DECL(manifest_from_revision);
    int EXAM_DECL(diff_from_revision);
    int EXAM_DECL(commit_from_revision1);
    int EXAM_DECL(commit_from_revision2);
    int EXAM_DECL(access);
    int EXAM_DECL(linear_commits);
    int EXAM_DECL(linear_commits_neg);
    int EXAM_DECL(diff);
    int EXAM_DECL(fast_merge1);
    int EXAM_DECL(fast_merge2);
    int EXAM_DECL(fast_merge3);
    int EXAM_DECL(fast_merge4);
    int EXAM_DECL(fast_merge_conflict1);
    int EXAM_DECL(heads);
    int EXAM_DECL(merge1);
};
