// -*- C++ -*- Time-stamp: <96/11/25 10:47:15 ptr>
#ifndef __EDS_EvManager_h
#define __EDS_EvManager_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifndef __CLASS_lock_h
#include <CLASS/lock.h>
#endif

#ifndef PAIR_H
#include <stl/pair.h>
#endif

#ifndef DEQUE_H
#include <stl/deque.h>
#endif

#ifndef STACK_H
#include <stl/stack.h>
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
    public queue<deque<EDSEvInfo> >
{
    typedef queue<deque<EDSEvInfo> > ParentCls;
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
