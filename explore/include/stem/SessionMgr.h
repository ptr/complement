// -*- C++ -*- Time-stamp: <01/03/19 18:56:39 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

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

namespace EDS {

class SessionMgr :
    public EventHandler
{
  public:
    __PG_DECLSPEC SessionMgr();
    __PG_DECLSPEC SessionMgr( const char *info );
    __PG_DECLSPEC SessionMgr( addr_type addr, const char *info = 0 );
    __PG_DECLSPEC ~SessionMgr();
   
    void raw_establish_session( EventHandler *, addr_type );

  protected:

    virtual __PG_DECLSPEC EventHandler *session_leader( const std::string& account,
                                                    const std::string& passwd,
                                                    addr_type addr ) throw();
    virtual __PG_DECLSPEC void destroy_session_leader( EventHandler * ) throw();

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

    virtual __PG_DECLSPEC void pack( std::ostream& s ) const;
    virtual __PG_DECLSPEC void net_pack( std::ostream& s ) const;
    virtual __PG_DECLSPEC void unpack( std::istream& s );
    virtual __PG_DECLSPEC void net_unpack( std::istream& s );
};

} // namespace EDS

#endif // __SessionMgr_h
