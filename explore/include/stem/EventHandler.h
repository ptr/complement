// -*- C++ -*- Time-stamp: <10/03/08 13:12:43 ptr>

/*
 * Copyright (c) 1995-1999, 2002, 2003, 2005-2010
 * Petr Ovtchenkov
 * 
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __EventHandler_h
#define __EventHandler_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifndef __stem_Event_h
#include <stem/Event.h>
#endif

#include <algorithm>
#include <functional>
#ifndef STLPORT
# include <ext/functional>
#endif
#include <utility>
#include <vector>
#include <list>
#include <ostream>
#include <typeinfo>

#ifdef STLPORT
# include <type_traits>
#else
# include <misc/type_traits.h>
#endif

#include <mt/mutex>

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

namespace stem {

using namespace std;

typedef unsigned state_type;

#define ST_NULL     (stem::state_type)(0)
#define ST_TERMINAL (stem::state_type)(-1)

#ifdef __PG_USE_ABBREV
#  define __dispatcher                    _dch_
#  define __member_function               _mf_
#  define __dispatcher_convert_Event      _dch_cnv_Event_
#  define __dispatcher_convert_Event_extr _dch_cnv_Event_e_
#  define __AnyPMFentry APMFE
#endif

#define D1(cls) stem::__dispatcher<stem::__member_function<cls,stem::Event> >
#define D2(cls) stem::__dispatcher_void<stem::__member_function<cls,void>,stem::Event>

#define __D_EV_T(cls,T) stem::__dispatcher_convert<stem::__member_function<cls,stem::Event_base<T> >,stem::Event>
#define __D_T(cls,T) stem::__dispatcher_convert<stem::__member_function<cls,T>,stem::Event>

#ifndef __FIT_TEMPLATE_CLASSTYPE_BUG
#  define EV_EDS(state,event,handler) \
     RESPONSE_TABLE_ENTRY(state,event,handler,D1(ThisCls)::dispatch)
#  define EV_VOID(state,event,handler) \
     RESPONSE_TABLE_ENTRY(state,event,handler,D2(ThisCls)::dispatch)
#else // __FIT_TEMPLATE_CLASSTYPE_BUG
#  define EV_EDS(state,event,handler) \
     RESPONSE_TABLE_ENTRY(state,event,handler,__stem_Event::dispatch)
#  define EV_VOID(state,event,handler) \
     RESPONSE_TABLE_ENTRY(state,event,handler,__stem_Void::dispatch)
#endif // __FIT_TEMPLATE_CLASSTYPE_BUG

#define EV_Event_base_T_(state,event,handler,T) \
   RESPONSE_TABLE_ENTRY(state,event,handler,__D_EV_T(ThisCls,T)::dispatch)
#define EV_T_(state,event,handler,T) \
   RESPONSE_TABLE_ENTRY(state,event,handler,__D_T(ThisCls,T)::dispatch)

/*
#define EV_CALL(state,event,handler) \
  RESPONSE_TABLE_ENTRY(state,event,handler,D3(ThisCls)::dispatch)
*/

class EventHandler;
class EvManager;

struct GENERIC
{
    virtual void foo( Event& ) = 0; // This is to allow usage of virtual
                                    // catchers, indeed never used.
    typedef void (GENERIC::*PMF)();
    typedef void (*DPMF)( void*, PMF, const Event& );
};

namespace detail {

template <class T, class U>
struct convert
{
    // U operator ()( T x ) const
    //   { return (U)(x); }
};

template <class T>
struct convert<stem::Event,stem::Event_base<T> >
{
    stem::Event_base<T> operator ()( const stem::Event& x ) const
      {
        if ( (x.flags() & __Event_Base::expand) == 0 ) {
          throw std::invalid_argument( std::string("invalid conversion") );
        }
        stem::Event_base<T> tmp;
        tmp.unpack( x );
        
        return tmp;
      }
};

template <class T>
struct convert<stem::Event,T>
{
    T operator ()( const stem::Event& x ) const
      {
        if ( (x.flags() & __Event_Base::expand) == 0 ) {
          throw std::invalid_argument( std::string("invalid conversion") );
        }
        stem::Event_base<T> tmp;
        tmp.unpack( x );
        
        return tmp.value();
      }
};

template <class T>
struct convert<stem::Event_base<T>,stem::Event>
{
    stem::Event operator ()( const stem::Event_base<T>& x ) const
      {
        stem::Event tmp;

        x.pack( tmp );

        return tmp;
      }
};

template <>
struct convert<stem::Event,stem::Event>
{
    const stem::Event& operator ()( const stem::Event& x ) const
      { return x; }
};

template <>
struct convert<stem::Event,std::string>
{
    const std::string& operator ()( const stem::Event& x ) const
      { return x.value(); }
};

template <>
struct convert<stem::Event,void>
{
    void operator ()( const stem::Event& ) const
      { }
};

} // namespace detail

template <class T, class Arg>
struct __member_function
{
    typedef Arg argument_type;
    typedef typename std::tr1::add_lvalue_reference<typename std::tr1::add_const<typename std::tr1::remove_reference<Arg>::type>::type>::type const_reference_argument_type;
    typedef typename std::tr1::add_pointer<typename std::tr1::remove_pointer<T>::type>::type pointer_class_type;
    typedef void (T::*pmf_type)( const_reference_argument_type );
    typedef void (*dpmf_type)( typename std::tr1::add_const<typename std::tr1::add_pointer<typename std::tr1::remove_pointer<T>::type>::type>::type, pmf_type,
			       const_reference_argument_type arg );
};

template <class T>
struct __member_function<T,void>
{
    typedef void argument_type;
    typedef typename std::tr1::add_pointer<typename std::tr1::remove_pointer<T>::type>::type pointer_class_type;
    typedef void (T::*pmf_type)();
    typedef void (*dpmf_type)( typename std::tr1::add_const<typename std::tr1::add_pointer<typename std::tr1::remove_pointer<T>::type>::type>::type, pmf_type,
			       void * );
};

template <class PMF>
struct __dispatcher
{
    static void dispatch( typename PMF::pointer_class_type c, typename PMF::pmf_type pmf,
	                  typename PMF::const_reference_argument_type arg )
      { (c->*pmf)( arg ); }
};

template <class PMF, class Arg >
struct __dispatcher_void
{
    static void dispatch( typename PMF::pointer_class_type c, typename PMF::pmf_type pmf, const Arg& )
      {	(c->*pmf)(); }
};

template <class PMF, class Arg >
struct __dispatcher_convert
{
    static void dispatch( typename PMF::pointer_class_type c, typename PMF::pmf_type pmf, const Arg& arg )
      {	(c->*pmf)( typename stem::detail::convert<Arg,typename PMF::argument_type>()(arg) ); }
};

struct __AnyPMFentry
{
    stem::GENERIC::PMF  pmf;
    stem::GENERIC::DPMF dpmf;

    const char *pmf_name;
};

template <class T>
struct __PMFentry
{
    typedef void (T::*PMF)();

    PMF  pmf;
    __FIT_TYPENAME stem::GENERIC::DPMF dpmf;
    const char *pmf_name;
};

template <class T>
struct __DeclareAnyPMF
{
    state_type    st;
    /* __FIT_TYPENAME */ stem::code_type code;
    __FIT_TYPENAME stem::__PMFentry<T> func;
};

template <class Key1, class Key2, class Value>
class __EvTable
{
  public:
    typedef std::pair<Key2,Value> pair2_type;
    typedef std::vector<pair2_type> Container2;
    typedef std::pair<Key1,Container2> pair1_type;
    typedef std::vector<pair1_type> Container1;
    typedef typename Container1::iterator iterator1;
    typedef typename Container2::iterator iterator2;
    typedef typename Container1::const_iterator const_iterator1;
    typedef typename Container2::const_iterator const_iterator2;

    // Renaming get's was done due to VC 5.0 problem:
    // its unhappy with detecting const/nonconst function variant,
    // and with functions overloading (same names, parameters differ)
    // 
    bool get( Key1, Key2, Value& ) const;
    bool get_1( const_iterator1 i1, Key2 key2, Value& value ) const;
    const_iterator1 find( Key1 key1 ) const
      { return std::find_if( storage.begin(), storage.end(),
#ifdef STLPORT
                        std::compose1( std::bind2nd( eq_key1, key1 ), key1st )
#else
                        __gnu_cxx::compose1( std::bind2nd( eq_key1, key1 ), key1st )
#endif
          );
      }
    iterator1 find( Key1 key1 )
      { return std::find_if( storage.begin(), storage.end(),
#ifdef STLPORT
                        std::compose1( std::bind2nd( eq_key1, key1 ), key1st )
#else
                        __gnu_cxx::compose1( std::bind2nd( eq_key1, key1 ), key1st )
#endif
          );
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

    void append( Key1 key1, Key2 key2, const Value& value );

#ifdef STLPORT
    std::select1st<pair1_type> key1st;
    std::select1st<pair2_type> key2nd;
#else
    __gnu_cxx::select1st<pair1_type> key1st;
    __gnu_cxx::select1st<pair2_type> key2nd;
#endif
    std::equal_to<Key1> eq_key1;
    std::equal_to<Key2> eq_key2;
 
  protected:
   Container1 storage;
};

template <class Key1, class Key2, class Value>
bool __EvTable<Key1,Key2,Value>::get( Key1 key1, Key2 key2, Value& value ) const
{
  const_iterator1 i1 = std::find_if( storage.begin(), storage.end(),
#ifdef STLPORT
                                std::compose1( std::bind2nd( eq_key1, key1 ), key1st )
#else
                                __gnu_cxx::compose1( std::bind2nd( eq_key1, key1 ), key1st )
#endif
    );
  if ( i1 == storage.end() ) {
    return false;
  }
  const_iterator2 i2 = std::find_if( (*i1).second.begin(), (*i1).second.end(),
#ifdef STLPORT
                                std::compose1( std::bind2nd( eq_key2, key2 ), key2nd )
#else
                                 __gnu_cxx::compose1( std::bind2nd( eq_key2, key2 ), key2nd )
#endif
    );
  if ( i2 == (*i1).second.end() ) {
    return false;
  }
  value = (*i2).second;

  return true;
}

template <class Key1, class Key2, class Value>
bool __EvTable<Key1,Key2,Value>::get_1( __FIT_TYPENAME_ARG __EvTable<Key1,Key2,Value>::const_iterator1 i1, Key2 key2, Value& value ) const
{
  const_iterator2 i2 = std::find_if( (*i1).second.begin(), (*i1).second.end(),
#ifdef STLPORT
                                std::compose1( std::bind2nd( eq_key2, key2 ), key2nd )
#else
                                __gnu_cxx::compose1( std::bind2nd( eq_key2, key2 ), key2nd )
#endif
    );
  if ( i2 == (*i1).second.end() ) {
    return false;
  }
  value = (*i2).second;
  return true;
}

template <class Key1, class Key2, class Value>
void __EvTable<Key1,Key2,Value>::append( Key1 key1, Key2 key2, const Value& value )
{
  iterator1 i1 = find( key1 );

  if ( i1 == storage.end() ) {
    std::pair<Key2,Value> p2( key2, value );
    Container2 c2;
    c2.push_back( p2 );
    std::pair<Key1,Container2> p1( key1, c2 );
    storage.push_back( p1 );
    return;
  }
  iterator2 i2 = std::find_if( (*i1).second.begin(), (*i1).second.end(),
#ifdef STLPORT
                          std::compose1( std::bind2nd( eq_key2, key2 ), key2nd )
#else
                          __gnu_cxx::compose1( std::bind2nd( eq_key2, key2 ), key2nd )
#endif
    );
  if ( i2 == (*i1).second.end() ) {
    std::pair<Key2,Value> p2( key2, value );
    (*i1).second.push_back( p2 );
    return;
  }
  (*i2).second = value; // only last value stored.
}

template <class T, class InputIterator>
class __EvHandler
{
  public:
    typedef typename T::table_type table_type;

    __EvHandler()
      { __EvTableLoader( &table, (T *)0 ); }

    bool Dispatch( T *, InputIterator, InputIterator, const Event& event );
    bool DispatchStub( T *, InputIterator, InputIterator,
		       const Event& event );
    bool DispatchTrace( InputIterator first, InputIterator last,
			const Event& event, std::ostream& out );
    void Out( std::ostream& ) const;

    table_type table;
};

template <class T, class InputIterator>
bool __EvHandler<T, InputIterator>::Dispatch( T* c, InputIterator first,
				   InputIterator last, const Event& event )
{
  if ( first == last ) {
    return false;
  }
  stem::code_type code = event.code();
  __AnyPMFentry* entry;
  typename table_type::const_iterator1 i1 = table.find( code );
  if ( i1 == table.end() ) {
    return false;
  }  
  while ( first != last ) {
    if ( table.get_1( i1, *first++, entry ) ) {
      (*entry->dpmf)( c, entry->pmf, event );
      return true;
    }
  }
  return false;
}

template <class T, class InputIterator>
bool __EvHandler<T, InputIterator>::DispatchStub( T *, InputIterator first,
				   InputIterator last, const Event& event )
{
  if ( first == last ) {
    return false;
  }
  stem::code_type code = event.code();
  __AnyPMFentry *entry;
  typename table_type::const_iterator1 i1 = table.find( code );
  if ( i1 == table.end() ) {
    return false;
  }  
  while ( first != last ) {
    if ( table.get_1( i1, *first++, entry ) ) {
      return true;
    }
  }
  return false;
}

template <class T, class InputIterator>
bool __EvHandler<T, InputIterator>::DispatchTrace( InputIterator first,
                  InputIterator last, const Event& event, std::ostream& out )
{
  if ( first == last ) {
    out << "\n\tStates stack empty?";
    return false;
  }
  stem::code_type code = event.code();
  __AnyPMFentry *entry;
  int f = out.flags();
  while ( first != last ) {
    if ( table.get( code, *first, entry ) ) {
      out << "\tMessage " << std::hex << std::showbase << code << std::dec << " catcher "
          << entry->pmf_name << " (state " << *first << ")";
#ifdef STLPORT
      out.flags( f );
#else
      out.flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      return true;
    }
    ++first;
  }
  out << "\tCatcher not found for message " << std::hex << std::showbase << code;
#ifdef STLPORT
  out.flags( f );
#else
  out.flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
  return false;
}

template <class T, class InputIterator>
void __EvHandler<T, InputIterator>::Out( std::ostream& out ) const
{
  if ( table.empty() ) {
    return;
  }
  __AnyPMFentry *entry;
  typename table_type::const_iterator1 i1 = table.begin();
  int f = out.flags();
  while ( i1 != table.end() ) {
    stem::code_type key1 = table.key1st(*i1);
    typename table_type::const_iterator2 i2 = table.begin( i1 );
    out << "\tMessage: " << std::hex << std::showbase << key1 << std::dec << std::endl;
    while ( i2 != table.end( i1 ) ) {
      state_type key2 = table.key2nd(*i2++);
      table.get( key1, key2, entry );
      out << "\t\tState " << key2 << ":\t" << entry->pmf_name << "\n";
    }
    ++i1;
  }
#ifdef STLPORT
  out.flags( f );
#else
  out.flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
}

typedef std::list<state_type> HistoryContainer;
typedef HistoryContainer::iterator h_iterator;
typedef HistoryContainer::const_iterator const_h_iterator;

template <>
class __EvHandler<EventHandler,h_iterator >
{
  public:
    __EvHandler()
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
};


// *********************************************************** EventHandler

class EventHandler
{
  public:
    typedef __EvHandler<EventHandler,h_iterator> evtable_type;
    typedef __EvTable<code_type,state_type,__AnyPMFentry *> table_type;
    typedef __DeclareAnyPMF<EventHandler> evtable_decl_type;
    typedef EventHandler ThisCls;

  protected:
    typedef std::list<addr_type> addr_container_type;

    // See comment near EventHandler::EventHandler() implementation
    // HistoryContainer& theHistory;
    HistoryContainer theHistory;
    std::tr2::recursive_mutex _theHistory_lock;

  public:
    typedef addr_container_type::const_iterator id_iterator;

  public:

    class Init
    {
      public:
        Init();
        ~Init();
      private:
        static void _guard( int );
        static void __at_fork_prepare();
        static void __at_fork_child();
        static void __at_fork_parent();
    };

    __FIT_DECLSPEC EventHandler();
    explicit __FIT_DECLSPEC EventHandler( const char* info );
    explicit __FIT_DECLSPEC EventHandler( const addr_type& id, int nice = 0 );
    explicit __FIT_DECLSPEC EventHandler( const addr_type& id, const char* info );
    virtual __FIT_DECLSPEC ~EventHandler();

    void enable();
    void disable();

    __FIT_DECLSPEC bool is_avail( const addr_type& id ) const;
    static EvManager* manager()
      { return _mgr; }
    static addr_type ns();
    addr_type set_default() const; // become default object
    __FIT_DECLSPEC void Send( const Event& e ) const;
    __FIT_DECLSPEC void Forward( const Event& e ) const;
    __FIT_DECLSPEC void sync_call( const Event& e );

    template <class D>
    void Send( const stem::Event_base<D>& e ) const
      { EventHandler::Send( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }

    template <class D>
    void Forward( const stem::Event_base<D>& e ) const
      { EventHandler::Forward( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }

    template <class D>
    void sync_call( const stem::Event_base<D>& e )
      { EventHandler::sync_call( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }

    const addr_type& self_id() const
      { return _ids.empty() ? stem::badaddr : _ids.front(); }
    id_iterator self_ids_begin() const
      { return _ids.begin(); }
    id_iterator self_ids_end() const
      { return _ids.end(); }
    void solitary();

    void State( state_type state )
      { PushState( state ); }
    __FIT_DECLSPEC void PushState( state_type state );
    __FIT_DECLSPEC state_type State() const;
    __FIT_DECLSPEC void PopState();
    __FIT_DECLSPEC void PopState( state_type );
    __FIT_DECLSPEC void PushTState( state_type state );
    __FIT_DECLSPEC void RemoveState( state_type );
    __FIT_DECLSPEC bool isState( state_type ) const;
    virtual bool Dispatch( const Event& )
      { return false; }
    virtual bool DispatchStub( const Event& )
      { return false; }
    virtual bool DispatchTrace( const Event&, ostream& )
      { return false; }
    virtual void Trace( ostream& ) const
      { }
    virtual const std::type_info& classtype() const
       { return typeid(EventHandler); }
    __FIT_DECLSPEC void TraceStack( ostream& ) const;
    virtual int flags() const;

  private:
    EventHandler( const EventHandler& );
    EventHandler& operator =( const EventHandler& );

    h_iterator __find( state_type );
    const_h_iterator __find( state_type ) const;
    int _nice;

  protected:
    addr_container_type _ids;

  private:
    static class EvManager *_mgr;
    static class Names     *_ns;

    friend class Init;
    friend class EvManager;
};

template <class Handler>
void __EvTableLoader( EventHandler::table_type *table, Handler * )
{
  __EvTableLoader( table, (typename Handler::ParentThisCls *)0 );
  const typename Handler::evtable_decl_type *__e = Handler::get_ev_table_decl();
  while ( __e->func.dpmf != 0 ) {
//    if ( !__e->func.valid ) {
//      std::cerr << "Function type mismatch: " << typeid(Handler).name() << "::"
//                << __e->func.pmf_name << std::endl;
//      abort();
//    }
    table->append( __e->code, __e->st, (__AnyPMFentry *)&__e->func );
    ++__e;
  }
}

template <>
inline void __EvTableLoader<EventHandler>( EventHandler::table_type *,
                                           EventHandler * )
{ }


// ***************************************************************************

#ifdef __FIT_TEMPLATE_CLASSTYPE_BUG
#  define  __FIT_TEMPLATE_CLASSTYPE_BUG_PARTIAL_WORAROUND(cls) \
     typedef D2(cls) __stem_Void; \
     typedef D1(cls) __stem_Event;
#else // __FIT_TEMPLATE_CLASSTYPE_BUG
#  define  __FIT_TEMPLATE_CLASSTYPE_BUG_PARTIAL_WORAROUND(cls)
#endif // __FIT_TEMPLATE_CLASSTYPE_BUG


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
    __FIT_TEMPLATE_CLASSTYPE_BUG_PARTIAL_WORAROUND(cls)                   \
    virtual void Trace( std::ostream& out ) const                       \
       { theEventsTable.Out( out ); }                                     \
    virtual const std::type_info& classtype() const                       \
       { return typeid(ThisCls); }                                        \
    typedef stem::__EvHandler<cls,stem::h_iterator> evtable_type;           \
    typedef stem::__DeclareAnyPMF<cls> evtable_decl_type;                  \
    typedef cls ThisCls;			                          \
    typedef pcls::ThisCls ParentThisCls;	                          \
    static __FIT_DECLSPEC const evtable_decl_type *get_ev_table_decl();    \
  protected:					                          \
    virtual bool DispatchTrace( const stem::Event& __e, std::ostream& __s )\
       {                                                                  \
         return theEventsTable.DispatchTrace( theHistory.begin(),         \
                                              theHistory.end(), __e, __s ); } \
    virtual bool Dispatch( const stem::Event& __e )                        \
       {                                                                  \
         return theEventsTable.Dispatch( this, theHistory.begin(),        \
                                         theHistory.end(), __e ); }       \
    virtual bool DispatchStub( const stem::Event& __e )                    \
       {                                                                  \
         return theEventsTable.DispatchStub( this, theHistory.begin(),    \
                                             theHistory.end(), __e ); }   \
    static __FIT_DECLSPEC evtable_type theEventsTable;                     \
    static __FIT_DECLSPEC evtable_decl_type theDeclEventsTable[]

// Macro for specification of response table body beginning:
// DEFINE_RESPONSE_TABLE( XX )
//   RESPONSE_TABLE_ENTRY( ST_NRM, XW_EXPOSE, OXWEvExpose, XEventDispatch );
// END_RESPONSE_TABLE

#define DEFINE_RESPONSE_TABLE( cls )		                          \
__FIT_DECLSPEC cls::evtable_type cls::theEventsTable;                      \
                                                                          \
__FIT_DECLSPEC const cls::evtable_decl_type *cls::get_ev_table_decl()      \
       { return theDeclEventsTable; }                                     \
__FIT_DECLSPEC stem::__DeclareAnyPMF<cls> cls::theDeclEventsTable[] = {

// Macro for specification of response table entry:
// RESPONSE_TABLE_ENTRY( ST_NRM, XW_EXPOSE, OXWEvExpose, XEventDispatch );
//                       ~~~~~~  ~~~~~~~~~  ~~~~~~~~~~~  ~~~~~~~~~~~~~~
//                       State     Event      Catcher      Dispatcher

#define RESPONSE_TABLE_ENTRY( state, code, catcher, dispatch )            \
  { state, code,                                                          \
    {(stem::__PMFentry<ThisCls>::PMF)&ThisCls::catcher, (stem::GENERIC::DPMF)dispatch, #catcher }},

// Macro for specification of response table end:

#define END_RESPONSE_TABLE			\
  { 0, 0, {0, 0, "End of table" } }             \
};

class stem_scope
{
  public:
    explicit stem_scope( const EventHandler& obj ) :
        r( obj )
      { const_cast<EventHandler&>(r).enable(); }
    ~stem_scope()
      { const_cast<EventHandler&>(r).disable(); }

  private:
    stem_scope( const stem_scope& );
    stem_scope& operator =( const stem_scope& );

    const EventHandler& r;
};

// #ifndef __FIT_NAMESPACE_TYPEDEF_BUG
} // namespace stem
// #endif

#endif  // __EventHandler_h
