// -*- C++ -*- Time-stamp: <96/09/09 14:45:36 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EDS/EvManager.h>
#include <EDS/EventsCore.h>

void OXWEvManager::dispatch()
{
  MT_MUTEX_LOCK( &lock_not_empty );
  MT_COND_SIGNAL( &empty_cond );
  while ( !empty() ) {
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
    pop();
  }
  MT_MUTEX_UNLOCK( &lock_not_empty );
}

void OXWEvManager::Done()
{
  // MT_MUTEX_LOCK( &lock_not_done );
  if ( !done ) {
    MT_COND_SIGNAL( &done_cond );
    done = true;
  }
  // MT_MUTEX_UNLOCK( &lock_not_done );
}

bool OXWEvManager::wait_empty()
{
  MT_MUTEX_LOCK( &lock_not_empty );
#ifdef _REENTRANT
  while ( !empty() ) {
    MT_COND_WAIT( &empty_cond, &lock_not_empty );
  }
#endif
  MT_MUTEX_UNLOCK( &lock_not_empty );

  return !done;
}

bool OXWEvManager::wait_not_empty()
{
  MT_MUTEX_LOCK( &lock_empty );
#ifdef _REENTRANT
  while ( empty() ) {
    MT_COND_WAIT( &not_empty_cond, &lock_empty );
  }
#endif
  MT_MUTEX_UNLOCK( &lock_empty );

  return !done;
}

bool OXWEvManager::wait_done()
{
  MT_MUTEX_LOCK( &lock_not_done );
#ifdef _REENTRANT
  while ( !done ) {
    MT_COND_WAIT( &done_cond, &lock_not_done );
  }
#endif
  MT_MUTEX_UNLOCK( &lock_not_done );

  return done;
}
