// -*- C++ -*- Time-stamp: <99/06/18 19:31:11 ptr>

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

    virtual EventHandler *session_leader( const std::string& account,
                                          const std::string& passwd,
                                          Event::key_type addr ) throw() = 0;
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
    std::select1st<account_type> _skey;
    std::select2nd<account_type> _sess;
    std::equal_to<key_type>      _eq_key;

    DECLARE_RESPONSE_TABLE( SessionMgr, EventHandler );
};


struct SessionRsp :
   public __pack_base
{
    SessionMgr::key_type key;
    Event::key_type      addr;

    virtual __DLLEXPORT void pack( std::ostream& s ) const;
    virtual __DLLEXPORT void net_pack( std::ostream& s ) const;
    virtual __DLLEXPORT void unpack( std::istream& s );
    virtual __DLLEXPORT void net_unpack( std::istream& s );
};

} // namespace EDS

#endif // __SessionMgr_h
