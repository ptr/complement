// -*- C++ -*- Time-stamp: <99/05/24 13:51:02 ptr>
#ident "$SunId$ %Q%"

#include <thr_mgr.h>
#include <algorithm>

using namespace std;

namespace __impl {

struct bad_thread :
    public unary_function<Thread *,bool>
{
    bool operator()(const Thread *__x) const
      { return !__x->good(); }
};

__DLLEXPORT ThreadMgr::~ThreadMgr()
{
  MT_REENTRANT( _lock, _1 );
  container_type::iterator i = _M_c.begin();

  while ( i != _M_c.end() ) {
    if ( (*i)->good() ) {
      (*i)->kill( SIGTERM );
    }
    delete *i;
    _M_c.erase( i++ );
  }
}

__DLLEXPORT
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

__DLLEXPORT
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
