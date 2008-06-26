// -*- C++ -*- Time-stamp: <07/08/23 12:36:32 ptr>

#ifndef __vshostmgr_h
#define __vshostmgr_h

#include <janus/vtime.h>

#include <stem/NetTransport.h>
#include <stem/Event.h>
#include <sockios/sockmgr.h>

#include <list>

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

class VSHostMgr :
    public janus::VTHandler
{
  private:
    // typedef std::list<stem::gaddr_type> vshost_container_t;
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_set<stem::gaddr_type> vshost_container_t;
    typedef std::tr1::unordered_set<std::string> vs_wellknown_hosts_container_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_set<stem::gaddr_type> vshost_container_t;
    typedef __gnu_cxx::hash_set<std::string> vs_wellknown_hosts_container_t;
#endif
#ifdef __USE_STLPORT_HASH
    typedef std::hash_set<stem::gaddr_type> vshost_container_t;
    typedef std::hash_set<std::string> vs_wellknown_hosts_container_t;
#endif

  public:
    typedef vshost_container_t::size_type size_type;

    VSHostMgr();
    VSHostMgr( stem::addr_type id, const char *info  = 0 );
    VSHostMgr( const char *info );
    ~VSHostMgr();

    // void handler( const stem::Event& );
    void VSNewMember( const stem::Event_base<VSsync_rq>& );
    void VSOutMember( const stem::Event_base<VSsync_rq>& );
    void VSsync_time( const stem::Event_base<VSsync>& ev );

    static void add_wellknown( const char *nm );
    static void add_wellknown( const std::string& nm );
    static void rm_wellknown( const char *nm );
    static void rm_wellknown( const std::string& nm );
    static void add_srvport( int );

    int connect( const char *, int );
    int serve( int );

    size_type vs_known_processes() const
       {
         xmt::recursive_scoped_lock lk( this->_theHistory_lock );
         size_type tmp = vshost.size();
         return tmp;
       }

    void Subscribe( stem::addr_type, group_type );

  private:
    typedef std::list<stem::NetTransportMgr *> nmgr_container_t;
    typedef std::list<std::sockmgr_stream_MP<stem::NetTransport> *> srv_container_t;

    vshost_container_t vshost;
    nmgr_container_t _clients;
    srv_container_t  _servers;

    class Finalizer :
        public stem::EventHandler
    {
      public:
        Finalizer( const char *info ) :
          EventHandler( info )
          { cnd.set( false ); }

        void wait()
          { cnd.try_wait(); }

      private:
        void final()
          { cnd.set( true ); }

        xmt::condition cnd;

        DECLARE_RESPONSE_TABLE( Finalizer, stem::EventHandler );
    };

    Finalizer finalizer;

    static xmt::mutex _wknh_lock;
    static vs_wellknown_hosts_container_t _wknhosts;
    static int _srvport;

    // DECLARE_RESPONSE_TABLE( VSHostMgr, janus::VTHandler );
};

} // namespace janus

#endif // __vshostmgr_h
