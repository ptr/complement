// -*- C++ -*- Time-stamp: <96/03/06 14:41:30 ptr>
#ifndef __OXW_EventHandler_h
#define __OXW_EventHandler_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifndef __OXW_OXWdefs_h
#include <OXW/OXWdefs.h>
#endif

#ifndef __OXW_Event_h
#include <OXW/Event.h>
#endif

#ifndef ALGO_H
#include <stl/algo.h>
#endif

#ifndef PAIR_H
#include <stl/pair.h>
#endif

#ifndef PROJECTN_H
#include <stl/projectn.h>
#endif

#ifndef VECTOR_H
#include <stl/vector.h>
#endif

#ifndef LIST_H
#include <stl/list.h>
#endif

#ifndef STRSTREAM_H
#include <strstream.h>
#endif

class OXWEventHandler;

typedef list<state_type> HistoryContainer;
typedef HistoryContainer::iterator h_iterator;
typedef HistoryContainer::const_iterator const_h_iterator;


class GENERIC
{
  virtual void foo( OXWEvent& ) = 0; // This is to allow usage of virtual
	                             // catchers, indeed never used.
  public:
    typedef void (GENERIC::*PMF)();
    typedef void (*DPMF)();
};


template <class T, class U>
struct convert
{
    const U& operator ()( const T& x ) const
      { return (const U&)(x); }
};

template <class T, class Arg>
struct __member_function
{
    typedef void (T::*pmf_type)( Arg );
    typedef Arg argument_type;
    typedef Arg& reference_argument_type;
    typedef T   class_type;
    typedef T * pointer_class_type;
    typedef T&  reference_class_type;
    typedef const T * const_pointer_class_type;
    typedef const T&  const_reference_class_type;
    typedef void (*dpmf_type)( const_pointer_class_type, pmf_type,
			       argument_type arg );
};

template <class T >
struct __member_function_void
{
    typedef void (T::*pmf_type)();
    typedef T   class_type;
    typedef T * pointer_class_type;
    typedef T&  reference_class_type;
    typedef const T * const_pointer_class_type;
    typedef const T&  const_reference_class_type;
    typedef void (*dpmf_type)( const_pointer_class_type, pmf_type,
			       argument_type arg );
};

template <class PMF>
struct __dispatcher
{
    static void dispatch( PMF::const_pointer_class_type c, PMF::pmf_type pmf,
	                  PMF::reference_argument_type arg )
      {
	(c->*pmf)( arg );
      }
};

template <class PMF, class Arg >
struct __dispatcher_void
{
    static void dispatch( PMF::const_pointer_class_type c, PMF::pmf_type pmf,
	                  Arg& )
      {
	(c->*pmf)();
      }
};

template <class PMF, class Arg >
struct __dispatcher_convert
{
    static void dispatch( PMF::const_pointer_class_type c, PMF::pmf_type pmf,
	                  Arg& arg )
      {
	(c->*pmf)( convert<Arg,PMF::argument_type>()(arg) );
      }
};

struct __AnyPMFentry
{
    GENERIC::PMF  pmf;
    GENERIC::DPMF dpmf;
    const char *pmf_name;
};

template <class T>
struct __PMFentry
{
    typedef void (T::*PMF)();

    PMF  pmf;
    GENERIC::DPMF dpmf;
    const char *pmf_name;
};

template <class T>
struct __DeclareAnyPMF
{
    state_type    st;
    message_type  msg;
    __PMFentry<T> func;
};

template <class Key1, class Key2, class Value>
class __EvTable
{
  public:
    friend class __EvTable< Key1, Key2, Value>;
    typedef vector<pair<Key2,Value> > Container2;
    typedef vector<pair<Key1,Container2> > Container1;
    typedef Container1::iterator iterator1;
    typedef Container2::iterator iterator2;
    typedef Container1::const_iterator const_iterator1;
    typedef Container2::const_iterator const_iterator2;

    bool get( Key1, Key2, Value& ) const;
    iterator1 get( Key1 key1 )
      {
	iterator1 i1 = storage.begin();
	while ( i1 != storage.end() && key1st(*i1) != key1 ) {
	  ++i1;
	}
	return i1;
      }
    const_iterator1 get( Key1 key1 ) const
      {
	const_iterator1 i1 = storage.begin();
	while ( i1 != storage.end() && key1st(*i1) != key1 ) {
	  ++i1;
	}
	return i1;
      }
    iterator2 get( iterator1 i1, Key2 key2 )
      {
	Container2& c2 = (*i1).second;
	iterator2 i2 = c2.begin();
	while ( i2 != c2.end() && key2nd(*i2) != key2 ) {
	  ++i2;
	}
	return i2;
      }
    const_iterator2 get( const_iterator1 i1, Key2 key2 ) const
      {
	const Container2& c2 = (*i1).second;
	const_iterator2 i2 = c2.begin();
	while ( i2 != c2.end() && key2nd(*i2) != key2 ) {
	  ++i2;
	}
	return i2;
      }
    bool get( const_iterator1 i1, Key2 key2, Value& value ) const
      {
	const_iterator2 i2 = get( i1, key2 );
	if ( i2 == (*i1).second.end() ) {
	  return false;
	}
	value = (*i2).second;

	return true;
      }

    iterator1 begin()
      { return storage.begin(); }
    iterator1 end()
      { return storage.end(); }
    const_iterator1 begin() const
      { return storage.begin(); }
    const_iterator1 end() const
      { return storage.end(); }
    iterator2 begin( iterator1 i )
      { return (*i).second.begin(); }
    iterator2 end( iterator1 i )
      { return (*i).second.end(); }
    const_iterator2 begin( const_iterator1 i ) const
      { return (*i).second.begin(); }
    const_iterator2 end( const_iterator1 i ) const
      { return (*i).second.end(); }
    bool empty() const
      { return storage.empty(); }

    void append( const __EvTable<Key1,Key2,Value>& x )
      {
	if ( x.empty() ) {
	  return;
	}
	const_iterator1 i1 = x.storage.begin();
	while ( i1 != x.storage.end() ) {
	  Key1 key1 = key1st(*i1);
	  const_iterator2 i2 = (*i1).second.begin();
	  while ( i2 != (*i1).second.end() ) {
	    Key2 key2 = key2nd(*i2);
	    append( key1, key2, (*i2).second );
	    ++i2;
	  }
	  ++i1;
	}
      }
    void append( Key1 key1, Key2 key2, const Value& value )
      {
	iterator1 i1 = get( key1 );
	if ( i1 == storage.end() ) {
	  pair<Key2,Value> p2( key2, value );
	  Container2 c2;
	  c2.push_back( p2 );
	  pair<Key1,Container2> p1( key1, c2 );
	  storage.push_back( p1 );
	  return;
	}
	iterator2 i2 = get( i1, key2 );
	if ( i2 == (*i1).second.end() ) {
	  pair<Key2,Value> p2( key2, value );
	  (*i1).second.push_back( p2 );
	  return;
	}
	(*i2).second = value; // only last value stored.
      }

    select1st<pair<Key1,Container2>, Key1> key1st;
    select1st<pair<Key2,Value>, Key2> key2nd;
 
  protected:
   Container1 storage;
};

template <class Key1, class Key2, class Value>
bool __EvTable<Key1,Key2,Value>::get( Key1 key1, Key2 key2, Value& value ) const
{
  const_iterator1 i1 = get( key1 );
  if ( i1 == storage.end() ) {
    return false;
  }
  const_iterator2 i2 = get( i1, key2 );
  if ( i2 == (*i1).second.end() ) {
    return false;
  }
  value = (*i2).second;

  return true;
}

template <class T, class InputIterator >
class __EvHandler
{
  public:
    typedef __EvTable<message_type,state_type,__AnyPMFentry *> table_type;

    __EvHandler( const char *, __DeclareAnyPMF<T> * );

    bool Dispatch( T *, InputIterator, InputIterator, OXWEvent& event );
    bool DispatchStub( T *, InputIterator, InputIterator, OXWEvent& event );
    bool DispatchTrace( InputIterator first, InputIterator last,
			OXWEvent& event, ostrstream& out )
      {
	if ( first == last ) {
	  out << "\n\tStates stack empty?";
	  return false;
	}
	message_type msg = event.GetMessage();
	__AnyPMFentry *entry;
	while ( first != last && !table.get( msg, *first, entry ) ) {
	  ++first;
	}
	if ( first == last ) {
	  out << "\tCatcher not found for message 0x" << hex << msg << dec;
	  return false;
	}
	out << "\tMessage 0x" << hex << msg << dec << " catcher "
	    << entry->pmf_name << " (state " << *first << ")";
	return true;
      }
    void Out( ostrstream& ) const;

    convert<GENERIC::PMF, __member_function<T,OXWEvent>::pmf_type>  pmf;
    convert<GENERIC::DPMF,__member_function<T,OXWEvent>::dpmf_type> dpmf;

    table_type table;
    const char *class_name;
};

template <class T, class InputIterator >
__EvHandler<T, InputIterator>::__EvHandler( const char *nm,
					    __DeclareAnyPMF<T> *e )
{
  class_name = nm;
  const table_type& p = T::ParentThisCls::get_ev_table();
  table.append( p );
  while ( e->func.dpmf != 0 ) {
    table.append( e->msg, e->st, (__AnyPMFentry *)&e->func );
    ++e;
  }
}

template <class T, class InputIterator>
bool __EvHandler<T, InputIterator>::Dispatch( T *c, InputIterator first,
				       InputIterator last, OXWEvent& event )
{
  if ( first == last ) {
    return false;
  }
  message_type msg = event.GetMessage();
  __AnyPMFentry *entry;
  table_type::const_iterator1 i1 = table.get( msg );
  if ( i1 == table.end() ) {
    return false;
  }  
  while ( first != last && !table.get( i1, *first, entry ) ) {
    ++first;
  }
  if ( first == last ) {
    return false;
  }
  (*dpmf(entry->dpmf))( c, pmf(entry->pmf), event );

  return true;
}

template <class T, class InputIterator>
bool __EvHandler<T, InputIterator>::DispatchStub( T *, InputIterator first,
				       InputIterator last, OXWEvent& event )
{
  if ( first == last ) {
    return false;
  }
  message_type msg = event.GetMessage();
  __AnyPMFentry *entry;
  table_type::const_iterator1 i1 = table.get( msg );
  if ( i1 == table.end() ) {
    return false;
  }  
  while ( first != last && !table.get( i1, *first, entry ) ) {
    ++first;
  }
  if ( first == last ) {
    return false;
  }
  return true;
}

template <class T, class InputIterator>
void __EvHandler<T, InputIterator>::Out( ostrstream& out ) const
{
  if ( table.empty() ) {
    return;
  }
  __AnyPMFentry *entry;
  table_type::const_iterator1 i1 = table.begin();
  while ( i1 != table.end() ) {
    message_type key1 = table.key1st(*i1);
    table_type::const_iterator2 i2 = table.begin( i1 );
    out << "\tMessage: " << hex << key1 << dec << endl;
    while ( i2 != table.end( i1 ) ) {
      state_type key2 = table.key2nd(*i2);
      table.get( key1, key2, entry );
      out << "\t\tState " << key2 << ":\t" << entry->pmf_name << "\n";
      ++i2;
    }
    ++i1;
  }
}

class __EvHandler<OXWEventHandler,h_iterator>
{
  public:
    typedef __EvTable<message_type,state_type,__AnyPMFentry *> table_type;

    __EvHandler( const char *nm, __DeclareAnyPMF<OXWEventHandler> * )
      {	class_name = nm; }

    bool Dispatch( OXWEventHandler *, h_iterator, h_iterator, OXWEvent& )
      { return false; }
    bool DispatchStub( OXWEventHandler *, h_iterator, h_iterator, OXWEvent& )
      { return false; }
    bool DispatchTrace( h_iterator, h_iterator, OXWEvent&, ostrstream& )
      { return false; }
    void Out( ostrstream& ) const
      { }

    table_type table;
    const char *class_name;
};

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
    typedef __EvHandler<OXWEventHandler,h_iterator> evtable_type;
    typedef evtable_type::table_type table_type;

  protected:
    HistoryContainer& theHistory;
    static evtable_type theEventsTable;
    static __DeclareAnyPMF<OXWEventHandler> theDeclEventsTable[];
    NAME_IT;

  public:
    OXWEventHandler();
    ~OXWEventHandler();

    void State( state_type state )
      { PushState( state ); }
    void PushState( state_type state );
    state_type State() const;
    void PopState();
    void PopState( state_type );
    void PushTState( state_type state );
    void RemoveState( state_type );
    bool isState( state_type ) const;
    virtual bool Dispatch( OXWEvent& );
    virtual bool DispatchStub( OXWEvent& );
    virtual void DispatchTrace( OXWEvent&, ostrstream&  );
    virtual void Trace( ostrstream& ) const;
    void TraceStack( ostrstream& ) const;
    static const table_type& get_ev_table()
      { return theEventsTable.table; }

  private:
    h_iterator __find( state_type );
    const_h_iterator __find( state_type ) const;
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
    virtual void Trace( ostrstream& ) const;    \
    virtual void DispatchTrace( OXWEvent&, ostrstream& );\
    typedef __EvHandler<cls,h_iterator> evtable_type;\
    typedef cls ThisCls;			\
    typedef pcls ParentThisCls;			\
    static const table_type& get_ev_table()     \
      { return theEventsTable.table; }          \
  protected:					\
    virtual bool Dispatch( OXWEvent& );         \
    virtual bool DispatchStub( OXWEvent& );     \
    static evtable_type theEventsTable;         \
    static __DeclareAnyPMF<cls> theDeclEventsTable[]

// Macro for specification of response table body beginning:
// DEFINE_RESPONSE_TABLE( XX )
//   RESPONSE_TABLE_ENTRY( ST_NRM, XW_EXPOSE, OXWEvExpose, XEventDispatch );
// END_RESPONSE_TABLE

#define DEFINE_RESPONSE_TABLE( cls )		\
DEFINE_NAME_IT( cls );                          \
cls::evtable_type cls::theEventsTable( #cls, theDeclEventsTable ); \
                                                \
bool cls::Dispatch( OXWEvent& __event__ )       \
{						\
  return theEventsTable.Dispatch( this, theHistory.begin(), \
				  theHistory.end(), __event__ ); \
}						\
                                                \
bool cls::DispatchStub( OXWEvent& __event__ )   \
{						\
  return theEventsTable.DispatchStub( this, theHistory.begin(), \
				      theHistory.end(), __event__ ); \
}						\
                                                \
void cls::DispatchTrace( OXWEvent& __event__, ostrstream& out )  \
{						\
  theEventsTable.DispatchTrace( theHistory.begin(), \
	 		        theHistory.end(), __event__, out ); \
}						\
                                                \
void cls::Trace( ostrstream& out ) const        \
{                                               \
  theEventsTable.Out( out );                    \
}                                               \
__DeclareAnyPMF<cls> cls::theDeclEventsTable[] = {

// Macro for specification of response table entry:
// RESPONSE_TABLE_ENTRY( ST_NRM, XW_EXPOSE, OXWEvExpose, XEventDispatch );
//                       ~~~~~~  ~~~~~~~~~  ~~~~~~~~~~~  ~~~~~~~~~~~~~~
//                       State     Event      Catcher      Dispatcher

#define RESPONSE_TABLE_ENTRY( state, msg, catcher, dispatch ) \
  { state, msg,                                               \
    {(__PMFentry<ThisCls>::PMF)catcher, (GENERIC::DPMF)dispatch, #catcher }},

// Macro for specification of response table end:

#define END_RESPONSE_TABLE			\
  { 0, 0, {0, 0, "End of table" } }             \
};

#ifndef __TRACE

#define TRACE_EH(handler)           ((void)0)
#define TRACE_ED(handler,e)         ((void)0)

#endif // __TRACE

#ifndef __WARN

#define WARN_EH(c,handler)          ((void)0)
#define WARN_ED(c,handler,e)        ((void)0)

#endif // __WARN

#if defined(__TRACE) || defined(__WARN)

DIAG_DECLARE_GROUP(EventHandler);
DIAG_DECLARE_GROUP(EventDispatch);

#ifdef __TRACE

#define TRACE_EH(handler)           TRACEX_EH(EventHandler,4,(handler))
#define TRACEX_EH(g,l,handler)                                       \
  CLDiagBase::Out.seekp(0,ostream::beg);                             \
  CLDiagBase::Out << "\n\t" << handler.isA() << " -> ";              \
  handler.TraceStack( CLDiagBase::Out );                             \
  handler.Trace( CLDiagBase::Out );                                  \
  CLDiagBase::Out << ends;                                           \
  CLDiagGroup##g::Trace(l,CLDiagBase::Out.str(),__FILE__,__LINE__);

#define TRACE_ED(handler,e)         TRACEX_ED(EventDispatch,3,(handler),e)
#define TRACEX_ED(g,l,handler,e)                                     \
  CLDiagBase::Out.seekp(0,ostream::beg);                             \
  CLDiagBase::Out << "\n\t" << handler.isA() << " -> ";              \
  handler.TraceStack( CLDiagBase::Out );                             \
  handler.DispatchTrace( e, CLDiagBase::Out );                       \
  CLDiagBase::Out << ends;                                           \
  CLDiagGroup##g::Trace(l,CLDiagBase::Out.str(),__FILE__,__LINE__);

#endif // __TRACE

#ifdef __WARN

#define WARN_EH(c,handler)          WARNX_EH(EventHandler,c,4,(handler))
#define WARNX_EH(g,c,l,handler)                                     \
  if(c){                                                            \
    CLDiagBase::Out.seekp(0,ostream::beg);                          \
    CLDiagBase::Out << "\n\t" << handler.isA() << " -> ";           \
    handler.TraceStack( CLDiagBase::Out );                          \
    handler.Trace( CLDiagBase::Out );                               \
    CLDiagBase::Out << ends;                                        \
    CLDiagGroup##g::Warn(l,CLDiagBase::Out.str(),__FILE__,__LINE__);\
  }

#define WARN_ED(c,handler,e)        WARNX_ED(EventDispatch,c,3,(handler),e)
#define WARNX_ED(g,c,l,handler,e)                                   \
  if(c) {                                                           \
    CLDiagBase::Out.seekp(0,ostream::beg);                          \
    CLDiagBase::Out << "\n\t" << handler.isA() << " -> ";           \
    handler.TraceStack( CLDiagBase::Out );                          \
    handler.DispatchTrace( e, CLDiagBase::Out );                    \
    CLDiagBase::Out << ends;                                        \
    CLDiagGroup##g::Trace(l,CLDiagBase::Out.str(),__FILE__,__LINE__);\
  }

#endif // __WARN
#endif // __TRACE || __WARN

#endif          // __OXW_EventHandler_h
