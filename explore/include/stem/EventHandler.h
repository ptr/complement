#ident "%Z%$RCSfile$ v.$Revision$ %H% %T%" // -*- C++ -*-

#ifndef __OXW_EventHandler_h
#define __OXW_EventHandler_h

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif

#ifndef __OXW_OXWdefs_h
#include <OXW/OXWdefs.h>
#endif

#ifndef __OXW_Event_h
#include <OXW/Event.h>
#endif


class GENERIC;
template <class T> class ASResponseTableEntry;  // forward declaration
typedef ASResponseTableEntry<GENERIC> ASGenericTableEntry;

class ASEventHandler
{
  public:
    class ASEventInfo
    {
      public:
        ASEventInfo( unsigned long msg, unsigned long id = 0) : 
	  Msg(msg), Id(id), Entry(0) {}

        const unsigned long Msg;
        const unsigned long Id;
        GENERIC *Object;
        ASGenericTableEntry *Entry;
    };
    typedef int(*ASEqualOperator)( ASGenericTableEntry&, ASEventInfo& );

    // Find object corresponded to event info, return non-zero in success
    // end zero otherwise. You may make influence to selection of
    // event processor by specifying equal operator.
    virtual int Find( ASEventInfo&, ASEqualOperator = 0 );

    // Dispatch event info (with message as parameter)
    virtual long Dispatch( ASEventInfo&, ASEvent& );

  protected:
    int SearchEntries( ASGenericTableEntry *, ASEventInfo&, ASEqualOperator );
};

template <class T> class ASResponseTableEntry
{
  public:
    typedef void (T::*PMF)( ASEvent& );

    union {
      unsigned long Msg;
      unsigned long NotifyCode;
    };
    unsigned long Id;
    PMF Pmf;
};

//
// macros to declare a response table
//

#define DECLARE_RESPONSE_TABLE(cls)\
  private:\
    static ASResponseTableEntry< cls > __entries[];\
    typedef cls ThisCls;\
  public:\
    int  Find( ASEventInfo&, ASEqualOperator = 0 )

#define END_RESPONSE_TABLE\
  {0, 0, 0}}

#define DEFINE_RESPONSE_TABLE_ENTRIES(cls)\
  ASResponseTableEntry< cls > cls::__entries[] = {

//
// macro to define a response table for a class with no base response tables
//
// you use it like this:
//    DEFINE_RESPONSE_TABLE(cls)
//      EV_WM_PAINT,
//      EV_WM_LBUTTONDOWN,
//    END_RESPONSE_TABLE;
//
#define DEFINE_RESPONSE_TABLE(cls)\
  int cls::Find(ASEventInfo& eventInfo, ASEqualOperator equal){\
       eventInfo.Object = (GENERIC *)this;\
       return SearchEntries((ASGenericTableEntry *)__entries, eventInfo, equal);}\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
// macro to define a response table for a class with one base. use this macro
// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE1(cls, base)\
  int cls::Find(ASEventInfo& eventInfo, ASEqualOperator equal){\
       eventInfo.Object = (GENERIC *)this;\
       return SearchEntries((ASGenericTableEntry *)__entries, eventInfo, equal) ||\
              base::Find(eventInfo, equal);}\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
// macro to define a response table for a class with two bases. use this macro
// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE2(cls, base1, base2)\
  int cls::Find(ASEventInfo& eventInfo, ASEqualOperator equal){\
       eventInfo.Object = (GENERIC *)this;\
       return SearchEntries((ASGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
              base2::Find(eventInfo, equal);}\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
// macro to define a response table for a class with three bases. use this macro
// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE3(cls, base1, base2, base3)\
  int cls::Find(ASEventInfo& eventInfo, ASEqualOperator equal)\
       eventInfo.Object = (GENERIC *)this;\
       return SearchEntries((ASGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
              base2::Find(eventInfo, equal) ||\
              base3::Find(eventInfo, equal);}\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)


#endif          // __OXW_EventHandler_h
