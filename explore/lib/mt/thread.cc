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
#ifndef _WIN32
# include <unistd.h>
#endif
#include <fcntl.h>
#include <cstdlib>

#include <mutex>
#include <thread>
#include <mt/fork.h>
#include <mt/callstack.h>

#include <cstring>
#ifndef _WIN32
# include <ostream>
#endif

#include <memory>
#include <functional>
#include <cerrno>
#include <string>
#include <ctime>
#include <list>

#include <stdio.h>
#ifdef __FIT_SYSCALL
#  include <syscall.h>
#endif

#ifdef WIN32
# pragma warning( disable : 4290)
// using namespace std;
#endif

namespace std {

namespace tr2 {

namespace detail {

#ifdef __FIT_SYSCALL

/*
  Problem: ::getpid() really return cached value, so pid returned may be
  parent's pid really. I use syscall here and rewrite it during std::tr2::fork().
  This workaround really not correct: looks that glibc's getpid() problem
  was inspired by call from signal handler while the thread executes fork;
  i) syscall is a bad way in this case too; ii) looks, this problem is fixed.

  See also glibc's ./nptl/sysdeps/unix/sysv/linux/getpid.c and
  ./nptl/sysdeps/unix/sysv/linux/fork.c.

  BTW, syscall is treated as deprecated.
*/

static pid_t _pid  = syscall( SYS_getpid );
static pid_t _ppid = syscall( SYS_getppid );
#endif // __FIT_SYSCALL

#ifdef __FIT_PSHARED_MUTEX
std::string _notpshared( "Platform not support process shared mutex" );
#endif

#ifdef __FIT_XSI_THR
std::string _notrecursive( "Platform not support recursive mutex" );
#endif

} // namespace detail
} // namespace tr2
} // namespace std

namespace std {

namespace tr2 {

using namespace std;

defer_lock_t defer_lock;
try_to_lock_t try_to_lock;
adopt_lock_t adopt_lock;

const std::string msg1( "Can't create thread" );
const std::string msg2( "Can't fork" );

// __FIT_DECLSPEC
// void signal_throw( int sig )
// {
//   throw sig;
// }

pid_t getpid()
{
#ifdef __FIT_SYSCALL
  return detail::_pid;
#else
  return ::getpid();
#endif
}

pid_t getppid()
{
#ifdef __FIT_SYSCALL
  return detail::_ppid;
#else
  return ::getppid();
#endif
}

__FIT_DECLSPEC
void fork()
{
#ifdef __unix
  fork_in_parent f( ::fork() );
  if ( f.pid() > 0 ) {
    throw f;
  }
  if ( f.pid() == -1 ) {
    throw std::runtime_error( msg2 );
  }
#ifdef __FIT_SYSCALL
  detail::_ppid = detail::_pid;
  detail::_pid = syscall( SYS_getpid );
#endif

#endif
}

namespace this_thread
{

__FIT_DECLSPEC
void become_daemon()
{
#ifdef __unix
  try {
    std::tr2::this_thread::fork();

    ::chdir( "/var/tmp" ); // for CWD: if not done, process remain with same WD
                         // and don't allow unmount volume, for example
    ::setsid();   // become session leader
    ::close( 0 ); // close stdin
    ::close( 1 ); // close stdout
    ::close( 2 ); // close stderr
    // This is broken in some versions of glibc (Linux):
    ::open( "/dev/null", O_RDONLY, 0 ); // redirect stdin from /dev/null
    ::open( "/dev/null", O_WRONLY, 0 ); // redirect stdout to /dev/null
    ::open( "/dev/null", O_WRONLY, 0 ); // redirect stderr to /dev/null
  }
  catch ( fork_in_parent& ) {
    throw;
  }
  catch ( std::runtime_error& ) {
    throw;
  }
#endif
}

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

} // namespace tr2

} // namespace std
