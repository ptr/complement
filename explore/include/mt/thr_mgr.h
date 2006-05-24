// -*- C++ -*- Time-stamp: <02/09/25 11:37:29 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2005
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License Version 2.1
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
    void launch( Thread::entrance_type entrance, const void *p = 0, size_t psz = 0 );    
    __FIT_DECLSPEC void garbage_collector();

  protected:
    _Sequence _M_c;
    Mutex     _lock;
};

} // namespace xmt

#endif // __THR_MGR_H
