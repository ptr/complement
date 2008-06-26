// -*- C++ -*- Time-stamp: <07/08/16 09:02:08 ptr>

#ifndef __vt_operations_h
#define __vt_operations_h

#include <exam/suite.h>
#include <mt/xmt.h>
#include <mt/shm.h>

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
    int EXAM_DECL(mgroups);
    int EXAM_DECL(wellknownhost);

  private:
    const char *fname;
    xmt::shm_alloc<0> seg;
    xmt::allocator_shm<xmt::__condition<true>,0> shm_cnd;
    xmt::allocator_shm<xmt::__barrier<true>,0>   shm_b;
    xmt::__barrier<true> *b2;
};

} // namespace janus

#endif // __vt_operations_h
