// -*- C++ -*- Time-stamp: <99/12/22 15:01:18 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifndef __SessionMgr_h
#define __SessionMgr_h

#ident "$SunId$ %Q%"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifndef __Event_h
#include <EDS/Event.h>
#endif

#ifndef __EventHandler_h
#include <EDS/EventHandler.h>
#endif

#include <ctime>

#ifndef __EvPack_h
#include <EDS/EvPack.h>
#endif

#ifndef __EDS_DLL
#  if defined( WIN32 ) && defined( _MSC_VER )
#    define __EDS_DLL __declspec( dllimport )
#  else
#    define __EDS_DLL
#  endif
#endif

#if defined( WIN32 ) && defined( _MSC_VER )
#  undef __EDS_DLL_EXPORT
#  define __EDS_DLL_EXPORT __EDS_DLL
#endif

namespace EDS {

class SessionMgr :
    public EventHandler
{
  public:
    SessionMgr() :
        EventHandler()
      { }

    SessionMgr( const char *info ) :
        EventHandler( info )
      { }

     SessionMgr( addr_type addr, const char *info = 0 ) :
        EventHandler( addr, info )
      { }

    void raw_establish_session( EventHandler *, addr_type );

  protected:

    virtual __EDS_DLL EventHandler *session_leader( const std::string& account,
                                                    const std::string& passwd,
                                                    addr_type addr ) throw();
    virtual __EDS_DLL void destroy_session_leader( EventHandler * ) throw();

    void establish_session( const Event& );
    void restore_session( const Event_base<key_type>& );
    void close_session( const Event_base<key_type>& );

    struct __S
    {
        // Event::key_type leader;
        EventHandler *leader;
        time_t timeout;
    };

    typedef std::pair<key_type,__S> account_type;
    typedef std::vector<account_type> Container;

    key_type key_generate();
    Container _M_c;
    std::select1st<account_type> _skey;
    std::select2nd<account_type> _sess;
    std::equal_to<key_type>      _eq_key;

    DECLARE_RESPONSE_TABLE( SessionMgr, EventHandler );
};


struct SessionRsp :
   public __pack_base
{
    key_type  key;
    addr_type addr;

    virtual __EDS_DLL void pack( std::ostream& s ) const;
    virtual __EDS_DLL void net_pack( std::ostream& s ) const;
    virtual __EDS_DLL void unpack( std::istream& s );
    virtual __EDS_DLL void net_unpack( std::istream& s );
};

} // namespace EDS

#if defined( WIN32 ) && defined( _MSC_VER )
#  undef __EDS_DLL_EXPORT
#  define __EDS_DLL_EXPORT
#endif

#endif // __SessionMgr_h
