#ident "%Z%%Q% $RCSfile$ v$Revision$ %H% %T%"

#include <OXW/EventHandler.h>
#include <CLASS/checks.h>

int OXWEventHandler::Find( OXWEventInfo&, OXWEqualOperator )
{
  return 0;
}

class GENERIC
{
  virtual void pure( OXWEvent& ) = 0; // This is to allow usage of virtual
	                              // catchers, indeed never used.
};

typedef void (GENERIC::*GENERIC_PMF)( OXWEvent& );

inline
long OXWEventDispatch( GENERIC& generic, GENERIC_PMF pmf, OXWEvent& event )
{
  (generic.*pmf)( event );
  return 0;
}

long OXWEventHandler::Dispatch( OXWEventInfo& eventInfo, OXWEvent& event )
{
  PRECONDITION(eventInfo.Entry);
  return OXWEventDispatch( *(eventInfo.Object), eventInfo.Entry->Pmf, event );
}

int OXWEventHandler::SearchEntries( OXWGenericTableEntry *entries, OXWEventInfo& eventInfo, OXWEqualOperator equal )
{
  if ( equal ) {
    while ( entries->Pmf != 0 ) {
      if ( equal( *entries, eventInfo ) ) {
        eventInfo.Entry = entries;
        return 1;
      }
      ++entries;
    }
  } else {
    while ( entries->Pmf != 0 ) {
      if ( entries->Msg == eventInfo.Msg && entries->Id == eventInfo.Id ) {
        eventInfo.Entry = entries;
        return 1;
      }
      ++entries;
    }
  }
  return 0;
}
