#ident "%Z%%Q%$RCSfile$ ($Revision$): %H% %T%" // -*- C++ -*-

#ifndef __OXW_Event_h
#define __OXW_Event_h

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif

#ifndef __OXW_OXWdefs_h
#include <OXW/OXWdefs.h>
#endif

#ifndef __CLASS_checks_h
#include <CLASS/checks.h>
#endif

class GENERIC;
template <class T> class OXWEventsTable<T>;
class OXWEventsCore;
template <class S, class D > class OXWEventT;
typedef OXWEventT<OXWEventsCore,GENERIC> OXWEvent;

struct OXWInt2
{
  int A;
  int B;
};

template <class S, class D >
class OXWEventT
{
  friend OXWEventsTable<GENERIC>;

  public:
    OXWEventT() :
	Message( 0 ),
	theSender( 0 ),
	DataObject( 0 )
      { }
    OXWEventT( message_type msg ) :
	Message( msg ),
	theSender( 0 ),
	DataObject( 0 )
      { }
    OXWEventT( message_type msg, D *data ) :
	Message( msg ),
	theSender( 0 ),
	DataObject( data )
      { }
    OXWEventT( XEvent& x ) :
	Message( x.type ),
	theSender( 0 ),
	DataObject( (D *)&x )
      { }
    OXWEventT( OXWEvent& e ) :
	Message( e.GetMessage() ),
	theSender( (S *)e.Sender() ),
	DataObject( (D *)e.Data() )
      { }

    message_type GetMessage() const
      { return Message; }
    S *Sender() const
      { return theSender; }
    D *Data() const
      { return DataObject; }
    void Sender( S *x )
      { theSender = x; }
    operator OXWEvent& ()
      { return *( (OXWEventT<OXWEventsCore,GENERIC> *)this ); }
    operator XEvent& ()
      {
	PRECONDITION( Message < LASTEvent );
	return *((XEvent *)DataObject);
      }
    operator OXWInt2& ()
      { return *((OXWInt2 *)DataObject); }

  private:
    message_type Message;
    S *theSender;
    D *DataObject;
};

#endif
