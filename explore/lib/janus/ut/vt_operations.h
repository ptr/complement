// -*- C++ -*- Time-stamp: <10/06/10 16:09:52 ptr>

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
    int EXAM_DECL(VT_one_group_core);
    int EXAM_DECL(VT_one_group_core3);
    int EXAM_DECL(VT_one_group_core3_sim);
    int EXAM_DECL(VT_one_group_send);
    int EXAM_DECL(VT_one_group_multiple_send);
    int EXAM_DECL(VT_one_group_join_exit);
    int EXAM_DECL(VT_one_group_join_send);
    int EXAM_DECL(VT_one_group_multiple_joins);
    int EXAM_DECL(VT_one_group_replay);
    int EXAM_DECL(VT_one_group_late_replay);

    int EXAM_DECL(VT_one_group_network);
    int EXAM_DECL(VT_one_group_access_point);

    int EXAM_DECL(VT_one_group_recover);

    int EXAM_DECL(leader);
    int EXAM_DECL(leader_fail);

    int EXAM_DECL( double_flush );
    int EXAM_DECL( double_exit );
    int EXAM_DECL( flush_and_join );
    int EXAM_DECL( flush_and_exit );
    int EXAM_DECL( lock_and_torder );
};

} // namespace janus

#endif // __vt_operations_h
