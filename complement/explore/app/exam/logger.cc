// -*- C++ -*- Time-stamp: <07/07/13 10:53:32 ptr>

#include "logger.h"
#include <iostream>

namespace exam {

using namespace std;

int base_logger::flags() const
{
  return _flags;
}

bool base_logger::is_trace() const
{
  return (_flags & trace) != 0;
}

int base_logger::flags( int f )
{
  int tmp = _flags;
  _flags = f;
  if ( (f & silent) != 0 ) {
    _flags &= ~trace_suite;
  }
  return tmp;
}

void trivial_logger::report( const char *file, int line, bool cnd, const char *expr )
{
  if ( (cnd && ((_flags & trace) == 0)) || ((_flags & silent) != 0) ) {
    return;
  }

  if ( s != 0 ) {
    *s << file << ":" << line << ": " << (cnd ? "pass" : "fail" ) << ": " << expr
       << std::endl;
  } else {
    fprintf( f, "%s:%d: %s: %s\n", file, line, (cnd ? "pass" : "fail" ), expr );
  }
}

void trivial_logger::begin_ts()
{
  if ( (_flags & trace_suite) == 0 ) {
    return;
  }

  if ( s != 0 ) {
    *s << "== Begin test suite\n";
  } else {
    fprintf( f, "== Begin test suite\n" );
  }
}

void trivial_logger::end_ts()
{
  if ( (_flags & trace_suite) == 0 ) {
    return;
  }

  if ( *s ) {
    *s << "==  End test suite\n";
  } else {
    fprintf( f, "==  End test suite\n" );
  }
}

void trivial_logger::result( const base_logger::stat& _stat, const string& suite_name )
{
  if ( s != 0 ) {
    *s << "*** " << (_stat.failed != 0 ? "FAIL " : "PASS " ) << suite_name
       << " (+" << _stat.passed
       <<   "-" << _stat.failed
       <<   "~" << _stat.skipped << "/" << _stat.total << ") ***" << endl;
  } else {
    fprintf( f, "*** %s (+%d-%d~%d/%d) ***\n", (_stat.failed != 0 ? "FAIL" : "PASS" ), _stat.passed, _stat.failed, _stat.skipped, _stat.total );
  }
}

void trivial_logger::tc( base_logger::tc_result r, const std::string& name )
{
  if ( ((_flags & silent) != 0) || ((r == pass) && ((_flags & verbose) == 0) )) {
    return;
  }

  static const char *m[] = { "  PASS ", "  FAIL ", "  SKIP " };
  const char *rs = "";

  switch ( r )
  {
    case pass:
      rs = m[0];
      break;
    case fail:
      rs = m[1];
      break;
    case skip:
      rs = m[2];
      break;
  }

  if ( s != 0 ) {
    *s << rs << name << endl;
  } else {
    fprintf( f, "%s%s\n", rs, name.c_str() );
  }
}

} //namespace exam
