// -*- C++ -*- Time-stamp: <98/01/20 21:21:29 ptr>
#ifndef __EDS_EvManager_h
#define __EDS_EvManager_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifndef __CLASS_lock_h
#include <CLASS/lock.h>
#endif

#ifndef __UTILITY__
#include <utility>
#endif

#ifndef __QUEUE__
#include <queue>
#endif

#ifndef __EDS_Event_h
#include <EDS/Event.h>
#endif

class EDSEventsCore;

struct EDSEvTarget
{
    enum CallType { Parent, Siblings, Branch, BranchChilds, This};
    enum DestType { Local, Global };

    EDSEvTarget( EDSEventsCore *x, CallType ct, DestType dt = Local ) :
	target( x ),
	ctype( ct ),
	dtype( dt )
      { }

    EDSEventsCore *target;
    CallType ctype;
    DestType dtype;
};

typedef pair<EDSEvTarget,EDSEvent> EDSEvInfo;

class EDSEvManager :
    public queue<EDSEvInfo,deque<EDSEvInfo> >
{
    typedef queue<EDSEvInfo,deque<EDSEvInfo> > ParentCls;
  public:
    EDSEvManager();

    void push(const value_type& x);
    bool empty();

    void dispatch();
    void Done();
    bool isDone()
      { return lock_not_done.get_condition(); }
    bool wait_done();

  private:
    MutexCondition queue_lock;
    MutexCondition lock_not_done;
};

#endif // __EDS_EvManager_h
