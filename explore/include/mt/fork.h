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

#ifndef __FORK_H
#define __FORK_H

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <memory>
#include <cstddef>
#include <climits>
#include <stdexcept>

#ifdef __unix
# include <pthread.h>
# include <semaphore.h>
# include <sched.h>
# include <signal.h>
#endif // __unix

#include <cerrno>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <mt/callstack.h>

namespace std {

namespace tr2 {

namespace this_thread
{ }

class fork_in_parent :
        public std::exception
{
  public:
    fork_in_parent() throw()
      { _pid = 0; }
    fork_in_parent( pid_t p ) throw()
      { _pid = p; }
    virtual ~fork_in_parent() throw()
      { }
    virtual const char *what() const throw()
      { return "class fork_in_parent"; }
    pid_t pid() throw()
      { return _pid; }

  private:
    pid_t _pid;
};

__FIT_DECLSPEC void fork();

//thread_base::id get_id();

pid_t getpid();
pid_t getppid();

namespace this_thread
{
using tr2::fork;

__FIT_DECLSPEC void become_daemon();

// std::thread_base::id get_id();
// using tr2::get_id;

//inline void yield()
//{
//#ifdef __FIT_PTHREADS
//  // sched_yield();
//  pthread_yield();
//#endif
//}

//__FIT_DECLSPEC void sleep( const std::tr2::system_time& abs_t );

//__FIT_DECLSPEC void sleep( const std::tr2::nanoseconds& rel_t );

//template <class Duration>
//void sleep( const Duration& rel_t )
//{ std::tr2::this_thread::sleep( static_cast<std::tr2::nanoseconds>(rel_t) ); }

__FIT_DECLSPEC void block_signal( int sig );
__FIT_DECLSPEC void unblock_signal( int sig );
__FIT_DECLSPEC int signal_handler( int sig, void (*handler)(int) );
__FIT_DECLSPEC int signal_handler( int sig, void (*handler)(int, siginfo_t*, void*) );

} // namespace this_thread

} // namespace tr2

} // namespace std

#endif /* __THREAD_H */
