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

template <class _InputIter1, class _InputIter2, class _OutputIter,
          class _Compare>
_OutputIter vt_union(_InputIter1 __first1, _InputIter1 __last1,
		     _InputIter2 __first2, _InputIter2 __last2,
		     _OutputIter __result, _Equal __equal, _Aggregate __aggr )
{
  
  while ( __first1 != __last1 ) {
    _InputIter1 __cur1 = __first1;
    for ( ; __cur1 != __last1, ++__cur1 ) {
      if ( __equal(*__cur1, *__first2) ) {
	__aggr( __cur1, __first2, __result );	
	++__first2;
	break;
      }
      ++__result;
    }
    if ( __cur1 == __last1 ) {
      
    }
  }
  
  while (__first1 != __last1 && __first2 != __last2) {
    if (__comp(*__first1, *__first2)) {
      *__result = *__first1;
      ++__first1;
    } else if (__comp(*__first2, *__first1)) {
      *__result = *__first2;
      ++__first2;
    } else {
      *__result = *__first1;
      ++__first1;
      ++__first2;
    }
    ++__result;
  }
  return copy(__first2, __last2, copy(__first1, __last1, __result));
}

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
    {
      vt.clear();
      std::copy( _vt.vt.begin(), _vt.vt.end(), std::back_insert_iterator<vtime_type>(vt) );
    }

  bool operator ==( const vtime& r ) const
    { return vt == r.vt; }

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
  
  vtime& operator +=( const vtime_proc_type& );
    
  vtime_type vt;
};

// typedef std::pair<group_type, vtime> vtime_group_type;
// typedef std::list<vtime_group_type> gvtime_type;
typedef std::hash_map<group_type, vtime_type> gvtime_type;

vtime_unit_type comp( const gvtime_type&, group_type, oid_type );
gvtime_type& operator +=( gvtime_type&, const vtime_group_type& );

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
    {
      gvt.clear();
      std::copy( _gvt.gvt.begin(), _gvt.gvt.end(), std::back_insert_iterator<gvtime_type>(gvt) );
    }

  gvtime& operator +=( const vtime_group_type& );

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

    void mess( const stem::Event_base<VTmess>& );

  private:

  bool order_correct( const stem::Event_base<VTmess>& );

    enum vtgroup {
      first_group,
      second_group,
      n_groups
    };

    // vtime_type vt[n_groups];
    gvtime_type lvt;
    gvtime_type vt;
    // vtime_type last_vt[n_groups];
    // gvtime_type last_vt;

    DECLARE_RESPONSE_TABLE( Proc, stem::EventHandler );
};

#define MESS 0x300

} // namespace vt

namespace std {

ostream& operator <<( ostream&, const vt::vtime_proc_type& );
ostream& operator <<( ostream&, const vt::vtime_type& );
ostream& operator <<( ostream&, const vt::vtime_group_type& );
ostream& operator <<( ostream&, const vt::gvtime_type& );

} // namespace std

#endif // __vtime_h
