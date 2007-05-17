// -*- C++ -*- Time-stamp: <07/03/06 19:23:37 ptr>

#ifndef __vtime_h
#define __vtime_h

#include <algorithm>
#include <list>

#include <istream>
#include <ostream>
#include <stdexcept>

#include <stem/Event.h>
#include <stem/EventHandler.h>

namespace vt {

typedef unsigned vtime_unit_type;
typedef std::pair<stem::addr_type, vtime_unit_type> vtime_proc_type;
typedef std::list<vtime_proc_type> vtime_type;

inline bool operator <( const vtime_proc_type& l, const vtime_proc_type& r )
{
  if ( l.first == r.first ) {
    return l.second < r.second;
  }

  throw std::invalid_argument( "uncomparable" );
}

inline bool operator <=( const vtime_proc_type& l, const vtime_proc_type& r )
{
  if ( l.first == r.first ) {
    return l.second <= r.second;
  }

  throw std::invalid_argument( "uncomparable" );
}

inline bool operator ==( const vtime_proc_type& l, const vtime_proc_type& r )
{
  if ( l.first == r.first ) {
    return l.second == r.second;
  }

  throw std::invalid_argument( "uncomparable" );
}


struct vtime :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const;
    void net_pack( std::ostream& s ) const;
    void unpack( std::istream& s );
    void net_unpack( std::istream& s );

  vtime()
    { }
  vtime( const vtime& _vt ) :
    vt( _vt.vt.begin(), _vt.vt.end() )
    { }

  vtime& operator =( const vtime& _vt )
  { vt.clear(); }

  vtime_type vt;
};

class Proc :
  public stem::EventHandler
{
  public:
    Proc()
      { }
    Proc( stem::addr_type id ) :
        stem::EventHandler( id )
      { }

    void mess( const stem::Event_base<vtime>& );

  private:
    DECLARE_RESPONSE_TABLE( Proc, stem::EventHandler );
};

#define MESS 0x300

} // namespace vt

#endif // __vtime_h
