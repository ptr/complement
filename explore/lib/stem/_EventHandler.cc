#ident "%Z%%Q% $RCSfile$ v$Revision$ %H% %T%"

#include <OXW/EventHandler.h>
#include <OXW/OXWEvents.h>
#include <CLASS/checks.h>

// #include <iostream.h>

char trace_buffer[4096];

ostrstream OXWEventHandler::Out( trace_buffer, 4096 );
GENERIC_StatesTbl OXWEventHandler::theStatesTable;

DEFINE_NAME_IT( OXWEventHandler );

int OXWEventHandler::RespTblConfigure()
{
  return 1;
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
  list<state_type>::iterator hst_i = theHistory.begin();

  while ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    if ( *hst_i == state ) {
      theHistory.erase( hst_i, theHistory.end() );
      return;
    }
    ++hst_i;
  }
}

bool OXWEventHandler::isState( state_type state ) const
{
  list<state_type>::const_reverse_iterator hst_i = theHistory.rbegin();

  while ( hst_i != theHistory.rend() && *hst_i != ST_TERMINAL ) {
    if ( *hst_i == state ) {
      return 1;
    }
    ++hst_i;
  }
  return 0;
}

OXWEventHandler::OXWEventHandler()
{
  State( ST_NULL );
}

bool OXWEventHandler::__Dispatch( state_type state, OXWEvent& event )
{
  return theStatesTable.Dispatch( *((GENERIC *)this), state, event );
}

bool OXWEventHandler::Dispatch( OXWEvent& event )
{
//  cerr <<  "........\n" << Trace() << endl;
  list<state_type>::reverse_iterator hst_i = theHistory.rbegin();
  bool d = 0;
  while ( hst_i != theHistory.rend() && *hst_i != ST_TERMINAL && 
          !(d =__Dispatch( *hst_i, event ) ) ) {
    ++hst_i;
  }
  return d;
}

void OXWEventHandler::TraceStack( ostrstream& out ) const
{
  list<state_type>::const_reverse_iterator hst_i = theHistory.rbegin();
  out << "State stack (" << isA() << "):\n";
  while ( hst_i != theHistory.rend() ) {
    out << "\tstate: " << *hst_i << "\n";
    ++hst_i;
  }
}

const char *OXWEventHandler::Trace() const
{
  Out.seekp( 0, ostream::beg );
  TraceStack( Out );
  OXWEventHandler::theStatesTable.Out( Out );
  Out << ends;

  return Out.str();
}
