// -*- C++ -*- Time-stamp: <96/10/07 19:28:48 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EDS/EvManager.h>
#include <EDS/EventsCore.h>

void OXWEvManager::dispatch()
{
  queue_lock.wait();
  while ( !empty() ) {
    // queue_sem.wait();
    OXWEvInfo& top = front();
    switch ( top.first.dtype ) {
      case OXWEvTarget::Local:
	switch ( top.first.ctype ) {
	  case OXWEvTarget::This:
	    top.first.target->Notify( top.second );
	    break;
	  case OXWEvTarget::BranchChilds:
	    top.first.target->BranchNotifyChilds( top.second );
	    break;
	  case OXWEvTarget::Branch:
	    top.first.target->BranchNotify( top.second );
	    break;
	  case OXWEvTarget::Siblings:
	    top.first.target->SiblingNotify( top.second );
	    break;
	  case OXWEvTarget::Parent:
	    top.first.target->ParentNotify( top.second );
	    break;
	}
	break;
      case OXWEvTarget::Global:
	WARN( true, "Global events not accepted yet" );
	break;
    }
    queue_lock.lock();
    pop();
    queue_lock.unlock();
  }

  queue_lock.set_condition();
}

void OXWEvManager::Done()
{
  lock_not_done.signal();
}

bool OXWEvManager::wait_done()
{
  lock_not_done.wait();

  return lock_not_done.get_condition();
}
