#ident "%Z%%Q%$RCSfile$ ($Revision$): %H% %T%" // -*- C++ -*-

#ifndef __OXW_Event_h
#define __OXW_Event_h

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif

#ifndef DEFALLOC_H
#include <stl/defalloc.h>
#endif

#ifndef __OXW_OXWdefs_h
#include <OXW/OXWdefs.h>
#endif

#ifndef __CLASS_checks_h
#include <CLASS/checks.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

template <class T> class OXWEventsTable<T>;
template <class T> class OXWCallbackObject;
template <class S, class D > class OXWEventT;

class GENERIC;
class OXWEventsCore;

typedef OXWEventT<OXWEventsCore,void> OXWEvent;
typedef OXWEventT<OXWEventsCore,XEvent> OXWEventX;
typedef OXWEventT<OXWEventsCore,OXWCallbackObject<GENERIC> > OXWEventCb;

template <class T>
class OXWCallbackObject
{
  public:
    typedef void (T::*PMF)();
    OXWCallbackObject( T *o, PMF pmf )
      { object = o; Pmf = pmf; }
    OXWCallbackObject()
      { object = 0; Pmf = 0; }
    operator OXWCallbackObject<GENERIC>&()
      { return *((OXWCallbackObject<GENERIC> *)this); }
    T *object;
    PMF Pmf;
};

struct OXWInt2
{
  int A;
  int B;
};

// ***************************************************************** OXWEventT

template <class S, class D >
class OXWEventT
{
  friend OXWEvent;

  public:
    typedef allocator<D> Allocator;
    typedef Allocator::value_type value_type;
    typedef Allocator::pointer pointer;
    typedef Allocator::const_pointer const_pointer;
    typedef Allocator::reference reference;
    typedef Allocator::const_reference const_reference;

    OXWEventT() :
	Message( 0 ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( 0 )
      { }
    OXWEventT( message_type msg ) :
	Message( msg ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( 0 )
      { }
    OXWEventT( message_type msg, const D& data ) :
	Message( msg ),
	theSender( 0 )
      {
	DataObject = data_allocator.allocate(1);
	construct( DataObject, data );
	sz = sizeof( D );
      }
    OXWEventT( const OXWEvent& e ) :
	Message( e.GetMessage() ),
	theSender( (S *)e.Sender() ),
	sz( e.DataSize() )
      {
	if ( sz ) {
	  WARN( sz != sizeof( D ), "Inconsistent data size!" );
	  DataObject = data_allocator.allocate(1);
	  construct( DataObject, *((D *)e.Data()) );
	} else {
	  DataObject = (D *)e.Data();
	}
      }
    ~OXWEventT()
      {
	if ( sz ) {
	  data_allocator.deallocate( DataObject );
	}
      }

    message_type GetMessage() const
      { return Message; }
    S *Sender() const
      { return theSender; }
    D *Data() const
      { return DataObject; }
    size_t DataSize() const
      { return sz; }
    void Sender( S *x )
      { theSender = x; }
    operator OXWEvent& ()
      { return *( (OXWEvent *)this ); }
    operator D& ()
      {
	WARN( sz == 0, "You made attempt to read not allocated data!" );
	return sz ? *DataObject : error_data;
      }
    static D error_data;

  private:
    message_type Message;
    S *theSender;
    D *DataObject;
    size_t sz;

    static Allocator data_allocator;
};

template <class S, class D >
OXWEventT<S,D>::Allocator OXWEventT<S,D>::data_allocator;

template <class S, class D >
D OXWEventT<S,D>::error_data;

// ********************************************* OXWEventT<OXWEventsCore,void>

class OXWEventT<OXWEventsCore,void>
{
  friend OXWEventsTable<GENERIC>;
  friend OXWEvent;

  public:
    OXWEventT() :
	Message( 0 ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( 0 )
      { }
    OXWEventT( message_type msg ) :
	Message( msg ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( 0 )
      { }
    ~OXWEventT()
      {
	if ( sz ) {
	  delete DataObject;
	}
      }

    message_type GetMessage() const
      { return Message; }
    OXWEventsCore *Sender() const
      { return theSender; }
    void *Data() const
      { return DataObject; }
    size_t DataSize() const
      { return sz; }
    void Sender( OXWEventsCore *x )
      { theSender = x; }
    operator OXWEventX& ()
      {
	PRECONDITION( Message < LASTEvent );
	return *((OXWEventX *)this);
      }

    OXWEventT( const OXWEventT& e ) :
	Message( e.Message ),
	theSender( e.theSender ),
	sz( e.sz )
      {
	if ( sz ) {
	  DataObject = (void *)new char [sz];
	  memcpy( DataObject, e.DataObject, sz );
	} else {
	  DataObject = e.DataObject;
	}
      }

  protected:
    operator =( const OXWEventT& e )
      { construct( this, e ); }

  private:
    message_type Message;
    OXWEventsCore *theSender;
    void *DataObject;
    size_t sz;
};

// ******************************************* OXWEventT<OXWEventsCore,XEvent>

class OXWEventT<OXWEventsCore,XEvent>
{
  friend OXWEvent;

  public:
    OXWEventT() :
	Message( 0 ),
	theSender( 0 ),
	DataObject( 0 ),
	sz( 0 )
      { }
    OXWEventT( const XEvent& x ) :
	Message( x.type ),
	theSender( 0 ),
	DataObject( (XEvent *)&x ),
	sz( 0 )
      { }
    OXWEventT( const OXWEvent& e ) :
	Message( e.GetMessage() ),
	theSender( 0 ),
	DataObject( (XEvent *)e.Data() )
	sz( 0 )
      {
	PRECONDITION( Message < LASTEvent );
      }
    message_type GetMessage() const
      { return Message; }

    operator XEvent& ()
      {	return *DataObject; }
    operator OXWEvent& ()
      { return *( (OXWEvent *)this ); }

  private:
    message_type Message;
    OXWEventsCore *theSender;
    XEvent *DataObject;
    size_t sz;
};

#endif
