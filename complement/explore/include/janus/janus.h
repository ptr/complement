// -*- C++ -*- Time-stamp: <07/08/25 01:40:20 ptr>

#ifndef __janus_h
#define __janus_h

#include <ostream>
#include <iterator>

#include <mt/xmt.h>
#include <stem/Event.h>
#include <stem/EventHandler.h>

#include <janus/vtime.h>

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

namespace janus {

class Janus :
        public stem::EventHandler,
        public vs_base
{
  private:
#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<oid_type, detail::vtime_obj_rec> vt_map_type;
    typedef std::hash_multimap<group_type, oid_type> gid_map_type;
    typedef std::hash_set<stem::addr_type> addr_cache_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<oid_type, detail::vtime_obj_rec> vt_map_type;
    typedef __gnu_cxx::hash_multimap<group_type, oid_type> gid_map_type;
    typedef __gnu_cxx::hash_set<stem::addr_type> addr_cache_t;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<oid_type, detail::vtime_obj_rec> vt_map_type;
    typedef std::tr1::unordered_multimap<group_type, oid_type> gid_map_type;
    typedef std::tr1::unordered_set<stem::addr_type> addr_cache_t;
#endif

  public:
    typedef std::iterator_traits<gid_map_type::iterator>::difference_type difference_type;

    enum traceflags {
      notrace = 0,
      tracenet = 1,
      tracedispatch = 2,
      tracefault = 4,
      tracedelayed = 8,
      tracegroup = 0x10
    };

    Janus() :
        _trflags( notrace ),
        _trs( 0 ),
        _hostmgr( 0 )
      { }

    Janus( const char *info ) :
        stem::EventHandler( info ),
        _trflags( notrace ),
        _trs( 0 ),
        _hostmgr( 0 )
      { }

    Janus( stem::addr_type id ) :
        stem::EventHandler( id ),
        _trflags( notrace ),
        _trs( 0 ),
        _hostmgr( 0 )
      { }

    Janus( stem::addr_type id, const char *info ) :
        stem::EventHandler( id, info ),
        _trflags( notrace ),
        _trs( 0 ),
        _hostmgr( 0 )
      { }

    ~Janus();

    void JaSend( const stem::Event& e, group_type );

    void settrf( unsigned f );
    void unsettrf( unsigned f );
    void resettrf( unsigned f );
    void cleantrf();
    unsigned trflags() const;
    void settrs( std::ostream * );

    void connect( const char *, int );
    void serve( int );
    size_t vs_known_processes() const;

    difference_type group_size( group_type ) const;

  private:
    void Subscribe( stem::addr_type, const oid_type&, group_type );
    void Unsubscribe( const oid_type&, group_type );
    void Unsubscribe( const oid_type& );

    void get_gvtime( group_type, stem::addr_type, gvtime_type& );
    void set_gvtime( group_type, stem::addr_type, const gvtime_type& );
    void check_and_send( detail::vtime_obj_rec&, const stem::Event_base<VSmess>& );
    void check_and_send_delayed( detail::vtime_obj_rec& );
    
    vt_map_type vtmap;
    gid_map_type grmap;

    xmt::mutex _lock_tr;
    unsigned _trflags;
    std::ostream *_trs;

  protected:

    class VSHostMgr *_hostmgr;

    friend class VTHandler::Init;
    friend class VTHandler;
    friend class VSHostMgr;

#ifdef __FIT_EXAM
    friend class vtime_operations;
#endif

  private:

    void JaDispatch( const stem::Event_base<VSmess>& );
    void VSNewMember( const stem::Event_base<VSsync_rq>& e );
    void VSNewRemoteMemberDirect( const stem::Event_base<VSsync_rq>& e );
    void VSNewRemoteMemberRevert( const stem::Event_base<VSsync_rq>& e );
    void VSOutMember( const stem::Event_base<VSsync_rq>& e );

    DECLARE_RESPONSE_TABLE( Janus, stem::EventHandler );
};

} // namespace janus

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

#endif // __janus_h
