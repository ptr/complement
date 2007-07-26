// -*- C++ -*- Time-stamp: <07/07/25 23:30:52 ptr>

#ifndef __vtime_h
#define __vtime_h

#include <algorithm>
#include <list>
#include <vector>
#include <hash_map>
#include <hash_set>
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

bool operator <=( const vtime_type& l, const vtime_type& r );
inline bool operator >=( const vtime_type& l, const vtime_type& r )
  { return r <= l; }
vtime_type operator -( const vtime_type& l, const vtime_type& r );
vtime_type operator +( const vtime_type& l, const vtime_type& r );
vtime_type& operator +=( vtime_type& l, const vtime_type& r );

// vt::vtime_type max( const vt::vtime_type& l, const vt::vtime_type& r );
vtime_type& sup( vtime_type& l, const vtime_type& r );

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
  bool operator >=( const vtime& r ) const
    { return vt >= r.vt; }

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


// vtime max( const vtime& l, const vtime& r );
vtime& sup( vtime& l, const vtime& r );

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

    VTmess() :
        code(0),
        src(0),
        gvt(),
        grp(0),
        mess()
      { }
    VTmess( const VTmess& _gvt ) :
        code( _gvt.code ),
        src( _gvt.src ),
        gvt( _gvt.gvt ),
        grp( _gvt.grp ),
        mess( _gvt.mess )
      { }

    stem::code_type code;
    oid_type src;    
    gvtime gvt;
    group_type grp;
    std::string mess;
};

namespace detail {

class vtime_obj_rec
{
  public:

    typedef std::hash_set<group_type> groups_container_type;
    typedef std::hash_map<oid_type, gvtime_type> delta_vtime_type;
    typedef std::hash_map<group_type, gvtime_type> snd_delta_vtime_t;

    void add_group( group_type g )
      { groups.insert(g); }

    // void add_group_member( group_type g, oid_type p )
    //   { vt[g][p]; }

    bool deliver( const VTmess& ev );
    bool deliver_delayed( const VTmess& ev );

    stem::addr_type addr;  // stem address of object
    delta_vtime_type lvt;  // last recieve VT from neighbours
    snd_delta_vtime_t svt; // last send VT to neighbours
    gvtime vt;             // VT of object

  private:
    groups_container_type groups; // member of groups

  public:
    // delay pool should be here
    typedef std::pair<int,stem::Event_base<VTmess>*> delay_item_t;
    typedef std::list<delay_item_t> dpool_t;

    dpool_t dpool;

  private:
    bool order_correct( const VTmess& );
    bool order_correct_delayed( const VTmess& );
};

} // namespace detail

class VTDispatcher :
        public stem::EventHandler
{
  public:
    VTDispatcher()
      { }

    VTDispatcher( stem::addr_type id ) :
        stem::EventHandler( id )
      { }

    void VTDispatch( const stem::Event_base<VTmess>& );

    void VTSend( const stem::Event& e, group_type );
    void Subscribe( stem::addr_type, oid_type, group_type );

  private:    

    typedef std::hash_map<oid_type, detail::vtime_obj_rec> vt_map_type;
    typedef std::hash_multimap<group_type, oid_type> gid_map_type;
    // oid_type map_gid( group_type );
    // gid_type -> (oid_type, oid_type, ...)

    // in our case we can use gid = hi bits | oid

    void VTDispatch_( const stem::Event_base<VTmess>&, const std::pair<gid_map_type::const_iterator,gid_map_type::const_iterator>& );
    
    vt_map_type vtmap;
    gid_map_type grmap;

    DECLARE_RESPONSE_TABLE( VTDispatcher, stem::EventHandler );
};

class VTHandler :
        public stem::EventHandler
{
  public:
    class Init
    {
      public:
        Init();
        ~Init();
      private:
        static void _guard( int );
        static void __at_fork_prepare();
        static void __at_fork_child();
        static void __at_fork_parent();
    };

  public:
    VTHandler();
    explicit VTHandler( const char *info );
    explicit VTHandler( stem::addr_type id, const char *info = 0 );
    virtual ~VTHandler();

    void VTSend( const stem::Event& e );

    template <class D>
    void VTSend( const stem::Event_base<D>& e )
      { VTHandler::VTSend( stem::Event_convert<D>()( e ) ); }


  private:
    static class VTDispatcher *_vtdsp;
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
