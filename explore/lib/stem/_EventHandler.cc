// -*- C++ -*- Time-stamp: <99/03/24 18:04:11 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EventHandler.h>
#include <EvManager.h>

#if defined(__TRACE) || defined(__WARN)

// DIAG_DEFINE_GROUP(EventHandler,0,4);
// DIAG_DEFINE_GROUP(EventDispatch,0,3);

#endif // __TRACE || __WARN

namespace EDS {

EventHandler::evtable_type EventHandler::theEventsTable( EventHandler::theDeclEventsTable );

__DeclareAnyPMF<EventHandler> EventHandler::theDeclEventsTable[] = {
  { 0, 0, { 0, 0, "End of table" }}
};

char *Init_buf[32];
EvManager *EventHandler::_mgr = 0;

int EventHandler::Init::_count = 0;

EventHandler::Init::Init()
{
  if ( _count++ == 0 ) {
    _mgr = new EvManager();
  }
}

EventHandler::Init::~Init()
{
  if ( --_count == 0 ) {
    delete _mgr;
  }
}

const string& EventHandler::who_is( const Event::key_type& k ) const
{
  return _mgr->who_is( k );
}

unsigned EventHandler::sid( const Event::key_type& k ) const
{
  return _mgr->sid( k );
}

void EventHandler::Send( const Event& e )
{
  _mgr->Send( e, _id );
}

void EventHandler::PushState( state_type state )
{
  RemoveState( state );
  theHistory.push_front( state );
}

state_type EventHandler::State() const
{
  return theHistory.front();
}

void EventHandler::PushTState( state_type state )
{
  theHistory.push_front( ST_TERMINAL );
  theHistory.push_front( state );
}

void EventHandler::PopState()
{
  theHistory.pop_front();
  while ( theHistory.front() == ST_TERMINAL ) {
    theHistory.pop_front();
  }
}

void EventHandler::PopState( state_type state )
{
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( theHistory.begin(), ++hst_i );
  }
}

void EventHandler::RemoveState( state_type state )
{
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( hst_i );
  }
}

bool EventHandler::isState( state_type state ) const
{
  const HistoryContainer& hst = theHistory;
  const_h_iterator hst_i = __find( state );
  if ( hst_i != hst.end() && *hst_i != ST_TERMINAL ) {
    return true;
  }
  return false;
}

h_iterator EventHandler::__find( state_type state )
{
  if ( theHistory.empty() ) {
    return theHistory.end();
  }
  h_iterator hst_i = theHistory.begin();

  while ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL
	  && *hst_i != state ) {
    ++hst_i;
  }
  return hst_i;
}

const_h_iterator EventHandler::__find( state_type state ) const
{
  if ( theHistory.empty() ) {
    return theHistory.end();
  }
  const_h_iterator hst_i = theHistory.begin();

  while ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL
	  && *hst_i != state ) {
    ++hst_i;
  }
  return hst_i;
}

// Trick with reference to theHistory instead of object itself is here due to
// SunPro's C++ 4.1 bug (its occur only for large enough programm):
// instances of theHistory in inline and outline methods are not same!!!
// Of cause, you can found this only if theHistory is a template with static
// member (stl/list, for example).

EventHandler::EventHandler()// :
//    theHistory( *(new HistoryContainer()) )
{
  new( Init_buf ) Init();
  State( ST_NULL );
  _id = _mgr->Subscribe( this, "" );
}

EventHandler::EventHandler( const Event::key_type& id )// :
//    theHistory( *(new HistoryContainer()) )
{
  new( Init_buf ) Init();
  State( ST_NULL );
  _id = _mgr->SubscribeID( id, this, "" );
  __stl_assert( _id != -1 ); // already registered, or id has Event::extbit
//  if ( _id == -1 ) {
//    _mgr->Subscribe( this, "" );
//  }
}

EventHandler::~EventHandler()
{
//  delete &theHistory;
   _mgr->Unsubscribe( _id );
  ((Init *)Init_buf)->~Init();
}

bool EventHandler::Dispatch( const Event& event )
{
  return theEventsTable.Dispatch( this, theHistory.begin(),theHistory.end(),
				  event );
}

bool EventHandler::DispatchStub( const Event& event )
{
  return theEventsTable.DispatchStub( this, theHistory.begin(),
				      theHistory.end(), event );
}
void EventHandler::DispatchTrace( const Event& __event__, ostream& out )
{
  theEventsTable.DispatchTrace( theHistory.begin(),theHistory.end(),
				__event__, out );
}

void EventHandler::TraceStack( ostream& out ) const
{
  const HistoryContainer& hst = theHistory;
  HistoryContainer::const_iterator hst_i = hst.begin();
  while ( hst_i != hst.end() ) {
    out << "[" << *hst_i++ << "]";
  }
  out << endl;
}

void EventHandler::Trace( ostream& out ) const
{
  EventHandler::theEventsTable.Out( out );
}

} // namespace EDS
