// -*- C++ -*- Time-stamp: <96/11/03 10:40:16 ptr>
#ifndef __EDS_Event_h
#define __EDS_Event_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifndef DEFALLOC_H
#include <stl/defalloc.h>
#endif

#ifndef __EDS_EDSdefs_h
#include <EDS/EDSdefs.h>
#endif

#ifndef __CLASS_checks_h
#include <CLASS/checks.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

template <class T> class OXWEventsTable<T>;
template <class S, class D> class OXWEventT;

class GENERIC;
class OXWEventsCore;

template <class S, class D>
class EDSEvent_basic
{
  public:
    typedef D value_type;
    typedef D * pointer;
    typedef size_t size_type;
    typedef const D * const_pointer;

    EDSEvent_basic() :
	Message( 0 ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( 0 ),
	n( 0 )
      { }

    EDSEvent_basic( message_type msg ) :
	Message( msg ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( 0 ),
	n( 0 )
      { }

    EDSEvent_basic( message_type msg, size_type num, size_type size ) :
	Message( msg ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( size ),
	n( num )
      { }

    EDSEvent_basic( const EDSEvent_basic& e ) :
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

typedef OXWEventT<OXWEventsCore,void> OXWEvent;

// ***************************************************************** OXWEventT

template <class S, class D>
class OXWEventT :
    public EDSEvent_basic<S,D>
{
  friend OXWEvent;

  public:
    typedef D& reference;
    typedef const D& const_reference;

    OXWEventT() :
	EDSEvent_basic()
      { }
    OXWEventT( message_type msg, size_type num = 1,
	       size_type size = sizeof( D ) );
    OXWEventT( message_type msg, const_reference data, size_type num = 1,
	       size_type size = sizeof( D ) );
    OXWEventT( const OXWEvent& e );
    ~OXWEventT();

    operator OXWEvent& ()
      { return *reinterpret_cast<OXWEvent *>(this); }
    operator const OXWEvent& () const
      { return *reinterpret_cast<const OXWEvent * const >(this); }
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
	return *reinterpret_cast<pointer>(
	  reinterpret_cast<char *>(DataObject) + i * sz);
      }
    const_reference operator []( size_t i ) const
      {
	CHECK_LENGTH( sz != 0 );
	CHECK_RANGE( i < n );
	return *reinterpret_cast<const_pointer>(
	  reinterpret_cast<char *>(DataObject) + i * sz);
      }
};

template <class S, class D>
OXWEventT<S,D>::OXWEventT( message_type msg, OXWEventT::const_reference data,
			   OXWEventT::size_type num,
			   OXWEventT::size_type size ) :
    EDSEvent_basic<S,D>( msg, num, size )
{
  CHECK_OUT_OF_RANGE( n >= 1 && sz >= sizeof( value_type ) );
  DataObject = (pointer)::operator new( n * sz );
  while ( num-- ) {
    construct( reinterpret_cast<pointer>(
      reinterpret_cast<char *>(DataObject) + num * sz),
	       *reinterpret_cast<const_pointer>(
		 reinterpret_cast<const char *>(&data) + num * sz) );
  }
}

template <class S, class D>
OXWEventT<S,D>::OXWEventT( message_type msg, OXWEventT::size_type num,
			   OXWEventT::size_type size ) :
    EDSEvent_basic<S,D>( msg, num, size )
{
  CHECK_OUT_OF_RANGE( n >= 1 && sz >= sizeof( value_type ) );
  DataObject = reinterpret_cast<pointer>(::operator new( n * sz ));
  while ( num-- ) {
    new (reinterpret_cast<char *>(DataObject) + num * sz) D();
  }
}

template <class S, class D>
OXWEventT<S,D>::OXWEventT( const OXWEvent& e ) :
    EDSEvent_basic<S,D>( e )
{
  if ( sz ) {
    CHECK_OUT_OF_RANGE( n >= 1 && sz >= sizeof( value_type ) );
    DataObject = reinterpret_cast<pointer>(::operator new( n * sz ));
    size_type num = n;
    while ( num-- ) {
      construct( reinterpret_cast<pointer>(
	reinterpret_cast<char *>(DataObject) + num * sz),
		 *reinterpret_cast<const_pointer>(
		   reinterpret_cast<const char *>(e.Data()) + num * sz) );
    }
  } else {
    DataObject = reinterpret_cast<pointer>(e.Data());
  }
}

template <class S, class D>
OXWEventT<S,D>::~OXWEventT()
{
  if ( sz ) {
    while ( n-- ) {
      destroy( reinterpret_cast<pointer>(
	reinterpret_cast<char *>(DataObject) + n * sz) );
    }
    ::operator delete( DataObject );
  }
}

// ********************************************* OXWEventT<OXWEventsCore,void>

class OXWEventT<OXWEventsCore,void> :
    public EDSEvent_basic<OXWEventsCore,void>
{
  friend OXWEventsTable<GENERIC>;
  friend OXWEvent;

  public:
    OXWEventT() :
	EDSEvent_basic<OXWEventsCore,void>()
      { }
    OXWEventT( message_type msg ) :
	EDSEvent_basic<OXWEventsCore,void>( msg )
      { }
    OXWEventT( const OXWEventT& e ) :
	EDSEvent_basic<OXWEventsCore,void>( e )
      {
	if ( sz ) {
	  CHECK_OUT_OF_RANGE( n >= 1 );
	  DataObject = ::operator new( n * sz );
	  memcpy( DataObject, e.DataObject, n * sz );
	} else {
	  DataObject = e.DataObject;
	}
      }
    ~OXWEventT()
      {
	if ( sz ) {
	  ::operator delete( DataObject );
	}
      }
};

#endif
