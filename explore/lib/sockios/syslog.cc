// -*- C++ -*-

/*
 * Copyright (c) 2009-2011, 2020
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

#include <sstream>
#include <thread>
#include <mt/fork.h>
#include <sockios/sockstream>

#if defined(STLPORT) || defined(__FIT_CPP_0X)
#  include <unordered_map>
#  include <unordered_set>
#  define __USE_STLPORT_TR1
#endif

namespace misc {

std::ostream& use_syslog()
{ return detail::xsyslog(); }

namespace detail {
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
typedef std::unordered_map<std::thread::id,std::sockstream*> log_heap_type;
#else
#error "unordered_map expected"
#endif
} // namespace detail
} // namespce misc

#include <sys/syslog.h>
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
        static std::mutex _init_lock;
        static int _count;
        static bool _at_fork;
    };

    static char Init_buf[];

    // protected:
  public:
    syslog_init();
    ~syslog_init();
};

static log_heap_type slogs;

// const char format[] = "%h %e %T ";
#if 0
//                       0123456789012345
//                          |  |  |  |  |
const char timeline[] = "         :  :   "; // "Jun 23 14:56:38 ";
#else
const char timeline[] = ""; // null-length string
#endif

char syslog_init::Init_buf[128];
int syslog_init::Init::_count = 0;
std::mutex syslog_init::Init::_init_lock;
bool syslog_init::Init::_at_fork = false;
std::mutex _heap_lock;
static string prefix;

void syslog_init::Init::_guard( int direction )
{
  if ( direction ) {
    std::lock_guard<std::mutex> lk( _init_lock );
    if ( _count++ == 0 ) {
      if ( !_at_fork ) { // call only once
        if ( pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child ) ) {
          // throw system_error
        }
        _at_fork = true;
        stringstream s;

        s << detail::timeline << __progname << '[' << std::tr2::getpid() << "]: ";
        prefix = s.str();
      }
    }
  } else {
    std::lock_guard<std::mutex> lk( _init_lock );
    --_count;
  }
}

void syslog_init::Init::__at_fork_prepare()
{
  _init_lock.lock();
  close_syslog();
}

void syslog_init::Init::__at_fork_child()
{
  stringstream s;

  s << detail::timeline << __progname << '[' << std::tr2::getpid() << "]: ";
  prefix = s.str();

  _init_lock.unlock();
}

void syslog_init::Init::__at_fork_parent()
{
  _init_lock.unlock();
}

syslog_init::syslog_init()
{
  new (Init_buf) Init();
}

syslog_init::~syslog_init()
{
  Init* tmp = reinterpret_cast<Init*>(Init_buf);
  tmp->~Init();
}

static syslog_init _slog_aux; // register fork handlers

int default_log_level = LOG_ERR;
int default_log_facility = LOG_USER;

ostream& xsyslog( int _level, int _facility )
{
  if ( LOG_PRI(_level) == 0 ) {
    return cerr; // throw invalid_argument();
  }

  if ( (_facility & LOG_FACMASK) == 0 ) {
    return cerr; // throw invalid_argument();
  }

  std::thread::id id =  std::this_thread::get_id();
  _heap_lock.lock();
  if ( detail::slogs[id] == 0 ) {
    detail::slogs[id] = new sockstream( "/dev/log", sock_base::sock_dgram );
  }
  sockstream& slog = *detail::slogs[id];
  _heap_lock.unlock();

  // typedef time_put<char, ostreambuf_iterator<char, char_traits<char> > > time_put_facet;
  // struct tm ts;
  // const time_put_facet& tmp = use_facet<time_put_facet>( *detail::_loc );

  // time_t t = std::tr2::get_system_time().seconds_since_epoch();
  // localtime_r( &t, &ts );

  if ( slog.fail() ) {
    slog.clear();
    slog.close();
    slog.open("/dev/log", sock_base::sock_dgram);
  }

  slog << '<' << (LOG_PRI(_level) | (_facility & LOG_FACMASK)) << '>';

  /*
   Jun 23 14:56:32
   0123456789012345

   syslogd 1.5 ignore client timestamp (even local),
   but syslogd 1.4 (1.4.1 too?) not. But both try to detect invalid
   timestamp and put own if so. To detect timestamp used
   bytes 3, 6 (space), 9, 12 (':'); total length is 16.
   
   */
  // tmp.put( *detail::_slog, *detail::_slog, ' ', &ts, detail::format, detail::format + sizeof(detail::format) );
  slog << prefix /* detail::timeline
             << __progname << '[' << std::tr2::getpid() << "]: " */;
  
  return slog;
}

ostream& xsyslog( int _level )
{
  return xsyslog( _level, default_log_facility );
}

ostream& xsyslog()
{
  return xsyslog( default_log_level, default_log_facility );
}

} // namespace detail

void set_default_log_level(int log_level)
{
  detail::default_log_level = log_level;
}

void set_default_log_facility(int log_facility)
{
  detail::default_log_facility = log_facility;
}

void close_syslog()
{
  std::lock_guard<std::mutex> lk( detail::_heap_lock );
  for ( detail::log_heap_type::iterator i = detail::slogs.begin(); i != detail::slogs.end(); ++i ) {
    delete i->second;
  }
  detail::slogs.clear();
}

} // namespace misc
