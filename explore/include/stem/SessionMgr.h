// -*- C++ -*- Time-stamp: <06/06/28 15:09:44 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 2.1
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

#ifndef __stem_SessionMgr_h
#define __stem_SessionMgr_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifndef __stem_Event_h
#include <stem/Event.h>
#endif

#ifndef __stem_EventHandler_h
#include <stem/EventHandler.h>
#endif

#include <ctime>

#ifndef __stem_EvPack_h
#include <stem/EvPack.h>
#endif

namespace stem {

class SessionMgr :
    public EventHandler
{
  public:
    __FIT_DECLSPEC SessionMgr();
    __FIT_DECLSPEC SessionMgr( const char *info );
    __FIT_DECLSPEC SessionMgr( addr_type addr, const char *info = 0 );
    __FIT_DECLSPEC ~SessionMgr();
   
    void raw_establish_session( EventHandler *, addr_type );

  protected:

    virtual __FIT_DECLSPEC EventHandler *session_leader( const std::string& account,
                                                    const std::string& passwd,
                                                    addr_type addr ) throw();
    virtual __FIT_DECLSPEC void destroy_session_leader( EventHandler * ) throw();

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
#ifdef STLPORT
    std::select1st<account_type> _skey;
    std::select2nd<account_type> _sess;
#else
    __gnu_cxx::select1st<account_type> _skey;
    __gnu_cxx::select2nd<account_type> _sess;
#endif
    std::equal_to<key_type>      _eq_key;

    DECLARE_RESPONSE_TABLE( SessionMgr, EventHandler );
};


struct SessionRsp :
   public __pack_base
{
    key_type  key;
    addr_type addr;

    virtual __FIT_DECLSPEC void pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void net_pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void unpack( std::istream& s );
    virtual __FIT_DECLSPEC void net_unpack( std::istream& s );
};

} // namespace stem

#endif // __stem_SessionMgr_h
