// -*- C++ -*- Time-stamp: <06/10/10 19:33:54 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2005, 2006
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "mt/thr_mgr.h"
#include <algorithm>
#include <functional>

namespace xmt {

using namespace std;

struct bad_thread :
    public unary_function<Thread *,bool>
{
    bool operator()(const Thread *__x) const
      { return !__x->good(); }
};

__FIT_DECLSPEC ThreadMgr::~ThreadMgr()
{
  MT_REENTRANT( _lock, _x1 );
  container_type::iterator i = _M_c.begin();

  while ( i != _M_c.end() ) {
    if ( (*i)->good() ) {
      // (*i)->kill( SIGTERM );
    }
    (*i)->join(); 
    delete *i;
    _M_c.erase( i++ );
  }
}

__FIT_DECLSPEC
void ThreadMgr::launch( Thread::entrance_type entrance, const void *p, size_t psz, unsigned flags, size_t stack_sz )
{
  MT_REENTRANT( _lock, _x1 );
  container_type::iterator i = _M_c.begin();

  while ( i != _M_c.end() ) {
    if ( !(*i)->good() ) {
      (*i)->join();
      delete *i;
      _M_c.erase( i++ );
    } else {
      ++i;
    }
  }

  _M_c.push_back( new Thread( entrance, p, psz, flags, stack_sz ) );
}

__FIT_DECLSPEC
void ThreadMgr::garbage_collector()
{
  MT_REENTRANT( _lock, _x1 );
  container_type::iterator i = _M_c.begin();

  while ( i != _M_c.end() ) {
    if ( !(*i)->good() ) {
      (*i)->join();
      delete *i;
      _M_c.erase( i++ );
    } else {
      ++i;
    }
  }
}

ThreadMgr::container_type::size_type ThreadMgr::size()
{
  MT_REENTRANT( _lock, _x1 );
  container_type::iterator i = _M_c.begin();

  while ( i != _M_c.end() ) {
    if ( !(*i)->good() ) {
      (*i)->join();
      delete *i;
      _M_c.erase( i++ );
    } else {
      ++i;
    }
  }
  return _M_c.size();
}

} // namespace xmt
