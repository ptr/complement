// -*- C++ -*- Time-stamp: <96/10/07 19:29:14 ptr>
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

class OXWEventsCore;

struct OXWEvTarget
{
    enum CallType { Parent, Siblings, Branch, BranchChilds, This};
    enum DestType { Local, Global };

    OXWEvTarget( OXWEventsCore *x, CallType ct, DestType dt = Local ) :
	target( x ),
	ctype( ct ),
	dtype( dt )
      { }

    OXWEventsCore *target;
    CallType ctype;
    DestType dtype;
};

typedef pair<OXWEvTarget,OXWEvent> OXWEvInfo;

class OXWEvManager :
    public queue<deque<OXWEvInfo> >
{
    typedef queue<deque<OXWEvInfo> > ParentCls;
  public:
    OXWEvManager()
      { lock_not_done.set_condition(); queue_lock.set_condition(); }

    ~OXWEvManager()
      { }

    void Register( unsigned long, OXWEventsCore * );

    void push(const value_type& x)
      {
	// queue_lock.lock();
	ParentCls::push( x );
	// queue_lock.unlock();
	queue_lock._signal();
	// queue_sem.post();
      }
    bool empty()
      {
	MT_REENTRANT( queue_lock, lck );
	return ParentCls::empty();
      }

    void dispatch();
    void Done();
    bool isDone()
      { return lock_not_done.get_condition(); }
    bool wait_done();

  private:
    MutexCondition queue_lock;
    MutexCondition lock_not_done;
    // Semaphore queue_sem;
};

#endif // __OXW_EvManager_h
