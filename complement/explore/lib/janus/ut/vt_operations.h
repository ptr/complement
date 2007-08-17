// -*- C++ -*- Time-stamp: <07/08/16 09:02:08 ptr>

#ifndef __vt_operations_h
#define __vt_operations_h

#include <exam/suite.h>

struct vtime_operations
{
  int EXAM_DECL(vt_compare);
  int EXAM_DECL(vt_add);
  int EXAM_DECL(vt_diff);
  int EXAM_DECL(vt_max);

  int EXAM_DECL(gvt_add);

  int EXAM_DECL(VTMess_core);

  int EXAM_DECL(vt_object);

  int EXAM_DECL(VTDispatch1);
  int EXAM_DECL(VTDispatch2);

  int EXAM_DECL(VTHandler1);
  int EXAM_DECL(VTHandler2);

  int EXAM_DECL(VTSubscription);
  int EXAM_DECL(VTEntryIntoGroup);
  int EXAM_DECL(VTEntryIntoGroup2);
  int EXAM_DECL(VTEntryIntoGroup3);

  int EXAM_DECL(remote);
};

#endif // __vt_operations_h