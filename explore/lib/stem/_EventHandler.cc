// -*- C++ -*- Time-stamp: <99/09/08 14:36:38 ptr>
#ident "$SunId$ %Q%"

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#ifdef WIN32
#  ifdef _DLL
#    define __EDS_DLL __declspec( dllexport )
#  else
#    define __EDS_DLL
#  endif
#else
#  define __EDS_DLL
#endif

#include <EventHandler.h>
#include <EvManager.h>
#include <Names.h>

#if defined(__TRACE) || defined(__WARN)

// DIAG_DEFINE_GROUP(EventHandler,0,4);
// DIAG_DEFINE_GROUP(EventDispatch,0,3);

#endif // __TRACE || __WARN

namespace EDS {

char *Init_buf[32];
EvManager *EventHandler::_mgr = 0;
Names *_ns = 0;
const char *_ns_name = "ns";

int EventHandler::Init::_count = 0;

EventHandler::Init::Init()
{
  if ( _count++ == 0 ) {
    _mgr = new EvManager();
    _ns = new Names( nsaddr, _ns_name );
  }
}

EventHandler::Init::~Init()
{
  if ( --_count == 0 ) {
    delete _ns;
    delete _mgr;
  }
}

__EDS_DLL
const string& EventHandler::who_is( addr_type k ) const
{
  return _mgr->who_is( k );
}

__EDS_DLL
unsigned EventHandler::sid( key_type k ) const
{
  return _mgr->sid( k );
}

__EDS_DLL
void EventHandler::Send( const Event& e )
{
  e.src( _id );
  _mgr->Send( e );
}

__EDS_DLL
void EventHandler::Send( const EventVoid& e )
{
  e.src( _id );
  _mgr->Send( EDS::Event_convert<void>()( e ) );
}

__EDS_DLL
void EventHandler::PushState( state_type state )
{
  RemoveState( state );
  theHistory.push_front( state );
}

__EDS_DLL
state_type EventHandler::State() const
{
  return theHistory.front();
}

__EDS_DLL
void EventHandler::PushTState( state_type state )
{
  theHistory.push_front( ST_TERMINAL );
  theHistory.push_front( state );
}

__EDS_DLL
void EventHandler::PopState()
{
  theHistory.pop_front();
  while ( theHistory.front() == ST_TERMINAL ) {
    theHistory.pop_front();
  }
}

__EDS_DLL
void EventHandler::PopState( state_type state )
{
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( theHistory.begin(), ++hst_i );
  }
}

__EDS_DLL
void EventHandler::RemoveState( state_type state )
{
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( hst_i );
  }
}

__EDS_DLL
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

__EDS_DLL
EventHandler::EventHandler()// :
//    theHistory( *(new HistoryContainer()) )
{
  new( Init_buf ) Init();
  State( ST_NULL );
  _id = _mgr->Subscribe( this );
}

__EDS_DLL
EventHandler::EventHandler( const char *info )// :
//    theHistory( *(new HistoryContainer()) )
{
  new( Init_buf ) Init();
  State( ST_NULL );
  _id = _mgr->Subscribe( this, info );
}

__EDS_DLL
EventHandler::EventHandler( addr_type id, const char *info )// :
//    theHistory( *(new HistoryContainer()) )
{
  new( Init_buf ) Init();
  State( ST_NULL );
  _id = _mgr->SubscribeID( id, this, info );
  __stl_assert( _id != -1 ); // already registered, or id has Event::extbit
//  if ( _id == -1 ) {
//    _mgr->Subscribe( this, "" );
//  }
}

__EDS_DLL
EventHandler::~EventHandler()
{
//  delete &theHistory;
   _mgr->Unsubscribe( _id );
  ((Init *)Init_buf)->~Init();
}

__EDS_DLL
void EventHandler::TraceStack( ostream& out ) const
{
  const HistoryContainer& hst = theHistory;
  HistoryContainer::const_iterator hst_i = hst.begin();
  while ( hst_i != hst.end() ) {
    out << "[" << *hst_i++ << "]";
  }
  out << endl;
}

} // namespace EDS
