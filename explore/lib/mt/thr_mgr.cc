// -*- C++ -*- Time-stamp: <00/02/24 19:33:40 ptr>
#ident "$SunId$ %Q%"

#include "mt/thr_mgr.h"
#include <algorithm>
#include <functional>

using namespace __STD;

namespace __impl {

struct bad_thread :
    public unary_function<Thread *,bool>
{
    bool operator()(const Thread *__x) const
      { return !__x->good(); }
};

__PG_DECLSPEC ThreadMgr::~ThreadMgr()
{
  MT_REENTRANT( _lock, _1 );
  container_type::iterator i = _M_c.begin();

  while ( i != _M_c.end() ) {
    if ( (*i)->good() ) {
      (*i)->kill( SIGTERM );
      (*i)->join();
    }
    delete *i;
    _M_c.erase( i++ );
  }
}

__PG_DECLSPEC
void ThreadMgr::launch( Thread::entrance_type entrance, const void *p, size_t psz )
{
  MT_REENTRANT( _lock, _1 );
  container_type::iterator i = find_if( _M_c.begin(), _M_c.end(), bad_thread() );
  if ( i == _M_c.end() ) {
    _M_c.push_back( new Thread( unsigned(Thread::daemon | Thread::detached) ) );
    i = --_M_c.end();
  }
  (*i)->launch( entrance, p, psz );
}

__PG_DECLSPEC
void ThreadMgr::garbage_collector()
{
  MT_REENTRANT( _lock, _1 );
  container_type::iterator i = _M_c.begin();

  while ( i != _M_c.end() ) {
    if ( !(*i)->good() ) {
      delete *i;
      _M_c.erase( i++ );
    } else {
      ++i;
    }
  }
}

} // namespace __impl
