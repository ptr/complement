#ident "%Z%%Q% $RCSfile$ v$Revision$ %H% %T%"

#include <OXW/EventHandler.h>
#include <OXW/OXWEvents.h>
#include <CLASS/checks.h>

#include <strstream.h>

GENERIC_StatesTbl OXWEventHandler::theStatesTable = OXWEventHandler::RTConfigure();
int OXWEventHandler::ini_flag = 0;

GENERIC_StatesTbl OXWEventHandler::RTConfigure()
{
  GENERIC_StatesTbl tmp;
  return tmp;
}


void OXWEventHandler::PopState()
{
  theHistory.pop_back();
  while ( theHistory.back() == ST_TERMINAL ) {
    theHistory.pop_back();
  }
}

OXWEventHandler::OXWEventHandler()
{
  State( ST_NULL );
}

bool OXWEventHandler::__Dispatch( state_type state, OXWEvent& event )
{
  TRACE( "Dispatch " << isA() << "\n" <<  theStatesTable.PrintContents() );
  return theStatesTable.Dispatch( *((GENERIC *)this), state, event );
}

bool OXWEventHandler::Dispatch( OXWEvent& event )
{
  {
    list<state_type>::reverse_iterator hst_i = theHistory.rbegin();
    ostrstream buf;
    buf << "State stack (" << isA() << "):\n";
    while ( hst_i != theHistory.rend() ) {
      buf << "\tstate: " << *hst_i << "\n";
      ++hst_i;
    }
    buf << ends;
    TRACE( buf.str() );
  }
  list<state_type>::reverse_iterator hst_i = theHistory.rbegin();
  bool d = 0;
  while ( hst_i != theHistory.rend() && *hst_i != ST_TERMINAL && 
          !(d =__Dispatch( *hst_i, event ) ) ) {
    ++hst_i;
  }
  if ( !d && State() == ST_NULL ) {
    TRACE( "Sorry, init now...(" << isA() << ")" );
    OXWEvRTInit( OXWEvent( OXW_RTINIT ) );
    d = Dispatch( event );
  }
  return d;
}

void OXWEventHandler::OXWEvRTInit( OXWEvent& )
{
  OXWRTInit();
  State( ST_READY );
}

void OXWEventHandler::OXWRTInit()
{ // do nothing: this is stub
}
