// -*- C++ -*- Time-stamp: <03/02/24 20:26:45 ptr>

/*
 * Copyright (c) 2003
 * Petr Ovchenkov
 *
 * Licensed under the Academic Free License Version 1.0
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

/*
 * This is library part of Linux specific problem:
 * function from dynamic library, that was assesed
 * via dlopen/dlsym calls, will fail to perform operations
 * with semaphores.
 * Here simple test: in function dl_function (that opened
 * from main, after dlopen) I initialize semaphore and
 * try to wait on this semaphore 1 second. Bug lead
 * to infinite delay here.
 * 
 */


#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

// #define _STLP_ASSERTIONS 1

#include <mt/xmt.h>
#include <iostream>

using namespace __impl;
using namespace std;

#if 0 // not solution
extern "C" void _init()
{
  fprintf( stderr, "init\n" );
}

extern "C" void _fini()
{
  fprintf( stderr, "fini\n" );
}
#endif // 0

extern "C" void dl_function()
{
  cerr << "dl_function start" << endl;

  timespec t;

  t.tv_sec = 1;
  t.tv_nsec = 0;

  __impl::Semaphore _s(0);
  _s.wait_delay( &t );

  cerr << "dl_function finish" << endl;

  return;
}
