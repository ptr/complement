// -*- C++ -*- Time-stamp: <99/04/07 13:50:32 ptr>
#ifndef __EventHandler_h
#define __EventHandler_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <Event.h>

#include <algorithm>
#include <utility>
#include <vector>
#include <list>
#include <ostream>

#ifdef WIN32
#include <_algorithm> // for select1st
#endif // WIN32

namespace EDS {

using namespace std;

typedef unsigned state_type;

#define ST_NULL     (EDS::state_type)(0)
#define ST_TERMINAL (EDS::state_type)(-1)

#define D1(cls) EDS::__dispatcher<EDS::__member_function<cls,EDS::Event> >
#define D2(cls) EDS::__dispatcher_void<EDS::__member_function_void<cls>,EDS::EventVoid>

#define __D_EV_T(cls,T) EDS::__dispatcher_convert_Event<EDS::__member_function<cls,\
                                      EDS::Event_base< T > >,EDS::Event>
#define __D_T(cls,T) EDS::__dispatcher_convert_Event_extr<EDS::__member_function<cls,T >,\
                                                          EDS::Event>


#if 0
#define D3(cls) __dispatcher_convert<__member_function<cls,\
                                     EDSCallbackObject<GENERIC> >,EDSEventCb>
#endif

#define EV_EDS(state,event,handler) \
   RESPONSE_TABLE_ENTRY(state,event,handler,D1(ThisCls)::dispatch)
#define EV_VOID(state,event,handler) \
   RESPONSE_TABLE_ENTRY(state,event,handler,D2(ThisCls)::dispatch)
#define EV_Event_base_T_(state,event,handler,T) \
   RESPONSE_TABLE_ENTRY(state,event,handler,__D_EV_T(ThisCls,T)::dispatch)
#define EV_T_(state,event,handler,T) \
   RESPONSE_TABLE_ENTRY(state,event,handler,__D_T(ThisCls,T)::dispatch)

// #define EV_CALL(state,event,handler) \
//   RESPONSE_TABLE_ENTRY(state,event,handler,D3(ThisCls)::dispatch)

class EventHandler;
class EvManager;

typedef list<state_type,__STL_DEFAULT_ALLOCATOR(state_type) > HistoryContainer;
typedef HistoryContainer::iterator h_iterator;
typedef HistoryContainer::const_iterator const_h_iterator;


class GENERIC
{
  virtual void foo( Event& ) = 0; // This is to allow usage of virtual
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

template <class T /* EDS::Event_base<X> */ >
struct convert_Event // from transport
{
      T operator ()( const EDS::Event& x ) const
      {
        T tmp;
        if ( x.is_from_foreign() ) {
          tmp.net_unpack( x );
        } else {
          tmp.unpack( x );
        }
        
        return tmp;
      }
};

template <class T>
struct convert_Event_extr // from transport and extract value
{
     T operator ()( const EDS::Event& x ) const
      {
        EDS::Event_base<T> tmp;
        if ( x.is_from_foreign() ) {
          tmp.net_unpack( x );
        } else {
          tmp.unpack( x );
        }
        
        return tmp.value();
      }
};

template <class T>
struct Event_convert // to transport
{
    EDS::Event operator ()( const EDS::Event_base<T>& x ) const
      {
        EDS::Event tmp;
        if ( x.is_to_foreign() ) {
          x.net_pack( tmp );
        } else {
          x.pack( tmp );
        }
        
        return tmp;
      }
};

template <class T, class Arg>
struct __member_function
{
    typedef Arg argument_type;
    typedef Arg& reference_argument_type;
    typedef const Arg& const_reference_argument_type;
    typedef T   class_type;
    typedef T * pointer_class_type;
    typedef T&  reference_class_type;
    typedef const T * const_pointer_class_type;
    typedef const T&  const_reference_class_type;
    typedef void (T::*pmf_type)( const_reference_argument_type );
    typedef void (*dpmf_type)( const_pointer_class_type, pmf_type,
			       const_reference_argument_type arg );
};

template <class T >
struct __member_function_void
{
    typedef T   class_type;
    typedef T * pointer_class_type;
    typedef T&  reference_class_type;
    typedef const T * const_pointer_class_type;
    typedef const T&  const_reference_class_type;
    typedef void (T::*pmf_type)();
    typedef void (*dpmf_type)( const_pointer_class_type, pmf_type,
			       void * );
};

template <class PMF>
struct __dispatcher
{
    static void dispatch( PMF::pointer_class_type c, PMF::pmf_type pmf,
	                  PMF::const_reference_argument_type arg )
      {	(c->*pmf)( arg ); }
};

template <class PMF, class Arg >
struct __dispatcher_void
{
    static void dispatch( PMF::pointer_class_type c, PMF::pmf_type pmf, const Arg& )
      {	(c->*pmf)(); }
};

template <class PMF, class Arg >
struct __dispatcher_convert
{
    static void dispatch( PMF::pointer_class_type c, PMF::pmf_type pmf, const Arg& arg )
      {	(c->*pmf)( convert<Arg,PMF::argument_type>()(arg) ); }
};

template <class PMF, class Arg >
struct __dispatcher_convert_Event
{
    static void dispatch( PMF::pointer_class_type c, PMF::pmf_type pmf, const Arg& arg )
      {	(c->*pmf)( convert_Event<PMF::argument_type>()(arg) ); }
};

template <class PMF, class Arg >
struct __dispatcher_convert_Event_extr
{
    static void dispatch( PMF::pointer_class_type c, PMF::pmf_type pmf, const Arg& arg )
      {	(c->*pmf)( convert_Event_extr<PMF::argument_type>()(arg) ); }
};

struct __AnyPMFentry
{
#ifndef _MSC_VER
    typename EDS::GENERIC::PMF  pmf;
    typename EDS::GENERIC::DPMF dpmf;
#else
    EDS::GENERIC::PMF  pmf;
    EDS::GENERIC::DPMF dpmf;
#endif
    const char *pmf_name;
};

template <class T>
struct __PMFentry
{
    typedef void (T::*PMF)();

    PMF  pmf;
    typename EDS::GENERIC::DPMF dpmf;
    const char *pmf_name;
};

template <class T>
struct __DeclareAnyPMF
{
    state_type    st;
    typename EDS::Event::code_type code;
    typename EDS::__PMFentry<T> func;
};

template <class Key1, class Key2, class Value>
class __EvTable
{
  public:
    typedef std::pair<Key2,Value> pair2_type;
    typedef std::vector<pair2_type,__STL_DEFAULT_ALLOCATOR(pair2_type) > Container2;
    typedef std::pair<Key1,Container2> pair1_type;
    typedef std::vector<pair1_type,__STL_DEFAULT_ALLOCATOR(pair1_type) > Container1;
    typedef Container1::iterator iterator1;
    typedef Container2::iterator iterator2;
    typedef Container1::const_iterator const_iterator1;
    typedef Container2::const_iterator const_iterator2;

    // Renaming get's was done due to VC 5.0 problem:
    // its unhappy with detecting const/nonconst function variant,
    // and with functions overloading (same names, parameters differ)
    // 
    bool get( Key1, Key2, Value& ) const;
    iterator1       get_5( Key1 key1 );
    const_iterator1 get_4( Key1 key1 ) const;
    iterator2       get_3( iterator1 i1, Key2 key2 );
    const_iterator2 get_2( const_iterator1 i1, Key2 key2 ) const;
    bool get_1( const_iterator1 i1, Key2 key2, Value& value ) const;

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

    void append( const __EvTable<Key1,Key2,Value>& x );
    void append( Key1 key1, Key2 key2, const Value& value );

    std::select1st<pair1_type> key1st;
    std::select1st<pair2_type> key2nd;
 
  protected:
   Container1 storage;
};

template <class Key1, class Key2, class Value>
bool __EvTable<Key1,Key2,Value>::get( Key1 key1, Key2 key2, Value& value ) const
{
  const_iterator1 i1 = get_4( key1 );
  if ( i1 == storage.end() ) {
    return false;
  }
  const_iterator2 i2 = get_2( i1, key2 );
  if ( i2 == (*i1).second.end() ) {
    return false;
  }
  value = (*i2).second;

  return true;
}

template <class Key1, class Key2, class Value>
__EvTable<Key1,Key2,Value>::iterator1 __EvTable<Key1,Key2,Value>::get_5( Key1 key1 )
{
  iterator1 i1 = storage.begin();
  while ( i1 != storage.end() && key1st(*i1) != key1 ) {
    ++i1;
  }
  return i1;
}

template <class Key1, class Key2, class Value>
__EvTable<Key1,Key2,Value>::const_iterator1 __EvTable<Key1,Key2,Value>::get_4( Key1 key1 ) const
{
  const_iterator1 i1 = storage.begin();
  while ( i1 != storage.end() && key1st(*i1) != key1 ) {
    ++i1;
  }
  return i1;
}

template <class Key1, class Key2, class Value>
__EvTable<Key1,Key2,Value>::iterator2 __EvTable<Key1,Key2,Value>::get_3( __EvTable<Key1,Key2,Value>::iterator1 i1, Key2 key2 )
{
  Container2& c2 = (*i1).second;
  iterator2 i2 = c2.begin();
  while ( i2 != c2.end() && key2nd(*i2) != key2 ) {
    ++i2;
  }
  return i2;
}

template <class Key1, class Key2, class Value>
__EvTable<Key1,Key2,Value>::const_iterator2 __EvTable<Key1,Key2,Value>::get_2( __EvTable<Key1,Key2,Value>::const_iterator1 i1, Key2 key2 ) const
{
  const Container2& c2 = (*i1).second;
  const_iterator2 i2 = c2.begin();
  while ( i2 != c2.end() && key2nd(*i2) != key2 ) {
    ++i2;
  }
  return i2;
}

template <class Key1, class Key2, class Value>
bool __EvTable<Key1,Key2,Value>::get_1( __EvTable<Key1,Key2,Value>::const_iterator1 i1, Key2 key2, Value& value ) const
{
  const_iterator2 i2 = get_2( i1, key2 );
  if ( i2 == (*i1).second.end() ) {
    return false;
  }
  value = (*i2).second;
  return true;
}

template <class Key1, class Key2, class Value>
void __EvTable<Key1,Key2,Value>::append( const __EvTable<Key1,Key2,Value>& x )
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

template <class Key1, class Key2, class Value>
void __EvTable<Key1,Key2,Value>::append( Key1 key1, Key2 key2, const Value& value )
{
  iterator1 i1 = get_5( key1 );
  if ( i1 == storage.end() ) {
    std::pair<Key2,Value> p2( key2, value );
    Container2 c2;
    c2.push_back( p2 );
    std::pair<Key1,Container2> p1( key1, c2 );
    storage.push_back( p1 );
    return;
  }
  iterator2 i2 = get_3( i1, key2 );
  if ( i2 == (*i1).second.end() ) {
    std::pair<Key2,Value> p2( key2, value );
    (*i1).second.push_back( p2 );
    return;
  }
  (*i2).second = value; // only last value stored.
}

template <class T, class InputIterator >
class __EvHandler
{
  public:
#ifndef _MSC_VER
    typedef __EvTable<typename EDS::Event::code_type,state_type,__AnyPMFentry *> table_type;
#else
    typedef __EvTable<EDS::Event::code_type,state_type,__AnyPMFentry *> table_type;
#endif

    __EvHandler( __DeclareAnyPMF<T> * );

    bool Dispatch( T *, InputIterator, InputIterator, const Event& event );
    bool DispatchStub( T *, InputIterator, InputIterator,
		       const Event& event );
    bool DispatchTrace( InputIterator first, InputIterator last,
			const Event& event, std::ostream& out );
    void Out( std::ostream& ) const;

    convert<GENERIC::PMF, __member_function<T,Event>::pmf_type>  pmf;
    convert<GENERIC::DPMF,__member_function<T,Event>::dpmf_type> dpmf;

    table_type table;
};

template <class T, class InputIterator >
__EvHandler<T, InputIterator>::__EvHandler( __DeclareAnyPMF<T> *e )
{
  const table_type& p = T::ParentThisCls::get_ev_table();
  table.append( p );
  while ( e->func.dpmf != 0 ) {
    table.append( e->code, e->st, (__AnyPMFentry *)&e->func );
    ++e;
  }
}

template <class T, class InputIterator>
bool __EvHandler<T, InputIterator>::Dispatch( T *c, InputIterator first,
				   InputIterator last, const Event& event )
{
  if ( first == last ) {
    return false;
  }
  typename EDS::Event::code_type code = event.code();
  __AnyPMFentry *entry;
  table_type::const_iterator1 i1 = table.get_4( code );
  if ( i1 == table.end() ) {
    return false;
  }  
  while ( first != last && !table.get_1( i1, *first, entry ) ) {
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
				   InputIterator last, const Event& event )
{
  if ( first == last ) {
    return false;
  }
  typename EDS::Event::code_type code = event.code();
  __AnyPMFentry *entry;
  table_type::const_iterator1 i1 = table.get_4( code );
  if ( i1 == table.end() ) {
    return false;
  }  
  while ( first != last && !table.get_1( i1, *first, entry ) ) {
    ++first;
  }
  if ( first == last ) {
    return false;
  }
  return true;
}

template <class T, class InputIterator >
bool __EvHandler<T, InputIterator>::DispatchTrace( InputIterator first,
                  InputIterator last, const Event& event, std::ostream& out )
{
  if ( first == last ) {
    out << "\n\tStates stack empty?";
    return false;
  }
  typename EDS::Event::code_type code = event.code();
  __AnyPMFentry *entry;
  while ( first != last && !table.get( code, *first, entry ) ) {
    ++first;
  }
  if ( first == last ) {
    out << "\tCatcher not found for message 0x" << std::hex << code << std::dec;
    return false;
  }
  out << "\tMessage 0x" << std::hex << code << std::dec << " catcher "
      << entry->pmf_name << " (state " << *first << ")";
  return true;
}

template <class T, class InputIterator>
void __EvHandler<T, InputIterator>::Out( std::ostream& out ) const
{
  if ( table.empty() ) {
    return;
  }
  __AnyPMFentry *entry;
  table_type::const_iterator1 i1 = table.begin();
  while ( i1 != table.end() ) {
    typename EDS::Event::code_type key1 = table.key1st(*i1);
    table_type::const_iterator2 i2 = table.begin( i1 );
    out << "\tMessage: " << std::hex << key1 << std::dec << std::endl;
    while ( i2 != table.end( i1 ) ) {
      state_type key2 = table.key2nd(*i2);
      table.get( key1, key2, entry );
      out << "\t\tState " << key2 << ":\t" << entry->pmf_name << "\n";
      ++i2;
    }
    ++i1;
  }
}

class __EvHandler<EventHandler,h_iterator>
{
  public:
#ifndef _MSC_VER
    typedef __EvTable<typename EDS::Event::code_type,state_type,__AnyPMFentry *> table_type;
#else
    typedef __EvTable<EDS::Event::code_type,state_type,__AnyPMFentry *> table_type;
#endif

    __EvHandler( __DeclareAnyPMF<EventHandler> * )
      { }

    bool Dispatch( EventHandler *, h_iterator, h_iterator, const Event& )
      { return false; }
    bool DispatchStub( EventHandler *, h_iterator, h_iterator,
		       const Event& )
      { return false; }
    bool DispatchTrace( h_iterator, h_iterator, const Event&, ostream& )
      { return false; }
    void Out( ostream& ) const
      { }

    table_type table;
};


// *********************************************************** EDSEventHandler

class EventHandler
{
  public:
    typedef __EvHandler<EventHandler,h_iterator> evtable_type;
    typedef evtable_type::table_type table_type;

  protected:
    // See comment near EventHandler::EventHandler() implementation
    // HistoryContainer& theHistory;
    HistoryContainer theHistory;
    static evtable_type theEventsTable;
    static __DeclareAnyPMF<EventHandler> theDeclEventsTable[];

  public:

    class Init
    {
      public:
        Init();
        ~Init();
      private:
        static int _count;
    };

    EventHandler();
    explicit EventHandler( const Event::key_type& id );
    ~EventHandler();

    const string& who_is( const Event::key_type& k ) const;
    unsigned sid( const Event::key_type& k ) const;
    static EvManager *manager()
      { return _mgr; }
    void Send( const Event& e );
    void Send( const EventVoid& e );

/* ************************************************************ *\
   Member template will be nice here, but sorry...
   I put macro: if you want send message MSG to H, sending object t
   of class T, you write in class [derived from EventHandle]
   
   class X :
      public EventHandle
   {
     public:
       ...
       SEND_T_(T)
       ...
   } x;

   class T :
      public __pack_base
   {
      virtual void pack( std::ostream& s ) const;
      virtual void net_pack( std::ostream& s ) const;
      virtual void unpack( std::istream& s );
      virtual void net_unpack( std::istream& s );
   } t;


   And then can send it via
     ...
     x.SendMessage( H, MSG, t );

\* ************************************************************ */
#define SEND_T_(T) \
    void SendMessage( Event::key_type dst, Event::code_type code, const T& s ) \
      { \
        Event_base<T> e( code, s ); \
        e.dest( dst ); \
        EventHandler::Send( EDS::Event_convert<T>()( e ) ); \
      }

    Event::key_type self_id() const
      { return _id; }
    void State( state_type state )
      { PushState( state ); }
    void PushState( state_type state );
    state_type State() const;
    void PopState();
    void PopState( state_type );
    void PushTState( state_type state );
    void RemoveState( state_type );
    bool isState( state_type ) const;
    virtual bool Dispatch( const Event& );
    virtual bool DispatchStub( const Event& );
    virtual void DispatchTrace( const Event&, ostream&  );
    virtual void Trace( ostream& ) const;
    void TraceStack( ostream& ) const;
    static const table_type& get_ev_table()
      { return theEventsTable.table; }

  private:
    h_iterator __find( state_type );
    const_h_iterator __find( state_type ) const;

    Event::key_type _id;
    static EvManager *_mgr;

    friend class Init;
    friend class NetTransport;
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


#define DECLARE_RESPONSE_TABLE( cls, pcls )	                          \
  public:                                                                 \
    virtual void Trace( std::ostream& ) const;                            \
    virtual void DispatchTrace( const EDS::Event&, std::ostream& );       \
    typedef EDS::__EvHandler<cls,EDS::h_iterator> evtable_type;           \
    typedef cls ThisCls;			                          \
    typedef pcls ParentThisCls;                                           \
    static const table_type& get_ev_table()                               \
      { return theEventsTable.table; }                                    \
  protected:					                          \
    virtual bool Dispatch( const EDS::Event& );                           \
    virtual bool DispatchStub( const EDS::Event& );                       \
    static evtable_type theEventsTable;                                   \
    static EDS::__DeclareAnyPMF<cls> theDeclEventsTable[]

// Macro for specification of response table body beginning:
// DEFINE_RESPONSE_TABLE( XX )
//   RESPONSE_TABLE_ENTRY( ST_NRM, XW_EXPOSE, OXWEvExpose, XEventDispatch );
// END_RESPONSE_TABLE

#define DEFINE_RESPONSE_TABLE( cls )		                          \
cls::evtable_type cls::theEventsTable( theDeclEventsTable );              \
                                                                          \
bool cls::Dispatch( const EDS::Event& __event__ )                         \
{						                          \
  return theEventsTable.Dispatch( this, theHistory.begin(),               \
				  theHistory.end(), __event__ );          \
}						                          \
                                                                          \
bool cls::DispatchStub( const EDS::Event& __event__ )                     \
{						                          \
  return theEventsTable.DispatchStub( this, theHistory.begin(),           \
				      theHistory.end(), __event__ );      \
}						                          \
                                                                          \
void cls::DispatchTrace( const EDS::Event& __event__, std::ostream& out ) \
{						                          \
  theEventsTable.DispatchTrace( theHistory.begin(),                       \
	 		        theHistory.end(), __event__, out );       \
}						                          \
                                                                          \
void cls::Trace( std::ostream& out ) const                                \
{                                                                         \
  theEventsTable.Out( out );                                              \
}                                                                         \
EDS::__DeclareAnyPMF<cls> cls::theDeclEventsTable[] = {

// Macro for specification of response table entry:
// RESPONSE_TABLE_ENTRY( ST_NRM, XW_EXPOSE, OXWEvExpose, XEventDispatch );
//                       ~~~~~~  ~~~~~~~~~  ~~~~~~~~~~~  ~~~~~~~~~~~~~~
//                       State     Event      Catcher      Dispatcher

#define RESPONSE_TABLE_ENTRY( state, code, catcher, dispatch )            \
  { state, code,                                                          \
    {(EDS::__PMFentry<ThisCls>::PMF)catcher, (EDS::GENERIC::DPMF)dispatch, #catcher }},

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

// DIAG_DECLARE_GROUP(EventHandler);
// DIAG_DECLARE_GROUP(EventDispatch);

#ifdef __TRACE
/*
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
*/
#endif // __TRACE

#ifdef __WARN
/*
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
*/
#endif // __WARN
#endif // __TRACE || __WARN

} // namespace EDS

#endif  // __EventHandler_h
