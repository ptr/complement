#ident "%Z%%Q% $RCSfile$ v$Revision$ %H% %T%" // -*- C++ -*-

#ifndef __OXW_EventHandler_h
#define __OXW_EventHandler_h

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif

#ifndef __OXW_OXWdefs_h
#include <OXW/OXWdefs.h>
#endif

#ifndef __OXW_Event_h
#include <OXW/Event.h>
#endif

#ifndef LIST_H
#include <list.h>
#endif

#ifndef ALGO_H
#include <algo.h>
#endif

// ******************************************************* OXWEventsTableEntry

template <class T>
class OXWEventsTableEntry
{
  friend class OXWEventsTableEntry<T>;

  public:
    typedef void (T::*PMF)( OXWEvent& );

    OXWEventsTableEntry( message_type msg, PMF pmf ) :
	Msg( msg ),
	Pmf( pmf )
      { }
    OXWEventsTableEntry() :
	Msg( 0 ),
	Pmf( 0 )
      { }
    bool operator==( const OXWEventsTableEntry<T>& probe ) const
      { return probe.Msg == Msg; }
    bool Dispatch( const T& generic, OXWEvent& event ) const
      {
	(generic.*Pmf)( event );
	return 1;
      }
    char *PrintContents() const;

  private:
    message_type Msg;
    PMF Pmf;
};

template <class T>
char *OXWEventsTableEntry<T>::PrintContents() const
{
  static char buffer[128];
  ostrstream buf( buffer, 128 );

  buf << "Message: " << Msg << ends;
  return buffer;
}

// ************************************************************ OXWEventsTable

template <class T>
class OXWEventsTable
{
  friend class OXWEventsTable<T>;

  public:
    typedef const OXWEventsTableEntry<T> const_table_entry;
    typedef OXWEventsTableEntry<T> table_entry;
    OXWEventsTable( state_type state ) :
	theState( state )
      { }
    OXWEventsTable() :
	theState( ST_NULL )
      { }
    void push( const OXWEventsTableEntry<T>& entry );
    bool operator==( const_table_entry& probe ) const
      { return probe.theState == theState; }
    bool Dispatch( const T& generic, OXWEvent& event ) const;
    state_type State() const
      { return theState; }
    char *PrintContents() const;

  private:
    state_type theState;
    list<table_entry> table;
};

template <class T>
void OXWEventsTable<T>::push( const OXWEventsTableEntry<T>& entry )
{
  list<table_entry>::iterator i = find( table.begin(), table.end(), entry );
  if ( i != table.end() ) {
    table.erase( i ); // only one entry allowed
  }
  table.push_back( entry );
}

template <class T>
bool OXWEventsTable<T>::Dispatch( const T& generic, OXWEvent& event ) const
{
  list<table_entry>::const_iterator i = find( table.begin(), table.end(),
					table_entry( event.Message, 0 ) );
  return i != table.end() ? (*i).Dispatch( generic, event ) : 0;
}

template <class T>
char *OXWEventsTable<T>::PrintContents() const
{
  static char buffer[1024];
  list<table_entry>::const_iterator i = table.begin();
  ostrstream buf( buffer, 1024 );

  buf << "State " << theState << "\n";
  while ( i != table.end() ) {
    buf << "\t" << (*i).PrintContents() << "\n";
    ++i;
  }
  buf << ends;
  return buffer;
}

// ************************************************************* OXWStateTable

template <class T>
class OXWStateTable
{
  friend class OXWStateTable<T>;

  public:
    typedef OXWEventsTable<T> table_entry;
    void push( state_type, OXWEventsTableEntry<T>& );
    bool Dispatch( const T& generic, state_type, OXWEvent& event );
    void swap( OXWStateTable& x )
      { table.swap( x.table ); }
    char *PrintContents() const;

  private:
    list<table_entry> table;
};

template <class T>
void OXWStateTable<T>::push( state_type state, OXWEventsTableEntry<T>& entry )
{
  list<table_entry>::iterator i = table.begin();

  while ( i != table.end() ) {
    if ( (*i).State() == state ) {
      (*i).push( entry );
      return;
    }
    ++i;
  }
  table_entry new_state( state );
  new_state.push( entry );
  table.push_back( new_state );
}

template <class T>
bool OXWStateTable<T>::Dispatch( const T& generic, state_type state,
				 OXWEvent& event )
{
  list<table_entry>::iterator i = table.begin();

  while ( i != table.end() ) {
    if ( (*i).State() == state ) {
      return (*i).Dispatch( generic, event );
    }
    ++i;
  }
  return 0;
}

template <class T>
char *OXWStateTable<T>::PrintContents() const
{
  static char buffer[1024];
  list<table_entry>::const_iterator i = table.begin();
  ostrstream buf( buffer, 1024 );

  while ( i != table.end() ) {
    buf << (*i).PrintContents();
    ++i;
  }
  buf << ends;
  return buffer;
}

// ******************************************************************* GENERIC

class GENERIC
{
  virtual void pure( OXWEvent& ) = 0; // This is to allow usage of virtual
	                              // catchers, indeed never used.
};

typedef OXWEventsTableEntry<GENERIC> GENERIC_EvTblEntry;
typedef OXWStateTable<GENERIC> GENERIC_StatesTbl;

// *********************************************************** OXWEventHandler

class OXWEventHandler
{
  public:
    OXWEventHandler();
    void State( state_type state )
      { theHistory.push_back( state ); }
    state_type State() const
      { return theHistory.back(); }
    void PopState();
    void TerminalState( state_type state )
      {
	theHistory.push_back( ST_TERMINAL );
	theHistory.push_back( state );
      }
    bool Dispatch( OXWEvent& );
    virtual const char *isA() const
      { return "OXWEventHandler"; }
    
  protected:
    void OXWEvRTInit( OXWEvent& );
    virtual void OXWRTInit();
    virtual bool __Dispatch( state_type, OXWEvent& );
    static GENERIC_StatesTbl RTConfigure();

    list<state_type> theHistory;
    static GENERIC_StatesTbl theStatesTable;

  private:
    static int ini_flag;
};

// ***************************************************************************

#define DECLARE_RESPONSE_TABLE( cls, pcls )	\
  private:					\
    typedef cls ThisCls;			\
    typedef pcls ParentThisCls;			\
  protected:					\
    virtual void OXWRTInit();                   \
    virtual bool __Dispatch( state_type, OXWEvent& ); \
    static GENERIC_StatesTbl theStatesTable


#define DEFINE_RESPONSE_TABLE( cls )		\
GENERIC_StatesTbl cls::theStatesTable;          \
                                                \
bool cls::__Dispatch( state_type state, OXWEvent& __event__ ) \
{						\
  TRACE( "Dispatch " << isA() << "\n" << theStatesTable.PrintContents() ); \
  return 					\
      theStatesTable.Dispatch( *((GENERIC *)this), state, __event__ ); \
}						\
						\
void cls::OXWRTInit()                           \
{						\
  ParentThisCls::OXWRTInit();                   \
  theStatesTable.swap( ParentThisCls::theStatesTable );

#define END_RESPONSE_TABLE			\
}

#define RESPONSE_TABLE_ENTRY( state, msg, catcher ) \
  theStatesTable.push( state, *((GENERIC_EvTblEntry *) \
	  &OXWEventsTableEntry< ThisCls >( msg, &ThisCls::catcher )) )

#endif          // __OXW_EventHandler_h
