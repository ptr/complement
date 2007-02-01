// -*- C++ -*- Time-stamp: <07/02/01 18:39:25 ptr>

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

#ifndef __THR_MGR_H
#define __THR_MGR_H

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifndef __XMT_H
#include <mt/xmt.h>
#endif

#include <list>

namespace xmt {

class ThreadMgr
{
  public:
    typedef std::list<Thread *> _Sequence;
    typedef _Sequence  container_type;

    ThreadMgr()
      { }
    __FIT_DECLSPEC ~ThreadMgr();

    __FIT_DECLSPEC
    void launch( Thread::entrance_type entrance, const void *p = 0, size_t psz = 0, unsigned flags = 0, size_t stack_sz = 0 );    
    __FIT_DECLSPEC void garbage_collector();
    __FIT_DECLSPEC void join();
    __FIT_DECLSPEC void signal( int );

    container_type::size_type size();

  protected:
    _Sequence _M_c;
    Mutex     _lock;
};

} // namespace xmt

#endif // __THR_MGR_H
