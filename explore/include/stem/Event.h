// -*- C++ -*- Time-stamp: <96/08/22 21:10:52 ptr>
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
template <class S, class D > class OXWEventT;

class GENERIC;
class OXWEventsCore;

typedef OXWEventT<OXWEventsCore,void> OXWEvent;

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
    operator const OXWEvent& () const
      { return *( (OXWEvent *)this ); }
    operator D& ()
      {
	WARN( sz == 0, "You made attempt to read not allocated data!" );
	return sz ? *DataObject : error_data;
      }
    operator const D& () const
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

#ifndef __TEMPLATE_DB__
#include "Event.cc"
#endif

#endif
