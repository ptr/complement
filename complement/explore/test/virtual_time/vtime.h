// -*- C++ -*- Time-stamp: <07/05/18 00:26:00 ptr>

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

inline bool uorder( const vtime_proc_type& l, const vtime_proc_type& r )
{
  return l.first < r.first;
}

inline bool operator <( const vtime_proc_type& l, const vtime_proc_type& r )
{
  if ( l.first == r.first ) {
    return l.second < r.second;
  }

  throw std::invalid_argument( "uncomparable vtime" );
}

inline bool operator <=( const vtime_proc_type& l, const vtime_proc_type& r )
{
  if ( l.first == r.first ) {
    return l.second <= r.second;
  }

  throw std::invalid_argument( "uncomparable vtime" );
}

inline bool operator ==( const vtime_proc_type& l, const vtime_proc_type& r )
{
  if ( l.first == r.first ) {
    return l.second == r.second;
  }

  throw std::invalid_argument( "uncomparable vtime" );
}


#if 0
bool operator <=( const vtime_type& l, const vtime_type& r )
{
  if ( l.size() == 0 ) { // 0 always less or equal of anything
    return true;
  }

  bool result = true;
  vtime_type::const_iterator i = l.begin();
  vtime_type::const_iterator j = r.begin();

  while ( j->first < i->first && j != r.end() ) {
    ++j;
  }

  if ( j == r.end() ) { // note, that i != l.end() here!
    return false;
  }

  for ( ; i != l.end() && j != r.end(); ) {
    if ( i->second > j->second ) {
    }
    
    if ( i->first < j->first ) {
      ++i;
    } else if ( i->first == j->first ) {
      if ( i->second > j->second ) {
        return false;
      }
    } else {
      ++j;
    }
  }
}
#endif


vtime_type operator -( const vtime_type& l, const vtime_type& r );

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
