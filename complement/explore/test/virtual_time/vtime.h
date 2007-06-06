// -*- C++ -*- Time-stamp: <07/05/18 00:26:00 ptr>

#ifndef __vtime_h
#define __vtime_h

#include <algorithm>
#include <list>
#include <vector>
#include <hash_map>
#include <iterator>

#include <istream>
#include <ostream>
#include <stdexcept>

#include <stem/Event.h>
#include <stem/EventHandler.h>

namespace vt {

typedef stem::addr_type oid_type;
typedef unsigned vtime_unit_type;
typedef uint32_t group_type;
typedef std::hash_map<oid_type, vtime_unit_type> vtime_type;

// typedef std::pair<oid_type, vtime_unit_type> vtime_proc_type;
// typedef std::list<vtime_proc_type> vtime_type;

bool operator <=( const vtime_type& l, const vtime_type& r );
vtime_type operator -( const vtime_type& l, const vtime_type& r );
vtime_type operator +( const vtime_type& l, const vtime_type& r );
vtime_type& operator +=( vtime_type& l, const vtime_type& r );

vtime_type max( const vtime_type& l, const vtime_type& r );

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
  vtime( const vtime_type& _vt ) :
    vt( _vt.begin(), _vt.end() )
    { }

  vtime& operator =( const vtime& _vt )
    { vt = _vt.vt; }

  // bool operator ==( const vtime& r ) const
  //   { return vt == r.vt; }

  bool operator <=( const vtime& r ) const
    { return vt <= r.vt; }

  vtime operator -( const vtime& r ) const
    { return vtime( vt - r.vt ); }

  vtime operator +( const vtime& r ) const
    { return vtime( vt + r.vt ); }

  vtime& operator +=( const vtime_type& t )
  {
    vt += t;
    return *this;
  }
  vtime& operator +=( const vtime& t )
  {
    vt += t.vt;
    return *this;
  }
  
  vtime& operator +=( const vtime_type::value_type& );

  vtime_type::data_type& operator[]( const vtime_type::key_type k )
  { return vt[k]; }
    
  vtime_type vt;
};


vtime max( const vtime& l, const vtime& r );

// typedef std::pair<group_type, vtime> vtime_group_type;
// typedef std::list<vtime_group_type> gvtime_type;
typedef std::hash_map<group_type, vtime> gvtime_type;

gvtime_type& operator +=( gvtime_type&, const gvtime_type::value_type& );
gvtime_type& operator +=( gvtime_type&, const gvtime_type& );
gvtime_type operator -( const gvtime_type& l, const gvtime_type& r );

struct gvtime :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const;
    void net_pack( std::ostream& s ) const;
    void unpack( std::istream& s );
    void net_unpack( std::istream& s );

  gvtime()
    { }
  gvtime( const gvtime& _gvt ) :
    gvt( _gvt.gvt.begin(), _gvt.gvt.end() )
    { }

  gvtime& operator =( const gvtime& _gvt )
    { gvt = _gvt.gvt; }

  gvtime& operator +=( const gvtime_type::value_type& );
  gvtime& operator +=( const gvtime& );

  gvtime_type::data_type& operator[]( const gvtime_type::key_type k )
    { return gvt[k]; }

  gvtime_type gvt;
};

struct VTmess :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const;
    void net_pack( std::ostream& s ) const;
    void unpack( std::istream& s );
    void net_unpack( std::istream& s );

  VTmess()
    { }
  VTmess( const VTmess& _gvt ) :
    gvt( _gvt.gvt ),
    mess( _gvt.mess )
    { }

  gvtime gvt;
  group_type grp;
  std::string mess;
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

    void add_group( group_type g )
      { groups.push_back( g ); }

    void add_group_member( group_type g, oid_type p )
      { vt[g][p]; }

    void SendVC( group_type, const std::string& mess );

    void mess( const stem::Event_base<VTmess>& );

    typedef std::list<group_type> groups_container_type;

  private:

    bool order_correct( const stem::Event_base<VTmess>& );

    typedef std::hash_map<oid_type, gvtime_type> delta_vtime_type;

    delta_vtime_type lvt;
    gvtime vt;
    groups_container_type groups;


    DECLARE_RESPONSE_TABLE( Proc, stem::EventHandler );
};

#define MESS 0x300

} // namespace vt

namespace std {

ostream& operator <<( ostream&, const vt::vtime_type::value_type& );
ostream& operator <<( ostream&, const vt::vtime_type& );
ostream& operator <<( ostream&, const vt::vtime& );
ostream& operator <<( ostream&, const vt::gvtime_type::value_type& );
ostream& operator <<( ostream&, const vt::gvtime_type& );

} // namespace std

#endif // __vtime_h
