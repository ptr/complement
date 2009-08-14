// -*- C++ -*- Time-stamp: <09/05/09 00:40:32 ptr>

#ifndef __vshostmgr_h
#define __vshostmgr_h

#include <janus/vtime.h>

#include <mt/condition_variable>
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
    typedef std::tr1::unordered_set<stem::addr_type> vshost_container_t;
    typedef std::tr1::unordered_set<std::string> vs_wellknown_hosts_container_t;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_set<stem::addr_type> vshost_container_t;
    typedef __gnu_cxx::hash_set<std::string> vs_wellknown_hosts_container_t;
#endif
#ifdef __USE_STLPORT_HASH
    typedef std::hash_set<stem::addr_type> vshost_container_t;
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
         std::tr2::lock_guard<std::tr2::recursive_mutex> lk( this->_theHistory_lock );
         size_type tmp = vshost.size();
         return tmp;
       }

    void Subscribe( stem::addr_type, group_type );

  private:
    typedef std::list<stem::NetTransportMgr *> nmgr_container_t;
    typedef std::list<std::connect_processor<stem::NetTransport> *> srv_container_t;

    vshost_container_t vshost;
    nmgr_container_t _clients;
    srv_container_t  _servers;

    class Finalizer :
        public stem::EventHandler
    {
      private:
        struct _not_stop
        {
            _not_stop( Finalizer& m ) :
                me( m )
              { }

            bool operator()() const
              { return me._final; }

            Finalizer& me;
        } not_stop;

      public:
        Finalizer( const char *info ) :
            EventHandler( info ),
            not_stop( *this ),
            _final( false )
          { }

        void wait()
          {
            std::tr2::unique_lock<std::tr2::mutex> lk(_lock);
            _cnd.wait( lk, not_stop );
          }

      private:
        void final()
          {
            std::tr2::lock_guard<std::tr2::mutex> lk(_lock);
            _final = true;
            _cnd.notify_one();
          }

        std::tr2::mutex _lock;
        std::tr2::condition_variable _cnd;
        bool _final;

        DECLARE_RESPONSE_TABLE( Finalizer, stem::EventHandler );
    };

    Finalizer finalizer;

    static std::tr2::mutex _wknh_lock;
    static vs_wellknown_hosts_container_t _wknhosts;
    static int _srvport;

    // DECLARE_RESPONSE_TABLE( VSHostMgr, janus::VTHandler );
};

} // namespace janus

#endif // __vshostmgr_h
