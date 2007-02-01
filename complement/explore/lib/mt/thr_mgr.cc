// -*- C++ -*- Time-stamp: <07/02/01 20:17:50 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2005-2007
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

// #include <iostream>

namespace xmt {

// int _supercount = 0;

using namespace std;

struct bad_thread :
    public unary_function<Thread *,bool>
{
    bool operator()(const Thread *__x) const
      { return !__x->good(); }
};

struct good_thread :
    public unary_function<Thread *,bool>
{
    bool operator()(const Thread *__x) const
      { return (__x != 0) && (__x->good()); }
};

struct rm_if_bad_thread :
    public unary_function<Thread *,bool>
{
    bool operator()(Thread *__x);
};

bool rm_if_bad_thread::operator()(Thread *__x)
{
  if ( __x == 0 ) {
    return true;
  }
  if ( __x->bad() ) {
    // --_supercount;
    delete __x;
    return true;
  }
  return false;
}

struct thread_signal :
    public binary_function<Thread *,int,void>
{
    void operator()(const Thread *__x, int sig ) const;
};

void thread_signal::operator()(const Thread *__x, int sig ) const
{
  if ( __x != 0 ) {
    const_cast<Thread *>(__x)->kill( sig );
  }
}

__FIT_DECLSPEC ThreadMgr::~ThreadMgr()
{
  ThreadMgr::join();
}

__FIT_DECLSPEC void ThreadMgr::join()
{
  // xmt::block_signal( SIGINT );
  // xmt::block_signal( SIGTERM );
  // xmt::block_signal( SIGPIPE );
  // xmt::block_signal( SIGCHLD );
  // xmt::block_signal( SIGPOLL );

  _lock.lock();
  _M_c.erase( remove_if( _M_c.begin(), _M_c.end(), rm_if_bad_thread() ), _M_c.end() );
  while ( !_M_c.empty() ) {
    _lock.unlock();
    xmt::delay( xmt::timespec(0,50000000) );
    _lock.lock();
    _M_c.erase( remove_if( _M_c.begin(), _M_c.end(), rm_if_bad_thread() ), _M_c.end() );
    // cerr << "### " << _supercount << " " << _M_c.size() << endl;
  }
  // _supercount = 0;
  _lock.unlock();
}

__FIT_DECLSPEC
void ThreadMgr::launch( Thread::entrance_type entrance, const void *p, size_t psz, unsigned flags, size_t stack_sz )
{
  Locker lk( _lock );
  _M_c.erase( remove_if( _M_c.begin(), _M_c.end(), rm_if_bad_thread() ), _M_c.end() );
  _M_c.push_back( new Thread( entrance, p, psz, flags, stack_sz ) );
  // ++_supercount;
}

__FIT_DECLSPEC
void ThreadMgr::garbage_collector()
{
  Locker lk( _lock );
  _M_c.erase( remove_if( _M_c.begin(), _M_c.end(), rm_if_bad_thread() ), _M_c.end() );
}

ThreadMgr::container_type::size_type ThreadMgr::size()
{
  Locker lk( _lock );
  // ThreadMgr::container_type::size_type sz = count_if( _M_c.begin(), _M_c.end(), good_thread() );
  // cerr << "Sz: " << sz << endl;
  
  return count_if( _M_c.begin(), _M_c.end(), good_thread() );
}

__FIT_DECLSPEC void ThreadMgr::signal( int sig )
{
  // cerr << "Signal!" << endl;
  Locker lk( _lock );
  for_each( _M_c.begin(), _M_c.end(), bind2nd( thread_signal(), sig ) );
}

} // namespace xmt
