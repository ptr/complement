// -*- C++ -*- Time-stamp: <99/06/04 12:55:26 ptr>

#ifndef __SessionMgr_h
#define __SessionMgr_h

#ident "$SunId$ %Q%"

#include <Event.h>
#include <EventHandler.h>
#include <ctime>
#include <EvPack.h>

namespace EDS {

class SessionMgr :
    public EventHandler
{
  public:
    typedef unsigned key_type;

    SessionMgr() :
        EventHandler()
      { }

     SessionMgr( Event::key_type addr ) :
        EventHandler( addr )
      { }

  protected:

    virtual EventHandler *session_leader( const string& account, const string& passwd ) throw() = 0;
    virtual void destroy_session_leader( EventHandler * ) throw() = 0;

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
    select1st<account_type> _skey;
    select2nd<account_type> _sess;

    DECLARE_RESPONSE_TABLE( SessionMgr, EventHandler );
};


struct SessionRsp :
   public __pack_base
{
    SessionMgr::key_type key;
    Event::key_type      addr;

    virtual void pack( std::ostream& s ) const;
    virtual void net_pack( std::ostream& s ) const;
    virtual void unpack( std::istream& s );
    virtual void net_unpack( std::istream& s );
};

} // namespace EDS

#endif // __SessionMgr_h
