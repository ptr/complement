// -*- C++ -*- Time-stamp: <97/09/26 15:39:50 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <CLASS/checks.h>
#include <EDS/EventHandler.h>

// #include <iostream.h>
#if defined(__TRACE) || defined(__WARN)

// DIAG_DEFINE_GROUP(EventHandler,0,4);
// DIAG_DEFINE_GROUP(EventDispatch,0,3);

#endif // __TRACE || __WARN

EDSEventHandler::evtable_type EDSEventHandler::theEventsTable(
  "EDSEventHandler", EDSEventHandler::theDeclEventsTable );

__DeclareAnyPMF<EDSEventHandler> EDSEventHandler::theDeclEventsTable[] = {
  { 0, 0, { 0, 0, "End of table" }}
};

DEFINE_NAME_IT( EDSEventHandler );

void EDSEventHandler::PushState( state_type state )
{
  RemoveState( state );
  theHistory.push_front( state );
}

state_type EDSEventHandler::State() const
{
  return theHistory.front();
}

void EDSEventHandler::PushTState( state_type state )
{
  theHistory.push_front( ST_TERMINAL );
  theHistory.push_front( state );
}

void EDSEventHandler::PopState()
{
  theHistory.pop_front();
  while ( theHistory.front() == ST_TERMINAL ) {
    theHistory.pop_front();
  }
}

void EDSEventHandler::PopState( state_type state )
{
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( theHistory.begin(), ++hst_i );
  }
}

void EDSEventHandler::RemoveState( state_type state )
{
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( hst_i );
  }
}

bool EDSEventHandler::isState( state_type state ) const
{
  const HistoryContainer& hst = theHistory;
  const_h_iterator hst_i = __find( state );
  if ( hst_i != hst.end() && *hst_i != ST_TERMINAL ) {
    return true;
  }
  return false;
}

h_iterator EDSEventHandler::__find( state_type state )
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

const_h_iterator EDSEventHandler::__find( state_type state ) const
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

// Trick with reference to theHistory instead of object itself is here due to
// SunPro's C++ 4.1 bug (its occur only for large enough programm):
// instances of theHistory in inline and outline methods are not same!!!
// Of cause, you can found this only if theHistory is a template with static
// member (stl/list, for example).

EDSEventHandler::EDSEventHandler() :
    theHistory( *(new HistoryContainer()) )
{
  State( ST_NULL );
}

EDSEventHandler::~EDSEventHandler()
{
  delete &theHistory;
}

bool EDSEventHandler::Dispatch( const EDSEvent& event )
{
  return theEventsTable.Dispatch( this, theHistory.begin(),theHistory.end(),
				  event );
}

bool EDSEventHandler::DispatchStub( const EDSEvent& event )
{
  return theEventsTable.DispatchStub( this, theHistory.begin(),
				      theHistory.end(), event );
}
void EDSEventHandler::DispatchTrace( const EDSEvent& __event__,
				     ostrstream& out )
{
  theEventsTable.DispatchTrace( theHistory.begin(),theHistory.end(),
				__event__, out );
}

void EDSEventHandler::TraceStack( ostrstream& out ) const
{
  const HistoryContainer& hst = theHistory;
  HistoryContainer::const_iterator hst_i = hst.begin();
  while ( hst_i != hst.end() ) {
    out << "[" << *hst_i++ << "]";
  }
  out << endl;
}

void EDSEventHandler::Trace( ostrstream& out ) const
{
  EDSEventHandler::theEventsTable.Out( out );
}
