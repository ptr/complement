#ident "%Z%$RCSfile$ v.$Revision$ %H% %T%" // -*- C++ -*-

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

struct tagLParam {
  unsigned char Hi;
  unsigned char hi;
  unsigned char lo;
  unsigned char Lo;
};

struct tagIParams
{
  union {
    unsigned long AParam;
    tagLParam AP;
  };
  union {
    unsigned long BParam;
    tagLParam BP;
  };
  union {
    unsigned long CParam;
    tagLParam CP;
  };
  union {
    unsigned long DParam;
    tagLParam DP;
    class OXWEventsCore *ECObject;
    class OXWWindowBase *WBObject;
  };
};

struct tagFParams
{
  double XParam;
  double YParam;
};

class OXWEvent
{
  public:
    enum contained_type {
      IntegerParams,
      FloatParams,
      StringParam,
      XEventParam
    };

    unsigned long Message;

    OXWEvent();
    OXWEvent( unsigned long );
    OXWEvent( unsigned long, const tagIParams& );
    OXWEvent( unsigned long, const tagFParams& );
    OXWEvent( unsigned long, const char * );
    OXWEvent( unsigned long, XEvent& );
    OXWEvent( XEvent& );

    void SetMessage( unsigned long );
    void SetIParams( unsigned long, const tagIParams& );
    void SetFParams( unsigned long, const tagFParams& );
    void SetSParam( unsigned long, const char * );
    void SetXEvent( unsigned long, XEvent& );
    void SetXEvent( XEvent& );

    unsigned long GetMessage() const
      { return Message; }
    tagIParams& GetIParams();
    tagFParams& GetFParams();
    char *GetSParam();
    XEvent *GetXEvent();

    contained_type isA() const;
    int isA( contained_type ) const;

  private:
    union {
      tagIParams IParams;
      tagFParams FParams;
      char SParam[16];
      XEvent *Xevent;
    };
    contained_type type;
};

inline OXWEvent::OXWEvent()
{
  memset( this, 0, sizeof( OXWEvent ) );
}

inline OXWEvent::OXWEvent( unsigned long msg )
{
  Message = msg;
  type = IntegerParams;
  memset( &IParams, 0, sizeof( unsigned long ) * 4 );
}

inline OXWEvent::OXWEvent( unsigned long msg, const tagIParams& ip )
{
  Message = msg;
  type = IntegerParams;
  memcpy( &IParams, &ip, sizeof( unsigned long ) * 4 );
}

inline OXWEvent::OXWEvent( unsigned long msg, const tagFParams& fp )
{
  Message = msg;
  type = FloatParams;
  FParams.XParam = fp.XParam;
  FParams.YParam = fp.YParam;
}

inline OXWEvent::OXWEvent( unsigned long msg, XEvent& event )
{
  Message = msg;
  type = XEventParam;
  Xevent = &event;
}

inline OXWEvent::OXWEvent( XEvent& event )
{
  Message = event.type;
  type = XEventParam;
  Xevent = &event;
}

inline void OXWEvent::SetMessage( unsigned long msg )
{
  Message = msg;
  type = IntegerParams;
  memset( &IParams, 0, sizeof( unsigned long ) * 4 );
}

inline void OXWEvent::SetIParams( unsigned long msg, const tagIParams& ip )
{
  Message = msg;
  type = IntegerParams;
  memcpy( &IParams, &ip, sizeof( unsigned long ) * 4 );
}

inline void OXWEvent::SetFParams( unsigned long msg, const tagFParams& fp )
{
  Message = msg;
  type = FloatParams;
  FParams.XParam = fp.XParam;
  FParams.YParam = fp.YParam;
}

inline void OXWEvent::SetXEvent( unsigned long msg, XEvent& event )
{
  Message = msg;
  type = XEventParam;
  Xevent = &event;
}

inline void OXWEvent::SetXEvent( XEvent& event )
{
  Message = event.type;
  type = XEventParam;
  Xevent = &event;
}

inline tagIParams& OXWEvent::GetIParams()
{
  PRECONDITION( type == IntegerParams );
  return IParams;
}

inline tagFParams& OXWEvent::GetFParams()
{
  PRECONDITION( type == FloatParams );
  return FParams;
}

inline char *OXWEvent::GetSParam()
{
  PRECONDITION( type == StringParam );
  return SParam;
}

inline XEvent *OXWEvent::GetXEvent()
{
  PRECONDITION( type == XEventParam );
  return Xevent;
}

inline OXWEvent::contained_type OXWEvent::isA() const
{
  return type;
}

inline int OXWEvent::isA( OXWEvent::contained_type tstType ) const
{
  return tstType == type;
}

#endif
