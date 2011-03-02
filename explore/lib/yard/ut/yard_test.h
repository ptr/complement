// -*- C++ -*- Time-stamp: <2011-03-03 00:44:04 ptr>

/*
 * Copyright (c) 2010-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <exam/suite.h>

struct yard_block_test
{
    int EXAM_DECL(block_type_lookup);
    int EXAM_DECL(block_type_route);
    int EXAM_DECL(block_type_divide);
    int EXAM_DECL(pack_unpack);
};

struct yard_btree_test
{
    int EXAM_DECL(btree_basic);
    int EXAM_DECL(btree_random);
    int EXAM_DECL(btree_init_existed);

    int EXAM_DECL(insert_extract);
    int EXAM_DECL(bad_key);
    int EXAM_DECL(open_modes);
};

class yard_test
{
  public:
    int EXAM_DECL(pack_test);

    // yard interface
    int EXAM_DECL(revision);
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

    // yard on disk
    int EXAM_DECL(core_life_cycle);
    int EXAM_DECL(clear_mod_flag);
    int EXAM_DECL(core_life_cycle_single_leaf);
    int EXAM_DECL(create);

    int EXAM_DECL(not_open_bug1);
};
