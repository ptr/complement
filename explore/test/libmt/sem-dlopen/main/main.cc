// -*- C++ -*- Time-stamp: <03/02/24 21:32:22 ptr>

/*
 * Copyright (c) 2003
 * Petr Ovtchenkov
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
 * This is main part of Linux specific problem:
 * function from dynamic library, that was assesed
 * via dlopen/dlsym calls, will fail to perform operations
 * with semaphores.
 * Here simple test: in function dl_function (that opened
 * from main, after dlopen) I initialize semaphore and
 * try to wait on this semaphore 1 second. Bug lead
 * to infinite delay here.
 *
 * 1. Start
 * 2. Initialize and wait 1 sec on semaphore with timeout (should work)
 * 3. Load library with dlopen
 * 4. Find address of dl_function from library
 * 5. Call dl_function
 * 6. In dl_function was called semaphore with 1 sec timeout,
 *    so waiting should be about 1 sec, but waiting is infinite.
 *    This is a Linux threads bug... 
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
#include <dlfcn.h>

using namespace __impl;
using namespace std;


typedef void (*func_type)();

int main( int, char * const * )
{
  char *err;

  cerr << "main: start" << endl;

  timespec t;

  t.tv_sec = 1;
  t.tv_nsec = 0;

  __impl::Semaphore _s(0);
   _s.wait_time( &t );
  
  cerr << "main: delay on semaphore" << endl;
  _s.wait_delay( &t );
  cerr << "main: delay on semaphore, passed" << endl;


  void *dll = dlopen( "../dl/obj/gcc/shared-g/libsem-dl_gcc-g.so", RTLD_NOW );

  if ( err = dlerror() ) {
    cerr << err << endl;
  }

  func_type f = (func_type)dlsym( dll, "dl_function" );

  if ( err = dlerror() ) {
    cerr << err << endl;
  }

  f();

  dlclose( dll );

  if ( err = dlerror() ) {
    cerr << err << endl;
  }

  cerr << "main: finish" << endl;

  return 0;
}
