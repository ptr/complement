// -*- C++ -*- Time-stamp: <96/03/05 18:51:37 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <X11/Xlib.h>

#include <stl/map.h>

#include <OXW/EvManager.h>
#include <OXW/EventsCore.h>
#include <OXW/WindowBase.h>
#include <OXW/Diagnosis.h>

static map<unsigned long, OXWEventsCore *, less<unsigned long> > windows;

void OXWEvManager::Register( unsigned long wndh, OXWEventsCore *wnd )
{
  windows[ wndh ] = wnd;
}

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

void OXWEvManager::X_dispatch()
{
  XEvent event;
  OXWEventsCore *theWindow;

  XNextEvent( OXWWindowBase::theDisplay, &event );
  if ( event.type == MappingNotify ) {
    XRefreshKeyboardMapping( &event.xmapping );
    return;
  }
  theWindow = windows[ event.xany.window ];
  TRACE_XEV( event );
  WARN( theWindow == 0, "unregistered window: don't know to do..." );
  if ( theWindow != 0 ) {
    theWindow->Notify( OXWEventX( event ) );
  }
}

void OXWEvManager::Done()
{
  MT_MUTEX_LOCK( &lock_not_done );
  if ( !done ) {
    MT_COND_SIGNAL( &done_cond );
    done = true;
  }
  MT_MUTEX_UNLOCK( &lock_not_done );
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
