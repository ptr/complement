// -*- C++ -*- Time-stamp: <99/09/03 10:47:18 ptr>
#ident "$SunId$ %Q%"

#ifdef WIN32
#  ifdef _DLL
#    define __XMT_DLL __declspec( dllexport )
#  else
#    define __XMT_DLL
#  endif
#else
#  define __XMT_DLL
#endif

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

__XMT_DLL ThreadMgr::~ThreadMgr()
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

__XMT_DLL
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

__XMT_DLL
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
