// -*- C++ -*- Time-stamp: <96/10/09 18:34:54 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EDS/EvManager.h>
#include <EDS/EventsCore.h>

void EDSEvManager::dispatch()
{
  queue_lock.wait();
  while ( !empty() ) {
    // queue_sem.wait();
    OXWEvInfo& top = front();
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
  lock_not_done.signal();
}

bool EDSEvManager::wait_done()
{
  lock_not_done.wait();

  return lock_not_done.get_condition();
}
