// -*- C++ -*- Time-stamp: <09/06/17 19:20:51 ptr>

/*
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

/*
How to make syslog stream, based on sockstream with UNIX domain
sockets.

This sample is similar to

openlog( exename, LOG_PID, LOG_USER );
...
syslog( LOG_INFO, "%s", message );
...
closelog();

 */
#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <mt/thread>
#include <sockios/sockstream>

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

namespace misc {
namespace detail {
#ifdef __USE_STLPORT_HASH
typedef std::hash_map<std::tr2::thread_base::id,std::sockstream*> log_heap_type;
#endif
#ifdef __USE_STD_HASH
typedef __gnu_cxx::hash_map<std::tr2::thread_base::id,std::sockstream*> log_heap_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
typedef std::tr1::unordered_map<std::tr2::thread_base::id,std::sockstream*> log_heap_type;
#endif
} // namespace detail
} // namespce misc

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
struct hash<std::tr2::thread_base::id>
{
    size_t operator()(const std::tr2::thread_base::id& __x) const
      { return *(size_t*)(&__x); } // hack
};

#ifdef __USE_STD_TR1
}
#endif

} // namespace __HASH_NAMESPACE

#undef __HASH_NAMESPACE

#ifdef __USE_STLPORT_TR1
#undef __USE_STLPORT_TR1
#endif

#ifdef __USE_STD_TR1
#undef __USE_STD_TR1
#endif

#ifdef __USE_STD_HASH
#undef __USE_STD_HASH
#endif

#ifdef __USE_STLPORT_HASH
#undef __USE_STLPORT_HASH
#endif

#include <sys/syslog.h>

#include <locale>
#include <mt/date_time>

#include <sockios/syslog.h>

extern char* __progname; // from crt0, linux

namespace misc {

using namespace std;

namespace detail {

class syslog_init
{
  private:
    class Init
    {
      public:
        Init()
          { _guard( 1 ); }
        ~Init()
          { _guard( 0 ); }

      private:
        static void _guard( int direction );
        static void __at_fork_prepare();
        static void __at_fork_child();
        static void __at_fork_parent();
        static std::tr2::mutex _init_lock;
        static int _count;
        static bool _at_fork;
    };

    static char Init_buf[];

    // protected:
  public:
    syslog_init();
    ~syslog_init();
};

// std::sockstream* _slog;
// std::locale* _loc;
static log_heap_type slogs;

// const char format[] = "%h %e %T ";

char syslog_init::Init_buf[128];
int syslog_init::Init::_count = 0;
std::tr2::mutex syslog_init::Init::_init_lock;
bool syslog_init::Init::_at_fork = false;
std::tr2::mutex _heap_lock;
#if 0
static string exename;
#endif

void syslog_init::Init::_guard( int direction )
{
  if ( direction ) {
    std::tr2::lock_guard<std::tr2::mutex> lk( _init_lock );
    if ( _count++ == 0 ) {
      if ( !_at_fork ) { // call only once
        if ( pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child ) ) {
          // throw system_error
        }
        _at_fork = true;
      }
#if 0
      if ( exename.empty() ) {
        stringstream exe;
        exe << "/proc/" << std::tr2::getpid() << "/exe";

        char b[1024];

        ssize_t sz = readlink( exe.str().c_str(), b, sizeof(b) );
        if ( sz >= 0 ) {
          // check errno?
          b[sz] = 0;
          exename = find( reverse_iterator<char*>( b + sz ), reverse_iterator<char*>(b), '/' ).base();
        }
      }

      openlog( exename.c_str(), LOG_PID, LOG_USER );
#else
      // ++_count; // see syslog_aux
      // detail::_loc = new locale( "C" ); // or loc( "" ); --- default
      // detail::_slog = new sockstream( "/dev/log" );
      // detail::_slog->imbue( *detail::_loc );
#endif
    }
  } else {
    std::tr2::lock_guard<std::tr2::mutex> lk( _init_lock );
    if ( --_count == 0 ) {
#if 0
      closelog();
#else
      // delete detail::_slog;
      // delete detail::_loc;
#endif
    }
  }
}

void syslog_init::Init::__at_fork_prepare()
{
  _init_lock.lock();
  if ( _count != 0 ) {
    // delete detail::_slog;
    // delete detail::_loc;
    close_syslog();
  }
}

void syslog_init::Init::__at_fork_child()
{
  if ( _count != 0 ) {
    // detail::_loc = new locale( "C" ); // or loc( "" ); --- default
    // detail::_slog = new sockstream( "/dev/log" );
    // detail::_slog->imbue( *detail::_loc );

    // closelog();
    // openlog( "", LOG_PID, LOG_USER );
    // _count = 1;
  }
  _init_lock.unlock();
}

void syslog_init::Init::__at_fork_parent()
{
  if ( _count != 0 ) {
    // detail::_loc = new locale( "C" ); // or loc( "" ); --- default
    // detail::_slog = new sockstream( "/dev/log" );
    // detail::_slog->imbue( *detail::_loc );

    // closelog();
    // openlog( "", LOG_PID, LOG_USER );
    // _count = 1;
  }
  _init_lock.unlock();
}

syslog_init::syslog_init()
{
  new(Init_buf) Init();
}

syslog_init::~syslog_init()
{
  ((Init *)Init_buf)->~Init();
}

static syslog_init _slog_aux;

ostream& xsyslog( int _level, int _facility )
{
  if ( LOG_PRI(_level) == 0 ) {
    return cerr; // throw invalid_argument();
  }

  if ( (_facility & LOG_FACMASK) == 0 ) {
    return cerr; // throw invalid_argument();
  }

  std::tr2::thread_base::id id =  std::tr2::this_thread::get_id();
  _heap_lock.lock();
  if ( detail::slogs[id] == 0 ) {
    detail::slogs[id] = new sockstream( "/dev/log" );
  }
  sockstream& slog = *detail::slogs[id];
  _heap_lock.unlock();

  // typedef time_put<char, ostreambuf_iterator<char, char_traits<char> > > time_put_facet;
  // struct tm ts;
  // const time_put_facet& tmp = use_facet<time_put_facet>( *detail::_loc );

  // time_t t = std::tr2::get_system_time().seconds_since_epoch();
  // localtime_r( &t, &ts );

  slog << '<' << (LOG_PRI(_level) | (_facility & LOG_FACMASK)) << '>';
  // tmp.put( *detail::_slog, *detail::_slog, ' ', &ts, detail::format, detail::format + sizeof(detail::format) );
  slog << __progname << '[' << std::tr2::getpid() << "]: ";
  
  return slog;
}

} // namespace detail

// void open_syslog()
// {
  // detail::_slog_aux = new detail::syslog_init();
// }

void close_syslog()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( detail::_heap_lock );
  // delete detail::_slog_aux;
  for ( detail::log_heap_type::iterator i = detail::slogs.begin(); i != detail::slogs.end(); ) {
    delete i->second;
    detail::slogs.erase( i++ );
  }
}

} // namespace misc
