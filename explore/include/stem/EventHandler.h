#ident "%Z%%Q%$RCSfile$ ($Revision$): %H% %T%" // -*- C++ -*-

#ifndef __OXW_EventHandler_h
#define __OXW_EventHandler_h

#ifndef __OXW_OXWdefs_h
#include <OXW/OXWdefs.h>
#endif

#ifndef __OXW_Event_h
#include <OXW/Event.h>
#endif

#ifndef __OXW_Dispatch_h
#include <OXW/Dispatch.h>
#endif

#ifndef LIST_H
#include <stl/list.h>
#endif

#ifndef ALGO_H
#include <stl/algo.h>
#endif

#ifndef STRSTREAM_H
#include <strstream.h>
#endif

// ******************************************************* OXWEventsTableEntry

template <class T >
class OXWEventsTableEntry
{
  friend class OXWEventsTableEntry<T>;

  public:
    typedef void (T::*PMF)( OXWEvent& );

    OXWEventsTableEntry( message_type msg, PMF pmf, AnyDPMF dpmf,
			 const char *cls, const char *pmf_name ) :
	Msg( msg ),
	Pmf( pmf ),
	Pmf_name( pmf_name ),
        class_name( cls ),
	dispatch( dpmf )
      { }
    OXWEventsTableEntry() :
	Msg( 0 ),
	Pmf( 0 ),
	Pmf_name( "??" ),
        class_name( "??" ),
        dispatch( 0 )
      { }

    OXWEventsTableEntry( message_type msg ) :
	Msg( msg ),
	Pmf( 0 ),
	Pmf_name( "??" ),
        class_name( "??" ),
        dispatch( 0 )
      { }

    bool operator==( const OXWEventsTableEntry<T>& probe ) const
      { return probe.Msg == Msg; }
    bool Dispatch( const T *generic, OXWEvent& event ) const
      {
	dispatch( (GENERIC *)generic, AnyPMF(Pmf), event );
	// (generic.*Pmf)( event );
	return 1;
      }
    void Out( ostrstream& out ) const
      { out << "Message: " << Msg << "\t" << class_name << "::" << Pmf_name; }

  private:
    message_type Msg;
    PMF Pmf;
    const char *Pmf_name;
    const char *class_name;
    const AnyDPMF dispatch;
};

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
    bool Dispatch( const T *generic, OXWEvent& event ) const;
    state_type State() const
      { return theState; }
    void Out( ostrstream& ) const;

  private:
    state_type theState;
    list<table_entry> table;
};

// ************************************************************* OXWStateTable

template <class T>
class OXWStateTable
{
  friend class OXWStateTable<T>;

  public:
    typedef OXWEventsTable<T> table_entry;
    void push( state_type, OXWEventsTableEntry<T>& );
    bool Dispatch( const T *generic, state_type, OXWEvent& event );
    void swap( OXWStateTable& x )
      { table.swap( x.table ); }
    void Out( ostrstream& ) const;

  private:
    list<table_entry> table;
};

typedef OXWEventsTableEntry<GENERIC> GENERIC_EvTblEntry;
typedef OXWStateTable<GENERIC> GENERIC_StatesTbl;

// macros to add method isA(), that return class name.

#define NAME_IT                                 \
  public:					\
    virtual const char *isA() const		\
      { return class_name; }			\
  private:					\
    static const char *class_name

#define DEFINE_NAME_IT( cls )                   \
const char *cls::class_name = #cls

// *********************************************************** OXWEventHandler

class OXWEventHandler
{
  public:
    OXWEventHandler();
    void State( state_type state )
      { PushState( state ); }
    void PushState( state_type state )
      {
	RemoveState( state );
	theHistory.push_back( state );
      }
    state_type State() const
      { return theHistory.back(); }
    void PopState();
    void PopState( state_type );
    void PushTState( state_type state )
      {
	theHistory.push_back( ST_TERMINAL );
	theHistory.push_back( state );
      }
    void RemoveState( state_type );
    bool isState( state_type ) const;
    bool Dispatch( OXWEvent& );
    virtual const char *Trace() const;

  protected:
    void TraceStack( ostrstream& ) const;
    virtual bool __Dispatch( state_type, OXWEvent& );
    static int RespTblConfigure();

    list<state_type> theHistory;
    static GENERIC_StatesTbl theStatesTable;
    static ostrstream Out;
    NAME_IT;

  private:
    list<state_type>::iterator __find( state_type );
};

// ***************************************************************************

// Macro for response table declaration:
// class XX :
//    public YY
// {
//   ...
//   DECLARE_RESPONSE_TABLE( XX, YY );
// };
//
// The body of response table is defined by macro DEFINE_RESPONSE_TABLE
// (see below).

#define DECLARE_RESPONSE_TABLE( cls, pcls )	\
  NAME_IT;                                      \
  public:                                       \
    virtual const char *Trace() const;          \
  private:					\
    typedef cls ThisCls;			\
    typedef pcls ParentThisCls;			\
  protected:					\
    static int RespTblConfigure();              \
    virtual bool __Dispatch( state_type, OXWEvent& ); \
    static GENERIC_StatesTbl theStatesTable;    \
    static int ini_flag

// Macro for specification of response table body beginning:
// DEFINE_RESPONSE_TABLE( XX )
//   RESPONSE_TABLE_ENTRY( ST_NRM, XW_EXPOSE, OXWEvExpose, XEventDispatch );
// END_RESPONSE_TABLE

#define DEFINE_RESPONSE_TABLE( cls )		\
DEFINE_NAME_IT( cls );                          \
GENERIC_StatesTbl cls::theStatesTable;          \
int cls::ini_flag = cls::RespTblConfigure();    \
                                                \
bool cls::__Dispatch( state_type state, OXWEvent& __event__ ) \
{						\
  return 					\
      theStatesTable.Dispatch( (GENERIC *)this, state, __event__ ); \
}						\
                                                \
const char *cls::Trace() const                  \
{                                               \
  Out.seekp( 0, ostream::beg );                 \
  TraceStack( Out );                            \
  ThisCls::theStatesTable.Out( Out );           \
  Out << ends;                                  \
                                                \
  return Out.str();                             \
}                                               \
						\
int cls::RespTblConfigure()                     \
{			                        \
  if ( ini_flag == 0) {                         \
    ParentThisCls::RespTblConfigure();          \
    theStatesTable = ParentThisCls::theStatesTable;

// Macro for specification of response table entry:
// RESPONSE_TABLE_ENTRY( ST_NRM, XW_EXPOSE, OXWEvExpose, XEventDispatch );
//                       ~~~~~~  ~~~~~~~~~  ~~~~~~~~~~~  ~~~~~~~~~~~~~~
//                       State     Event      Catcher      Dispatcher

#define RESPONSE_TABLE_ENTRY( state, msg, catcher, dispatch ) \
    theStatesTable.push( state, *((GENERIC_EvTblEntry *) \
	  &OXWEventsTableEntry<ThisCls>( msg,   \
	  (OXWEventsTableEntry<ThisCls>::PMF)(&ThisCls::catcher), \
	  AnyDPMF(dispatch),                    \
          class_name, #catcher )) )

// Macro for specification of response table end:

#define END_RESPONSE_TABLE			\
  }                                             \
  return 1;                                     \
}

#ifndef __TEMPLATE_DB__
#include "EventHandler.cc"
#endif

#endif          // __OXW_EventHandler_h
