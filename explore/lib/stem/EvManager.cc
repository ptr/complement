// -*- C++ -*- Time-stamp: <96/11/25 10:50:41 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EDS/EvManager.h>
#include <EDS/EventsCore.h>

EDSEvManager::EDSEvManager()
{
  lock_not_done.set_condition();
  queue_lock.set_condition();
}

void EDSEvManager::push(const EDSEvManager::value_type& x)
{
  queue_lock.lock();
  ParentCls::push( x );
  queue_lock._signal();
  queue_lock.unlock();
}

bool EDSEvManager::empty()
{
  MT_REENTRANT( queue_lock, lck );
  return ParentCls::empty();
}

void EDSEvManager::dispatch()
{
  queue_lock.wait();
  while ( !empty() ) {
    EDSEvInfo& top = front();
    switch ( top.first.dtype ) {
      case EDSEvTarget::Local:
	switch ( top.first.ctype ) {
	  case EDSEvTarget::This:
	    top.first.target->Notify( top.second );
	    break;
	  case EDSEvTarget::BranchChilds:
	    top.first.target->BranchNotifyChilds( top.second );
	    break;
	  case EDSEvTarget::Branch:
	    top.first.target->BranchNotify( top.second );
	    break;
	  case EDSEvTarget::Siblings:
	    top.first.target->SiblingNotify( top.second );
	    break;
	  case EDSEvTarget::Parent:
	    top.first.target->ParentNotify( top.second );
	    break;
	}
	break;
      case EDSEvTarget::Global:
	WARN( true, "Global events not accepted yet" );
	break;
    }
    queue_lock.lock();
    pop();
    queue_lock.unlock();
  }

  queue_lock.set_condition();
}

void EDSEvManager::Done()
{
  lock_not_done._signal();
  // queue_lock._signal();
}

bool EDSEvManager::wait_done()
{
  lock_not_done.wait();

  return lock_not_done.get_condition();
}
