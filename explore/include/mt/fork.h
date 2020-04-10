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

#include <stdexcept>

#ifdef __unix
# include <pthread.h>
# include <semaphore.h>
#endif // __unix

#include <mt/signal>

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

pid_t getpid();
pid_t getppid();

namespace this_thread
{
using tr2::fork;

__FIT_DECLSPEC void become_daemon();

using std::this_thread::block_signal;
using std::this_thread::unblock_signal;
using std::this_thread::signal_handler;

} // namespace this_thread

} // namespace tr2

} // namespace std

#endif /* __THREAD_H */
