// -*- C++ -*- Time-stamp: <10/02/03 12:59:49 ptr>

/*
 *
 * Copyright (c) 2008-2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __vt_operations_h
#define __vt_operations_h

#include <exam/suite.h>

namespace janus {

class vtime_operations
{
  public:
    vtime_operations();
    ~vtime_operations();

    int EXAM_DECL(vt_compare);
    int EXAM_DECL(vt_add);
    int EXAM_DECL(vt_diff);
    int EXAM_DECL(vt_max);

    int EXAM_DECL(VT_one_group_core);
    int EXAM_DECL(VT_one_group_core3);
    int EXAM_DECL(VT_one_group_core3_sim);
    int EXAM_DECL(VT_one_group_send);
    int EXAM_DECL(VT_one_group_replay);
    int EXAM_DECL(VT_one_group_late_replay);

    int EXAM_DECL(VT_one_group_network);
    int EXAM_DECL(VT_one_group_access_point);

    int EXAM_DECL(VT_one_group_recover);

    int EXAM_DECL(gvt_add);

    int EXAM_DECL(leader);
    int EXAM_DECL(leader_fail);

    int EXAM_DECL( double_flush );
    int EXAM_DECL( flush_and_join );

  private:
    // xmt::shm_alloc<0> seg;
    // xmt::allocator_shm<xmt::__condition<true>,0> shm_cnd;
    // xmt::allocator_shm<xmt::__barrier<true>,0>   shm_b;
    // xmt::__barrier<true> *b2;
};

} // namespace janus

#endif // __vt_operations_h
