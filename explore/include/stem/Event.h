// -*- C++ -*- Time-stamp: <97/09/17 16:10:15 ptr>
#ifndef __EDS_Event_h
#define __EDS_Event_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifndef __EDS_EDSdefs_h
#include <EDS/EDSdefs.h>
#endif

#ifndef __CLASS_checks_h
#include <CLASS/checks.h>
#endif

template <class T> class EDSEventsTable<T>;
template <class S, class D> class EDSEventT;

class GENERIC;
class EDSEventsCore;

template <class S, class D>
class EDSEvent_base
{
  public:
    typedef D value_type;
    typedef D * pointer;
    typedef size_t size_type;
    typedef const D * const_pointer;

    EDSEvent_base() :
	Message( 0 ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( 0 ),
	n( 0 )
      { }

    EDSEvent_base( message_type msg ) :
	Message( msg ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( 0 ),
	n( 0 )
      { }

    EDSEvent_base( message_type msg, const D *first, const D *last ) :
	Message( msg ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( sizeof(D) ),
	n( last - first )
      { }

    EDSEvent_base( const EDSEvent_base& e ) :
	Message( e.Message ),
	theSender( (S *)e.theSender ),
	DataObject( 0 ),
	sz( e.sz ),
	n( e.n )
      { }

    message_type GetMessage() const
      { return Message; }
    S *Sender() const
      { return theSender; }
    pointer Data() const
      { return DataObject; }
    size_type DataSize() const
      { return sz; }
    size_type Count() const
      { return n; }
    void Sender( const S *x ) const
      { const_cast<S *>(theSender) = const_cast<S *>(x); }

  protected:
    message_type Message;
    S *theSender;
    size_type sz;
    size_type n;
    pointer DataObject;
};

typedef EDSEventT<EDSEventsCore,void> EDSEvent;

// ***************************************************************** EDSEventT

template <class S, class D>
class EDSEventT :
    public EDSEvent_base<S,D>
{
  friend EDSEvent;

  public:
    typedef D& reference;
    typedef const D& const_reference;

    EDSEventT() :
	EDSEvent_basic()
      { }
    EDSEventT( message_type msg, const D& x );
    EDSEventT( message_type msg, const D *first, const D *last );

    EDSEventT( const EDSEvent& e );
    ~EDSEventT();

    operator EDSEvent& ()
      { return *reinterpret_cast<EDSEvent *>(this); }
    operator const EDSEvent& () const
      { return *reinterpret_cast<const EDSEvent * const >(this); }
    operator reference ()
      {
	CHECK_LENGTH( sz != 0 );
	return *DataObject;
      }
    operator const_reference () const
      {
	CHECK_LENGTH( sz != 0 );
	return *DataObject;
      }
    reference operator []( size_t i )
      {
	CHECK_LENGTH( sz != 0 );
	CHECK_RANGE( i < n );
	return *(DataObject + i);
      }
    const_reference operator []( size_t i ) const
      {
	CHECK_LENGTH( sz != 0 );
	CHECK_RANGE( i < n );
	return *(DataObject + i);
      }
};

template <class S, class D>
EDSEventT<S,D>::EDSEventT( message_type msg, const D& x ) :
    EDSEvent_base<S,D>( msg )
{
  n = 1;
  sz = sizeof(D);
  DataObject = new D ( x );
}

template <class S, class D>
EDSEventT<S,D>::EDSEventT( message_type msg, const D *first, const D *last ) :
    EDSEvent_base<S,D>( msg, first, last )
{
  if ( n > 0 ) {
    CHECK_RANGE( sz > 0 );
    DataObject = static_cast<pointer>(::operator new( n * sz ));
    uninitialized_copy( first, last, DataObject );
  }
}

template <class S, class D>
EDSEventT<S,D>::EDSEventT( const EDSEvent& e ) :
    EDSEvent_base<S,D>( e )
{
  if ( n > 0 ) {
    CHECK_RANGE( sz > 0 );
    DataObject = static_cast<pointer>(::operator new( n * sz ));
    const_pointer first = static_cast<const_pointer>( e.Data() );
    uninitialized_copy_n( first, n, DataObject );
  } else {
    DataObject = static_cast<pointer>(e.Data());
  }
}

template <class S, class D>
EDSEventT<S,D>::~EDSEventT()
{
  if ( n > 0 ) {
    CHECK_RANGE( sz > 0 );
    destroy( DataObject, DataObject + n );
    ::operator delete( DataObject );
  }
}

// ********************************************* EDSEventT<EDSEventsCore,void>

class EDSEventT<EDSEventsCore,void> :
    public EDSEvent_base<EDSEventsCore,void>
{
  friend EDSEventsTable<GENERIC>;

  public:
    EDSEventT() :
	EDSEvent_base<EDSEventsCore,void>()
      { }
    EDSEventT( message_type msg ) :
	EDSEvent_base<EDSEventsCore,void>( msg )
      { }
    EDSEventT( const EDSEventT& e ) :
	EDSEvent_base<EDSEventsCore,void>( e )
      {
	if ( n > 0 ) {
	  CHECK_RANGE( sz > 0 );
	  DataObject = ::operator new( n * sz );
	  uninitialized_copy_n(
	    static_cast<char * const>(e.DataObject), n * sz,
	    static_cast<char *>(DataObject) );
	} else {
	  DataObject = e.DataObject;
	}
      }
    ~EDSEventT()
      {
	if ( n > 0 ) {
	  CHECK_RANGE( sz > 0 );
	  ::operator delete( DataObject );
	}
      }
};

template <class T> class EDSCallbackObject;
typedef EDSEventT<EDSEventsCore,EDSCallbackObject<GENERIC> > EDSEventCb;

template <class T>
class EDSCallbackObject
{
  public:
    typedef void (T::*PMF)();
    EDSCallbackObject( T *o, PMF pmf )
      { object = o; Pmf = pmf; }
    EDSCallbackObject()
      { object = 0; Pmf = 0; }
    operator EDSCallbackObject<GENERIC>&()
      { return *((EDSCallbackObject<GENERIC> *)this); }
    operator const EDSCallbackObject<GENERIC>&() const
      { return *((EDSCallbackObject<GENERIC> *)this); }
    T *object;
    PMF Pmf;
};

#endif
