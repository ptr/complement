// -*- C++ -*- Time-stamp: <07/07/26 09:40:39 ptr>

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
};

#endif // __vt_operations_h
