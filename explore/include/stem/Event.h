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
  };
};

struct tagFParams
{
  double XParam;
  double YParam;
};

class ASEvent
{
  public:
    enum contained_type {
      IntegerParams,
      FloatParams,
      StringParam,
      XEventParam
    };

    unsigned long Message;

    ASEvent();
    ASEvent( unsigned long );
    ASEvent( unsigned long, const tagIParams& );
    ASEvent( unsigned long, const tagFParams& );
    ASEvent( unsigned long, const char * );
    ASEvent( unsigned long, XEvent& );
    ASEvent( XEvent& );

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

inline ASEvent::ASEvent()
{
  Message = 0;
  type = IntegerParams;
  IParams.AParam = 0;
  IParams.BParam = 0;
  IParams.CParam = 0;
  IParams.DParam = 0;
}

inline ASEvent::ASEvent( unsigned long msg )
{
  Message = msg;
  type = IntegerParams;
  IParams.AParam = 0;
  IParams.BParam = 0;
  IParams.CParam = 0;
  IParams.DParam = 0;
}

inline ASEvent::ASEvent( unsigned long msg, const tagIParams& ip )
{
  Message = msg;
  type = IntegerParams;
  IParams.AParam = ip.AParam;
  IParams.BParam = ip.BParam;
  IParams.CParam = ip.CParam;
  IParams.DParam = ip.DParam;
}

inline ASEvent::ASEvent( unsigned long msg, const tagFParams& fp )
{
  Message = msg;
  type = FloatParams;
  FParams.XParam = fp.XParam;
  FParams.YParam = fp.YParam;
}

inline ASEvent::ASEvent( unsigned long msg, XEvent& event )
{
  Message = msg;
  type = XEventParam;
  Xevent = &event;
}

inline ASEvent::ASEvent( XEvent& event )
{
  Message = event.type;
  type = XEventParam;
  Xevent = &event;
}

inline void ASEvent::SetMessage( unsigned long msg )
{
  Message = msg;
  type = IntegerParams;
  IParams.AParam = 0;
  IParams.BParam = 0;
  IParams.CParam = 0;
  IParams.DParam = 0;
}

inline void ASEvent::SetIParams( unsigned long msg, const tagIParams& ip )
{
  Message = msg;
  type = IntegerParams;
  IParams.AParam = ip.AParam;
  IParams.BParam = ip.BParam;
  IParams.CParam = ip.CParam;
  IParams.DParam = ip.DParam;
}

inline void ASEvent::SetFParams( unsigned long msg, const tagFParams& fp )
{
  Message = msg;
  type = FloatParams;
  FParams.XParam = fp.XParam;
  FParams.YParam = fp.YParam;
}

inline void ASEvent::SetXEvent( unsigned long msg, XEvent& event )
{
  Message = msg;
  type = XEventParam;
  Xevent = &event;
}

inline void ASEvent::SetXEvent( XEvent& event )
{
  Message = event.type;
  type = XEventParam;
  Xevent = &event;
}

inline tagIParams& ASEvent::GetIParams()
{
  PRECONDITION( type == IntegerParams );
  return IParams;
}

inline tagFParams& ASEvent::GetFParams()
{
  PRECONDITION( type == FloatParams );
  return FParams;
}

inline char *ASEvent::GetSParam()
{
  PRECONDITION( type == StringParam );
  return SParam;
}

inline XEvent *ASEvent::GetXEvent()
{
  PRECONDITION( type == XEventParam );
  return Xevent;
}

inline ASEvent::contained_type ASEvent::isA() const
{
  return type;
}

inline int ASEvent::isA( ASEvent::contained_type tstType ) const
{
  return tstType == type;
}

#endif
