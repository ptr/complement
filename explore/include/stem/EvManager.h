// -*- C++ -*- Time-stamp: <96/03/05 18:09:13 ptr>
#ifndef __OXW_EvManager_h
#define __OXW_EvManager_h

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

#ifndef __OXW_Event_h
#include <OXW/Event.h>
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
    OXWEvManager() :
	done( false )
      {
	MT_MUTEX_INIT( &lock_not_empty, USYNC_THREAD );
	MT_COND_INIT( &empty_cond, USYNC_THREAD );
	MT_MUTEX_INIT( &lock_empty, USYNC_THREAD );
	MT_COND_INIT( &not_empty_cond, USYNC_THREAD );
	MT_MUTEX_INIT( &lock_not_done, USYNC_THREAD );
	MT_COND_INIT( &done_cond,  USYNC_THREAD );
      }

    ~OXWEvManager()
      {
	MT_COND_DESTROY( &done_cond );
	MT_MUTEX_DESTROY( &lock_not_done );
	MT_COND_DESTROY( &not_empty_cond );
	MT_MUTEX_DESTROY( &lock_empty );
	MT_COND_DESTROY( &empty_cond );
	MT_MUTEX_DESTROY( &lock_not_empty );
      }

    void Register( unsigned long, OXWEventsCore * );

    void push(const value_type& x)
      {
	MT_MUTEX_LOCK( &lock_empty );
	MT_COND_SIGNAL( &not_empty_cond );
	ParentCls::push( x );
	MT_MUTEX_UNLOCK( &lock_empty );
      }

    bool wait_empty();
    bool wait_not_empty();
    void dispatch();
    void X_dispatch();
    void Done();
    bool isDone()
      { return done; }
    bool wait_done();

  private:
    mutex_t lock_not_empty;
    cond_t  empty_cond;
    mutex_t lock_empty;
    cond_t  not_empty_cond;
    mutex_t lock_not_done;
    cond_t  done_cond;
    bool done;
};

#endif // __OXW_EvManager_h
