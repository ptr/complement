// -*- C++ -*- Time-stamp: <07/08/23 10:16:54 ptr>

#ifndef __vtime_h
#define __vtime_h

#include <algorithm>
#include <list>
#include <vector>
#ifdef STLPORT
#  include <unordered_map>
#  include <unordered_set>
// #  include <hash_map>
// #  include <hash_set>
// #  define __USE_STLPORT_HASH
#  define __USE_STLPORT_TR1
#else
#  if defined(__GNUC__) && (__GNUC__ < 4)
#    include <ext/hash_map>
#    include <ext/hash_set>
#    define __USE_STD_HASH
#  else
#    include <tr1/unordered_map>
#    include <tr1/unordered_set>
#    define __USE_STD_TR1
#  endif
#endif
#include <iterator>
#include <istream>
#include <ostream>
#include <stdexcept>

#include <stem/Event.h>
#include <stem/EventHandler.h>

#include <mt/time.h>

namespace janus {

// typedef stem::addr_type oid_type;
typedef stem::gaddr_type oid_type;

} // namespace janus

#if defined(__USE_STLPORT_HASH) || defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
#  define __HASH_NAMESPACE std
#endif
#if defined(__USE_STD_HASH)
#  define __HASH_NAMESPACE __gnu_cxx
#endif

namespace __HASH_NAMESPACE {

#ifdef __USE_STD_TR1
namespace tr1 {
#endif

template <>
struct hash<janus::oid_type>
{
    size_t operator()(const janus::oid_type& __x) const
      { return __x.addr; }
};

#ifdef __USE_STD_TR1
}
#endif

} // namespace __HASH_NAMESPACE

#undef __HASH_NAMESPACE

namespace janus {

typedef uint32_t vtime_unit_type;
typedef stem::addr_type group_type; // required, used in VTSend
#ifdef __USE_STLPORT_HASH
typedef std::hash_map<oid_type, vtime_unit_type> vtime_type;
#endif
#ifdef __USE_STD_HASH
typedef __gnu_cxx::hash_map<oid_type, vtime_unit_type> vtime_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
typedef std::tr1::unordered_map<oid_type, vtime_unit_type> vtime_type;
#endif

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

  vtime_type::mapped_type& operator[]( const vtime_type::key_type& k )
    { return vt[k]; }
  const vtime_type::mapped_type& operator[]( const vtime_type::key_type& k ) const
    { return vt[k]; }

    
  mutable vtime_type vt;
};


// vtime max( const vtime& l, const vtime& r );
vtime& sup( vtime& l, const vtime& r );

// typedef std::pair<group_type, vtime> vtime_group_type;
// typedef std::list<vtime_group_type> gvtime_type;
#ifdef __USE_STLPORT_HASH
typedef std::hash_map<group_type, vtime> gvtime_type;
#endif
#ifdef __USE_STD_HASH
typedef __gnu_cxx::hash_map<group_type, vtime> gvtime_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
typedef std::tr1::unordered_map<group_type, vtime> gvtime_type;
#endif

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

  gvtime_type::mapped_type& operator[]( const gvtime_type::key_type k )
    { return gvt[k]; }
  const gvtime_type::mapped_type& operator[]( const gvtime_type::key_type k ) const
    { return gvt[k]; }

  mutable gvtime_type gvt;
};

struct VSsync_rq :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const;
    void net_pack( std::ostream& s ) const;
    void unpack( std::istream& s );
    void net_unpack( std::istream& s );

    VSsync_rq() :
        grp(0),
        mess()
      { }
    VSsync_rq( const VSsync_rq& _gvt ) :
        grp( _gvt.grp ),
        mess( _gvt.mess )
      { }

    group_type grp;
    std::string mess;
};

struct VSsync :
    public VSsync_rq
{
    void pack( std::ostream& s ) const;
    void net_pack( std::ostream& s ) const;
    void unpack( std::istream& s );
    void net_unpack( std::istream& s );

    VSsync()
      { }
    VSsync( const VSsync& _gvt ) :
        VSsync_rq( _gvt ),
        gvt( _gvt.gvt )
      { }

    gvtime gvt;
};

struct VSmess :
    public VSsync
{
    void pack( std::ostream& s ) const;
    void net_pack( std::ostream& s ) const;
    void unpack( std::istream& s );
    void net_unpack( std::istream& s );

    VSmess() :
        code(0),
        src()
      { }
    VSmess( const VSmess& _gvt ) :
        VSsync( _gvt ),
        code( _gvt.code ),
        src( _gvt.src )
      { }

    stem::code_type code;
    oid_type src;    
};

namespace detail {

class vtime_obj_rec
{
  public:
    void add_group( group_type g )
      { groups.insert(g); }
    void add( stem::addr_type a, group_type g )
      { addr = a; groups.insert(g); }
    bool rm_group( group_type );
    void rm_member( const oid_type& );

    template <typename BackInsertIterator>
    void groups_list( BackInsertIterator bi ) const
      { std::copy( groups.begin(), groups.end(), bi ); }

    stem::addr_type stem_addr() const
      { return addr; }

    bool deliver( const VSmess& ev );
    bool deliver_delayed( const VSmess& ev );
    std::ostream& trace_deliver( const VSmess& m, std::ostream& o );
    void next( const oid_type& from, group_type grp )
      { ++vt.gvt[grp][from]; /* increment my VT counter */ }
    void delta( gvtime& vtstamp, const oid_type& from, const oid_type& to, group_type grp );
    void base_advance( const oid_type& to )
      { svt[to] = vt.gvt; /* store last sent VT to obj */ }
    void get_gvt( gvtime_type& gvt ) const
      { gvt = vt.gvt; }
    void sync( group_type, const oid_type&, const gvtime_type& );

#ifdef __FIT_EXAM
    const gvtime_type::mapped_type& operator[]( const gvtime_type::key_type k ) const
      { return vt[k]; }
#endif

  private:
#ifdef __USE_STLPORT_HASH
    typedef std::hash_set<group_type> groups_container_type;
    typedef std::hash_map<oid_type, gvtime_type> delta_vtime_type;
    typedef std::hash_map<oid_type, gvtime_type> snd_delta_vtime_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_set<group_type> groups_container_type;
    typedef __gnu_cxx::hash_map<oid_type, gvtime_type> delta_vtime_type;
    typedef __gnu_cxx::hash_map<oid_type, gvtime_type> snd_delta_vtime_t;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_set<group_type> groups_container_type;
    typedef std::tr1::unordered_map<oid_type, gvtime_type> delta_vtime_type;
    typedef std::tr1::unordered_map<oid_type, gvtime_type> snd_delta_vtime_t;
#endif

    stem::addr_type addr;  // stem address of object
    delta_vtime_type lvt;  // last recieve VT from neighbours
    snd_delta_vtime_t svt; // last send VT to neighbours
    gvtime vt;             // VT of object

    groups_container_type groups; // member of groups

  public:
    // delay pool should be here
    typedef std::pair<xmt::timespec,stem::Event_base<VSmess>*> delay_item_t;
    typedef std::list<delay_item_t> dpool_t;

    dpool_t dpool;

  private:
    bool order_correct( const VSmess& );
    bool order_correct_delayed( const VSmess& );
};

} // namespace detail

struct vs_base
{
  enum {
    vshosts_group = 0,
    first_user_group = 10
  };
};

class Janus;

class VTHandler :
        public stem::EventHandler,
        public vs_base
{
  private:
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

    void JaSend( const stem::Event& e );
    void JoinGroup( group_type grp );
    void LeaveGroup( group_type grp );

    virtual void VSNewMember( const stem::Event_base<VSsync_rq>& e );
    virtual void VSOutMember( const stem::Event_base<VSsync_rq>& e );
    virtual void VSsync_time( const stem::Event_base<VSsync>& );
    virtual void VSMergeRemoteGroup( const stem::Event_base<VSsync_rq>& e );

    // template <class D>
    // void JaSend( const stem::Event_base<D>& e )
    //   { VTHandler::JaSend( stem::Event_convert<D>()( e ) ); }

    static Janus *vtdispatcher()
      { return _vtdsp; }

  protected:
    void Unsubscribe();
    void VSNewMember_data( const stem::Event_base<VSsync_rq>&, const std::string& data );
    void VSMergeRemoteGroup_data( const stem::Event_base<VSsync_rq>& e, const std::string& data );


    void get_gvtime( group_type g, gvtime_type& gvt );

  private:
    static Janus *_vtdsp;
    friend class Janus;

    DECLARE_RESPONSE_TABLE( VTHandler, stem::EventHandler );
};

#define VS_MESS               0x300
#define VS_NEW_MEMBER         0x301
#define VS_OUT_MEMBER         0x302
#define VS_SYNC_TIME          0x303
#define VS_NEW_REMOTE_MEMBER  0x304
#define VS_NEW_MEMBER_RV      0x305
#define VS_OLD_MEMBER_RV      0x306
#define VS_HOST_MGR_FINAL     0x307
#define VS_MERGE_GROUP        0x308
#define VS_SYNC_GROUP_TIME    0x309

#ifdef __USE_STLPORT_HASH
#  undef __USE_STLPORT_HASH
#endif
#ifdef __USE_STD_HASH
#  undef __USE_STD_HASH
#endif
#ifdef __USE_STLPORT_TR1
#  undef __USE_STLPORT_TR1
#endif
#ifdef __USE_STD_TR1
#  undef __USE_STD_TR1
#endif

} // namespace janus

namespace std {

ostream& operator <<( ostream&, const janus::vtime_type::value_type& );
ostream& operator <<( ostream&, const janus::vtime_type& );
ostream& operator <<( ostream&, const janus::vtime& );
ostream& operator <<( ostream&, const janus::gvtime_type::value_type& );
ostream& operator <<( ostream&, const janus::gvtime_type& );
ostream& operator <<( ostream&, const janus::gvtime& );
ostream& operator <<( ostream& o, const janus::VSsync& );
ostream& operator <<( ostream& o, const janus::VSmess& );

} // namespace std

#endif // __vtime_h
