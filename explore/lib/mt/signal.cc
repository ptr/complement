// -*- C++ -*-

/*
 * Copyright (c) 1997-1999, 2002-2011, 2020
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 * Derived from original <mt/xmt.h> of 'complement' project
 * [http://complement.sourceforge.net]
 * to make it close to JTC1/SC22/WG21 C++ 0x working draft
 * [http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/2008/n2521.pdf]
 */

#include <config/feature.h>

#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

namespace std {

namespace this_thread
{

__FIT_DECLSPEC void block_signal( int sig )
{
#ifdef __unix
  sigset_t sigset;

  sigemptyset( &sigset );
  sigaddset( &sigset, sig );
#  ifdef __FIT_PTHREADS
  pthread_sigmask( SIG_BLOCK, &sigset, 0 );
#  endif
#endif // __unix
}

__FIT_DECLSPEC void unblock_signal( int sig )
{
#ifdef __unix
  sigset_t sigset;

  sigemptyset( &sigset );
  sigaddset( &sigset, sig );
#  ifdef __FIT_PTHREADS
  pthread_sigmask( SIG_UNBLOCK, &sigset, 0 );
#  endif
#endif // __unix
}

__FIT_DECLSPEC int signal_handler( int sig, void (*handler)(int) )
{
#ifdef __unix
  struct sigaction act;

  sigemptyset( &act.sa_mask );
  sigaddset( &act.sa_mask, sig );

  act.sa_flags = 0; // SA_RESTART;
  act.sa_handler = handler;
  return sigaction( sig, &act, 0 );
#else
  return -1;
#endif // __unix
}

__FIT_DECLSPEC int signal_handler( int sig, void (*handler)(int, siginfo_t*, void*) )
{
#ifdef __unix
  struct sigaction act;

  sigemptyset( &act.sa_mask );
  sigaddset( &act.sa_mask, sig );

  act.sa_flags = SA_SIGINFO; // SA_RESTART;
  act.sa_sigaction = handler;
  return sigaction( sig, &act, 0 );
#else
  return -1;
#endif // __unix
}

} // namespace this_thread

} // namespace std
