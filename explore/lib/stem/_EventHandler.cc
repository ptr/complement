#ident "%Z%$RCSfile$ v.$Revision$ %H% %T%"

#include <OXW/EventHandler.h>
#include <CLASSLIB/checks.h>

int ASEventHandler::Find( ASEventInfo&, ASEqualOperator )
{
  return 0;
}

class GENERIC
{
};

long ASEventDispatch(GENERIC& generic, void (GENERIC::*pmf)( ASEvent& ), ASEvent& event )
{
  (generic.*pmf)( event );
  return 0;
}

long ASEventHandler::Dispatch( ASEventInfo& eventInfo, ASEvent& event )
{
  PRECONDITION(eventInfo.Entry);
  return ASEventDispatch( *(eventInfo.Object), eventInfo.Entry->Pmf, event );
}

int ASEventHandler::SearchEntries( ASGenericTableEntry *entries, ASEventInfo& eventInfo, ASEqualOperator equal )
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
