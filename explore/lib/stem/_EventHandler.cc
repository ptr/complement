// -*- C++ -*- Time-stamp: <96/02/28 11:38:24 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <OXW/EventHandler.h>
#include <OXW/OXWEvents.h>
#include <CLASS/checks.h>

// #include <iostream.h>
#if defined(__TRACE) || defined(__WARN)

DIAG_DEFINE_GROUP(EventHandler,0,4);
DIAG_DEFINE_GROUP(EventDispatch,0,3);

#endif // __TRACE || __WARN

OXWEventHandler::evtable_type OXWEventHandler::theEventsTable(
  "OXWEventHandler", OXWEventHandler::theDeclEventsTable );

__DeclareAnyPMF<OXWEventHandler> OXWEventHandler::theDeclEventsTable[] = {
  { 0, 0, { 0, 0, "End of table" }}
};

DEFINE_NAME_IT( OXWEventHandler );

void OXWEventHandler::PushState( state_type state )
{
  RemoveState( state );
  theHistory.push_back( state );
}

state_type OXWEventHandler::State() const
{
  return theHistory.back();
}

void OXWEventHandler::PushTState( state_type state )
{
  theHistory.push_back( ST_TERMINAL );
  theHistory.push_back( state );
}

void OXWEventHandler::PopState()
{
  theHistory.pop_back();
  while ( theHistory.back() == ST_TERMINAL ) {
    theHistory.pop_back();
  }
}

void OXWEventHandler::PopState( state_type state )
{
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( hst_i, theHistory.end() );
  }
}

void OXWEventHandler::RemoveState( state_type state )
{
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( hst_i );
  }
}

bool OXWEventHandler::isState( state_type state ) const
{
  const HistoryContainer& hst = theHistory;
  HistoryContainer::const_reverse_iterator hst_i = hst.rbegin();

  while ( hst_i != hst.rend() && *hst_i != ST_TERMINAL ) {
    if ( *hst_i++ == state ) {
      return true;
    }
  }
  return false;
}

h_iterator OXWEventHandler::__find( state_type state )
{
  if ( theHistory.empty() ) {
    return theHistory.end();
  }
  h_iterator hst_i = theHistory.end();

  while ( --hst_i != theHistory.begin() && *hst_i != ST_TERMINAL ) {
    if ( *hst_i == state ) {
      return hst_i;
    }
  }
  if ( *hst_i != state ) {
    return theHistory.end();
  }
  return hst_i;
}

// Trick with reference to theHistory instead of object itself is here due to
// SunPro's C++ 4.1 bug (its occur only for large enough programm):
// instances of theHistory in inline and outline methods are not same!!!
// Of cause, you can found this only if theHistory is a template with static
// member (stl/list, for example).

OXWEventHandler::OXWEventHandler() :
    theHistory( *(new HistoryContainer()) )
{
  State( ST_NULL );
}

OXWEventHandler::~OXWEventHandler()
{
  delete &theHistory;
}

bool OXWEventHandler::Dispatch( OXWEvent& event )
{
  return theEventsTable.Dispatch( this, theHistory.begin(),
				  theHistory.end(), event );
}

void OXWEventHandler::DispatchTrace( OXWEvent& __event__, ostrstream& out )
{
  theEventsTable.DispatchTrace( theHistory.begin(),
	 		        theHistory.end(), __event__, out );
}

void OXWEventHandler::TraceStack( ostrstream& out ) const
{
  const HistoryContainer& hst = theHistory;
  HistoryContainer::const_reverse_iterator hst_i = hst.rbegin();
  while ( hst_i != hst.rend() ) {
    out << "[" << *hst_i++ << "]";
  }
  out << endl;
}

void OXWEventHandler::Trace( ostrstream& out ) const
{
  OXWEventHandler::theEventsTable.Out( out );
}
