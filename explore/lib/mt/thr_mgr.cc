// -*- C++ -*- Time-stamp: <02/04/14 18:01:24 ptr>

/*
 * Copyright (c) 1997-1999, 2002
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

#include "mt/thr_mgr.h"
#include <algorithm>
#include <functional>

namespace __impl {

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
      (*i)->kill( SIGTERM );
      (*i)->join();
    }
    delete *i;
    _M_c.erase( i++ );
  }
}

__FIT_DECLSPEC
void ThreadMgr::launch( Thread::entrance_type entrance, const void *p, size_t psz )
{
  MT_REENTRANT( _lock, _x1 );
  container_type::iterator i = find_if( _M_c.begin(), _M_c.end(), bad_thread() );
  if ( i == _M_c.end() ) {
    _M_c.push_back( new Thread( unsigned(Thread::daemon | Thread::detached) ) );
    i = --_M_c.end();
  }
  (*i)->launch( entrance, p, psz );
}

__FIT_DECLSPEC
void ThreadMgr::garbage_collector()
{
  MT_REENTRANT( _lock, _x1 );
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
