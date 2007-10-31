// -*- C++ -*- Time-stamp: <07/03/06 19:23:37 ptr>

#ifndef __intercessor_h
#define __intercessor_h

#include <list>
#include <deque>
#include <mt/xmt.h>
#include <net/http.h>
#include <stem/Event.h>
#include <stem/EventHandler.h>

namespace intr {

struct httprq :
    public stem::__pack_base
{
    httprq()
      { }

    void pack( std::ostream& s ) const;
    void net_pack( std::ostream& s ) const;
    void unpack( std::istream& s );
    void net_unpack( std::istream& s );

    http::request rq;
};

struct httprs :
    public stem::__pack_base
{
    httprs()
      { }

    void pack( std::ostream& s ) const;
    void net_pack( std::ostream& s ) const;
    void unpack( std::istream& s );
    void net_unpack( std::istream& s );

    http::response rs;
};

class Intercessor :
    public stem::EventHandler
{
  public:
    Intercessor()
      { }

    Intercessor( stem::addr_type id ) :
        stem::EventHandler( id )
      { }

    void request( const stem::Event_base<intr::httprq>& );

  private:
    DECLARE_RESPONSE_TABLE( Intercessor, stem::EventHandler );
};

#define RESPONCE_OK  0x700
#define RESPONCE_NOK 0x701
#define RESPONCE_UNP 0x703

#define INTR_RQ      0x702

} // namespace intr

#endif // __intercessor_h
