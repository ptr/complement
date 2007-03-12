// -*- C++ -*- Time-stamp: <07/03/12 20:14:35 ptr>

/*
 * Copyright (c) 1997-1999, 2002-2007
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <config/feature.h>

#include <sys/types.h>
#ifndef _WIN32
# include <unistd.h>
#endif
#include <fcntl.h>

#include <mt/xmt.h>

#include <cstring>
#ifndef _WIN32
# include <ostream>
#endif

#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <functional>
#include <cerrno>
#include <string>

#ifdef __linux
#  ifndef __USE_GNU
#    define __USE_GNU
#  endif
#  include <sys/time.h>
#endif

#include <stdio.h>
#include <syscall.h>

#include <cmath> // for time operations

#ifdef WIN32
# pragma warning( disable : 4290)
// using namespace std;
#endif

/*
extern "C" {
#ifdef WIN32
  typedef unsigned long (__stdcall *entrance_type_C)( void * );
#else
  typedef void *(*entrance_type_C)( void * );
#endif
}
*/

#ifdef WIN32
#  if 0
extern "C" int APIENTRY
DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
{
  return TRUE;   // ok
}
#  endif
#endif

namespace xmt {
namespace detail {

int Init_count = 0;

// problem: ::getpid() really return cached value, so pid returned may be
// parent's pid really. I use syscall here and rewrite it during Thread::fork().

static pid_t _pid  = syscall( SYS_getpid );
static pid_t _ppid = syscall( SYS_getppid );

xmt::Thread::thread_key_type _mt_key = __STATIC_CAST(xmt::Thread::thread_key_type,-1);
# ifndef __FIT_WIN32THREADS
void *_uw_save = 0;
# endif

#ifdef _PTHREADS
xmt::Mutex _F_lock;
#  define _F_locklock  xmt::detail::_F_lock.lock();
#  define _F_lockunlock xmt::detail::_F_lock.unlock();
#endif

#ifdef __FIT_UITHREADS
#  error "Unimplemented"
#endif

#if defined(__FIT_WIN32THREADS)
#  define _F_locklock
#  define _F_lockunlock
#endif

#ifdef __FIT_PSHARED_MUTEX
std::string _notpshared( "Platform not support process shared mutex" );
#endif

#ifdef __FIT_XSI_THR
std::string _notrecursive( "Platform not support recursive mutex" );
#endif

} // namespace detail
} // namespace xmt

extern "C" void __at_fork_prepare()
{
#ifdef _PTHREADS
  if ( xmt::detail::Init_count > 0 ) {
    xmt::detail::_uw_save = pthread_getspecific( xmt::detail::_mt_key );
  }
#endif
}

extern "C" void __at_fork_parent()
{
#ifdef _PTHREADS
#endif
}

extern "C" void __at_fork_child()
{
#ifdef _PTHREADS
  if ( xmt::detail::Init_count > 0 ) {
     // otherwise we do it in Thread::Init::Init() below
# if !(defined(__FreeBSD__) || defined(__OpenBSD__))
    // I am misunderstand this point, Solaris 7 require this (to be restored)

    // pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child );

    // while Linux (and other) inherit this setting from parent process?
    // At least Linux glibc 2.2.5 try to made lock in recursive
    // call of pthread_atfork
# else
// should be fixed...
# endif // !(__FreeBSD__ || __OpenBSD__)
    pthread_key_create( &xmt::detail::_mt_key, 0 );
    pthread_setspecific( xmt::detail::_mt_key, xmt::detail::_uw_save );
    xmt::detail::_uw_save = 0;
    // Note, that only calling thread inherited when we use POSIX:
    xmt::detail::Init_count = 1; // i.e. only ONE (calling) thread...
  }
#endif
}

namespace xmt {

#ifndef _WIN32
using std::cerr;
using std::endl;
#endif

char *Init_buf[32];
int& Thread::Init::_count( detail::Init_count ); // trick to avoid friend declarations

const std::string msg1( "Can't create thread" );
const std::string msg2( "Can't fork" );

// __FIT_DECLSPEC
// void signal_throw( int sig ) throw( int )
// {
//   throw sig;
// }


Thread::Init::Init()
{
  // This is performance problem, let's consider replace to atomic lock...
  _F_locklock

  if ( _count++ == 0 ) {
#ifdef __FIT_UITHREADS
    thr_keycreate( &_mt_key, 0 );
#endif
#ifdef _PTHREADS
# if !(defined(__FreeBSD__) || defined(__OpenBSD__))
    if ( pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child ) ) {
      _F_lockunlock
      throw std::runtime_error( "Problems with pthread_atfork" );
    }
# endif // !(__FreeBSD__ || __OpenBSD__)
    pthread_key_create( &_mt_key, 0 );
#endif
#ifdef __FIT_WIN32THREADS
    _mt_key = TlsAlloc();
#endif
    Thread::_self_idx = Thread::xalloc();
  }

  _F_lockunlock
}

Thread::Init::~Init()
{
  // This is performance problem, let's consider replace to atomic lock...
  _F_locklock

  if ( --_count == 0 ) {
#ifdef __FIT_WIN32THREADS
    TlsFree( _mt_key );
#endif
#ifdef _PTHREADS
    pthread_key_delete( _mt_key );
#endif
  }

  _F_lockunlock
}

Thread::alloc_type Thread::alloc;
int Thread::_idx = 0;
int Thread::_self_idx = 0;
Mutex Thread::_idx_lock;

#ifdef __FIT_WIN32THREADS
const Thread::thread_id_type Thread::bad_thread_id = INVALID_HANDLE_VALUE;
#endif // __FIT_WIN32THREADS

#if defined(__FIT_UITHREADS) || defined(_PTHREADS)
# if !(defined(__FreeBSD__) || defined(__OpenBSD__))
const Thread::thread_id_type Thread::bad_thread_id = __STATIC_CAST(Thread::thread_id_type,-1);
# else // __FreeBSD__ || __OpenBSD__
// pthread_t is defined as 'typedef struct pthread *pthread_t;'
const Thread::thread_id_type Thread::bad_thread_id = __STATIC_CAST(Thread::thread_id_type,0);
# endif // !(__FreeBSD__ || __OpenBSD__)
#endif // __FIT_UITHREADS || _PTHREADS

Thread::thread_key_type& Thread::_mt_key( detail::_mt_key );

__FIT_DECLSPEC
void Thread::_dealloc_uw()
{
  if ( uw_alloc_size != 0 ) {
    // _STLP_ASSERT( _id != bad_thread_id );
    // _STLP_ASSERT( is_self() );
#ifdef __FIT_UITHREADS
    _uw_alloc_type *user_words;
    thr_getspecific( _mt_key, &(static_cast<void *>(user_words)) );
#endif
#ifdef _PTHREADS
    _uw_alloc_type *user_words = static_cast<_uw_alloc_type *>(pthread_getspecific( _mt_key ));
#endif
#ifdef __FIT_WIN32THREADS
    _uw_alloc_type *user_words = static_cast<_uw_alloc_type *>(TlsGetValue( _mt_key ));
#endif
    alloc.deallocate( user_words, uw_alloc_size );
    user_words = 0;
    uw_alloc_size = 0;
  }
}

__FIT_DECLSPEC
Thread::_uw_alloc_type *Thread::_alloc_uw( int __idx )
{
  // _STLP_ASSERT( _id != bad_thread_id );
  // _STLP_ASSERT( is_self() );

  _uw_alloc_type *user_words;

  if ( uw_alloc_size == 0 ) {
    uw_alloc_size = sizeof( _uw_alloc_type ) * (__idx + 1);
#if !defined(_STLP_VERSION) && defined(_MSC_VER)
    user_words = alloc.allocate( uw_alloc_size, (void *)0 );
#else
    user_words = alloc.allocate( uw_alloc_size );
#endif
    std::fill( user_words, user_words + uw_alloc_size, 0 );
#ifdef __FIT_UITHREADS
    thr_setspecific( _mt_key, user_words );
#endif
#ifdef _PTHREADS
    pthread_setspecific( _mt_key, user_words );
#endif
#ifdef __FIT_WIN32THREADS
    TlsSetValue( _mt_key, user_words );
#endif
  } else {
#ifdef __FIT_UITHREADS
    thr_getspecific( _mt_key, &(static_cast<void *>(user_words)) );
#endif
#ifdef _PTHREADS
    user_words = static_cast<_uw_alloc_type *>(pthread_getspecific( _mt_key ));
#endif
#ifdef __FIT_WIN32THREADS
    user_words = static_cast<_uw_alloc_type *>(TlsGetValue( _mt_key ));
#endif
    if ( (__idx + 1) * sizeof( _uw_alloc_type ) > uw_alloc_size ) {
      size_t tmp = sizeof( _uw_alloc_type ) * (__idx + 1);
#if !defined(_STLP_VERSION) && defined(_MSC_VER)
      _uw_alloc_type *_mtmp = alloc.allocate( tmp, (void *)0 );
#else
      _uw_alloc_type *_mtmp = alloc.allocate( tmp );
#endif
      std::copy( user_words, user_words + uw_alloc_size, _mtmp );
      std::fill( _mtmp + uw_alloc_size, _mtmp + tmp, 0 );
      alloc.deallocate( user_words, uw_alloc_size );
      user_words = _mtmp;
      uw_alloc_size = tmp;
#ifdef __FIT_UITHREADS
      thr_setspecific( _mt_key, user_words );
#endif
#ifdef _PTHREADS
      pthread_setspecific( _mt_key, user_words );
#endif
#ifdef __FIT_WIN32THREADS
      TlsSetValue( _mt_key, user_words );
#endif
    }
  }

  return user_words + __idx;
}

__FIT_DECLSPEC
Thread::Thread( unsigned __f ) :
    _id( bad_thread_id ),
    _rip_id( bad_thread_id ),
    _entrance( 0 ),
    _param( 0 ),
    _param_sz( 0 ),
    _flags( __f ),
    _stack_sz( 0 ),
    uw_alloc_size( 0 )
{
  new( Init_buf ) Init();
}

__FIT_DECLSPEC
Thread::Thread( Thread::entrance_type entrance, const void *p, size_t psz, unsigned __f, size_t stack_sz ) :
    _id( bad_thread_id ),
    _rip_id( bad_thread_id ),
    _entrance( entrance ),
    _param( 0 ),
    _param_sz( 0 ),
    _flags( __f ),
    _stack_sz( stack_sz ),
    uw_alloc_size( 0 )
{
  new( Init_buf ) Init();
  // Locker lk( _llock );
  _create( p, psz );
}

__FIT_DECLSPEC
Thread::~Thread()
{
  Thread::join();

  if ( (_flags & (daemon | detached)) != 0 ) { // not joinable
    // Locker lk( _llock );
    if ( _id != bad_thread_id ) { // still run?
      cerr << "Crash on daemon thread exit expected, threas id " << _id << endl;
    }
  }

  ((Init *)Init_buf)->~Init();

  // _STLP_ASSERT( _id == bad_thread_id );
  // Thread::kill( SIGTERM );
  // Thread::signal_exit( SIGTERM ); // call handler directly, avoid signal delivery
}

__FIT_DECLSPEC
bool Thread::is_self()
{
  // Locker lk( _llock );
#ifdef _PTHREADS
  return (_id != bad_thread_id) && (_id == pthread_self());
#elif defined(__FIT_UITHREADS)
  return (_id != bad_thread_id) && (_id == thr_self());
#elif defined(__FIT_WIN32THREADS)
  return (_id != bad_thread_id) && (_id == GetCurrentThread());
#else
#  error "Fix me! (replace pthread_self())"
#endif
}

__FIT_DECLSPEC
void Thread::launch( entrance_type entrance, const void *p, size_t psz, size_t stack_sz )
{
  // Locker lk( _llock );
  _stack_sz = stack_sz;
  if ( _not_run() ) {
    _entrance = entrance;
    _create( p, psz );
  }
}

__FIT_DECLSPEC
Thread::ret_code Thread::join()
{
  ret_code rt;

#ifdef __FIT_WIN32THREADS
  rt.iword = 0;
  if ( !_not_run() ) {
    WaitForSingleObject( _id, -1 );
    GetExitCodeThread( _id, &rt.iword );
    CloseHandle( _id );
    // Locker lk( _llock );
    _id = bad_thread_id;
  }
#endif // __FIT_WIN32THREADS
#if defined(__FIT_UITHREADS) || defined(_PTHREADS)
  rt.pword = 0;
  if ( is_join_req() ) {
#  ifdef _PTHREADS
    pthread_join( _rip_id, &rt.pword );
#  endif
#  ifdef __FIT_UITHREADS
    thr_join( _rip_id, 0, &rt.pword );
#  endif
    // Locker lk( _llock );
    _rip_id = bad_thread_id;
  }
#endif // __FIT_UITHREADS || PTHREADS

  return rt;
}

__FIT_DECLSPEC
int Thread::suspend()
{
  if ( good() ) {
    // to do: fix possible race with _id, when it used here
#ifdef __FIT_WIN32THREADS
    return SuspendThread( _id );
#endif
#ifdef _PTHREADS
    // sorry, POSIX threads don't have pthread_{suspend,continue} calls, so it should
    // be simulated via condwait
#  ifdef __hpux
    // but HP-UX 11.00 implementation of POSIX threads has extention (or it in X/Open?)
    // pthread_{suspend,continue}, I use it:
    return pthread_suspend( _id );
#  else
    _suspend.set( false );
    return _suspend.try_wait(); // thr_suspend and pthread_cond_wait return 0 in success
#  endif
#endif // __STL_PTHREADS
#ifdef __FIT_UITHREADS
    return thr_suspend( _id );
#endif
  }

  return -1;
}

__FIT_DECLSPEC
int Thread::resume()
{
  if ( good() ) {
    // to do: fix possible race with _id, when it used here
#ifdef __FIT_WIN32THREADS
    return ResumeThread( _id );
#endif
#ifdef _PTHREADS
    // sorry, POSIX threads don't have pthread_{suspend,continue} calls, so it should
    // be simulated via condwait
#  ifdef __hpux
    // but HP-UX 11.00 implementation of POSIX threads has extention (or it in X/Open?)
    // pthread_{suspend,continue}, I use it:
    return pthread_continue( _id );
#  else
    return _suspend.set( true ) == false ? 0 : -1; // less syscall than _suspend.signal();
#  endif
    
#endif
#ifdef __FIT_UITHREADS
    return thr_continue( _id );
#endif
  }

  return -1;
}

__FIT_DECLSPEC
int Thread::kill( int sig )
{
  // Locker lk( _llock );

  if ( _id != bad_thread_id ) {
#ifdef __FIT_UITHREADS
    return thr_kill( _id, sig );
#endif
#ifdef _PTHREADS
    return pthread_kill( _id, sig );
#endif
#ifdef __FIT_WIN32THREADS
  // The behavior of TerminateThread significant differ from SOLARIS and POSIX
  // threads, and I don't find analogs to workaround...
    return TerminateThread( _id, sig ) ? 0 : -1;
#endif
  }
  return -1;
}

__FIT_DECLSPEC
void Thread::_exit( int code )
{
#ifdef _PTHREADS
  ret_code v;
  v.iword = code;
  pthread_exit( v.pword );
#endif
#ifdef __FIT_UITHREADS
  ret_code v;
  v.iword = code;
  thr_exit( v.pword );
#endif
#ifdef __FIT_WIN32THREADS
  ExitThread( code );
#endif
}

#ifdef __FIT_UITHREADS
__FIT_DECLSPEC
int Thread::join_all()
{
  while ( thr_join( 0, 0, 0 ) == 0 ) ;

  return 0;
}
#endif

__FIT_DECLSPEC
void Thread::signal_exit( int sig )
{
#ifdef _PTHREADS
  _uw_alloc_type *user_words =
    static_cast<_uw_alloc_type *>(pthread_getspecific( Thread::_mt_key ));
  if ( user_words == 0 ) {
    // async signal unsafe? or wrong thread called?
    // this a bad point, do nothing
    return;
  }
  user_words += Thread::_self_idx;
  Thread *me = reinterpret_cast<Thread *>(*reinterpret_cast<void **>(user_words));
  // _STLP_ASSERT( me->is_self() );

  // follow part of _call
  if ( (me->_flags & (daemon | detached)) != 0 ) { // otherwise join expected
    // Locker lk( me->_llock ); // !!!!??? in the signal handler?
    me->_rip_id = me->_id = bad_thread_id;
  } else {
    me->_id = bad_thread_id;
  }
  me->_dealloc_uw(); // clear user words
  void *_param     = me->_param;
  size_t _param_sz = me->_param_sz;
  try {
    if ( _param_sz > sizeof(void *) ) { // that's allocated
      delete [] __STATIC_CAST(char *,_param);
      _param_sz = 0;
      _param = 0;
    }
  }
  catch ( ... ) {
  }
#endif
  // well, this is suspicious, due to pthread_exit isn't async signal
  // safe; what I should use here?
  Thread::_exit( 0 );
}

__FIT_DECLSPEC
void block_signal( int sig )
{
#ifdef __unix
  sigset_t sigset;

  sigemptyset( &sigset );
  sigaddset( &sigset, sig );
#  ifdef __FIT_UITHREADS
  thr_sigsetmask( SIG_BLOCK, &sigset, 0 );
#  endif
#  ifdef _PTHREADS
  pthread_sigmask( SIG_BLOCK, &sigset, 0 );
#  endif
#endif // __unix
}

__FIT_DECLSPEC
void unblock_signal( int sig )
{
#ifdef __unix
  sigset_t sigset;

  sigemptyset( &sigset );
  sigaddset( &sigset, sig );
#  ifdef __FIT_UITHREADS
  thr_sigsetmask( SIG_UNBLOCK, &sigset, 0 );
#  endif
#  ifdef _PTHREADS
  pthread_sigmask( SIG_UNBLOCK, &sigset, 0 );
#  endif
#endif // __unix
}

__FIT_DECLSPEC
int signal_handler( int sig, SIG_PF handler )
{
#ifdef __unix  // catch SIGPIPE here
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

__FIT_DECLSPEC
int signal_handler( int sig, siginfo_handler_type handler )
{
#ifdef __unix  // catch SIGPIPE here
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

__FIT_DECLSPEC
void fork() throw( fork_in_parent, std::runtime_error )
{
#ifdef __unix
  // MT_REENTRANT( detail::_F_lock, _1 );
  fork_in_parent f( ::fork() );
  if ( f.pid() > 0 ) {
    throw f;
  }
  if ( f.pid() == -1 ) {
    throw std::runtime_error( msg2 );
  }
  detail::_ppid = detail::_pid;
  detail::_pid = syscall( SYS_getpid );
#endif
}

__FIT_DECLSPEC
void become_daemon() throw( fork_in_parent, std::runtime_error )
{
#ifdef __unix
  try {
    xmt::fork();

    chdir( "/var/tmp" ); // for CWD: if not done, process remain with same WD
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

// #ifdef __GNUC__
// void Thread::_create( const void *p, size_t psz )
// #else
void Thread::_create( const void *p, size_t psz ) throw(std::runtime_error)
// #endif
{
  if ( psz > sizeof(void *) ) { // can't pass on pointer
    // Hey, deallocation SHOULD be either in this method, or in _call ONLY,
    // and never more!    
    _param = new char [psz];
#ifdef WIN32
    memcpy( _param, p, psz );
#else
    std::memcpy( _param, p, psz );
#endif
  } else {
    _param = const_cast<void *>(p);
  }
  _param_sz = psz;

  int err = 0;
#ifdef _PTHREADS
  pthread_attr_t attr;
  if ( _flags != 0 || _stack_sz != 0 ) {
    pthread_attr_init( &attr ); // pthread_attr_create --- HP-UX 10.20
    // pthread_attr_setstacksize( &attr, 0x80000 ); // min 0x50000
    if ( _flags & bound ) {
      pthread_attr_setscope( &attr, bound ); // PTHREAD_SCOPE_PROCESS
    }
    if ( _flags & detached ) { // the same as daemon: detached == daemon for POSIX
      pthread_attr_setdetachstate( &attr, detached ); // PTHREAD_CREATE_DETACHED
    }
    if ( _stack_sz != 0 ) {      
      _stack_sz = std::max( static_cast<size_t>(PTHREAD_STACK_MIN), _stack_sz );
      pthread_attr_setstacksize( &attr, _stack_sz /* PTHREAD_STACK_MIN * 2 */ );
    }
    // pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    // pthread_attr_setschedpolicy(&attr,SCHED_OTHER);
  }
  err = pthread_create( &_id, _flags != 0 || _stack_sz != 0 ? &attr : 0, _xcall, this );
  if ( err != 0 ) {
    _rip_id = _id = bad_thread_id;
  } else {
    _rip_id = _id;
  }
  if ( _flags != 0 || _stack_sz != 0 ) {
    pthread_attr_destroy( &attr );
  }
#endif
#ifdef __FIT_UITHREADS
  err = thr_create( 0, 0, _xcall, this, _flags, &_id );
#endif
#ifdef __FIT_WIN32THREADS
   _rip_id = _id = CreateThread( 0, 0, _xcall, this, (_flags & suspended), &_thr_id );
  err = GetLastError();
#endif

  if ( err != 0 ) {
    if ( psz > sizeof(void *) ) { // clear allocated here
      delete [] __STATIC_CAST(char *,_param);
    }
    std::stringstream s;
    s << msg1 << " error " << err << endl;
    throw std::runtime_error( s.str() );
  }
}

#ifdef _WIN32
#pragma warning( disable : 4101 )
#endif

extern "C" {
#ifdef __unix
  void *_xcall( void *p )
  {
    return Thread::_call( p );
  }
#endif
#ifdef WIN32
  unsigned long __stdcall _xcall( void *p )
  {
    return (unsigned long)Thread::_call( p );
  }
#endif
#ifdef __FIT_NETWARE
  void _xcall( void *p )
  {
    Thread::_call( p );
  }
#endif
} // extern "C"

void *Thread::_call( void *p )
{
  Thread *me = static_cast<Thread *>(p);

  // After exit of me->_entrance, there is may be no more *me itself,
  // so it's members may be unaccessible. Don't use me->"*" after call
  // of me->_entrance!!!
  void *_param     = me->_param;
  size_t _param_sz = me->_param_sz;
  ret_code ret;

//#ifdef _PTHREADS
//#  ifndef __hpux
//  if ( me->_flags & (daemon | detached) ) {
//    pthread_detach( me->_id );
//  }
//#  endif
//#endif

#ifdef WIN32
  set_unexpected( unexpected );
  set_terminate( terminate );
#else
# ifndef __FIT_NETWARE
  std::set_unexpected( Thread::unexpected );
  std::set_terminate( Thread::terminate );
# endif // !__FIT_NETWARE
#endif

  me->pword( _self_idx ) = me; // to have chance make Thread sanity by signal
  // In most cases for Linux there are problems with signals processing,
  // so I don't set it default more
  // signal_handler( SIGTERM, signal_exit ); // set handler for sanity
  try {
    ret = me->_entrance( _param );

    // I should make me->_id = bad_thread_id; here...
    // This is in conflict what I say in the begin of this function.
    // So don't delete Thread before it termination!

    if ( (me->_flags & (daemon | detached)) != 0 ) { // otherwise join expected
      // Locker lk( me->_llock );
#ifdef __FIT_WIN32THREADS
      CloseHandle( me->_id );
#endif
      me->_id = bad_thread_id;
      me->_rip_id = bad_thread_id;
    } else {
      // Locker lk( me->_llock );
      me->_id = bad_thread_id;
    }
    me->_dealloc_uw(); // free user words
  }
  catch ( std::exception& e ) {
    if ( (me->_flags & (daemon | detached)) != 0 ) { // otherwise join expected
      // Locker lk( me->_llock );
#ifdef __FIT_WIN32THREADS
      CloseHandle( me->_id );
#endif
      me->_id = bad_thread_id;
      me->_rip_id = bad_thread_id;
    } else {
      // Locker lk( me->_llock );
      me->_id = bad_thread_id;
    }
    me->_dealloc_uw(); // free user words
#ifndef _WIN32
    cerr << e.what() << endl;
#endif
    ret.iword = -1;
  }
  catch ( int sig ) {
    if ( (me->_flags & (daemon | detached)) != 0 ) { // otherwise join expected
      // Locker lk( me->_llock );
#ifdef __FIT_WIN32THREADS
      CloseHandle( me->_id );
#endif
      me->_id = bad_thread_id;
      me->_rip_id = bad_thread_id;
    } else {
      // Locker lk( me->_llock );
      me->_id = bad_thread_id;
    }

    me->_dealloc_uw(); // free user words
    // const char *_sig_ = strsignal( sig );
#ifndef _WIN32
    cerr << "\n--- Thread: signal " << sig /* (_sig_ ? _sig_ : "unknown") */ << " detected ---" << endl;
#endif
    ret.iword = sig;
  }
  catch ( ... ) {
    if ( (me->_flags & (daemon | detached)) != 0 ) { // otherwise join expected
      // Locker lk( me->_llock );
#ifdef __FIT_WIN32THREADS
      CloseHandle( me->_id );
#endif
      me->_id = bad_thread_id;
      me->_rip_id = bad_thread_id;
    } else {
      // Locker lk( me->_llock );
      me->_id = bad_thread_id;
    }
    me->_dealloc_uw(); // free user words
#ifndef _WIN32
    cerr << "\n--- Thread: unknown exception occur ---" << endl;
#endif
    ret.iword = -1;
  }

  try {
    if ( _param_sz > sizeof(void *) ) { // that's allocated
      delete [] __STATIC_CAST(char *,_param);
      _param_sz = 0;
      _param = 0;
    }
  }
  catch ( ... ) {
    ret.iword = -1;
  }

#if defined( __SUNPRO_CC ) && defined( __i386 )
  Thread::_exit( ret.iword );
#endif

  return ret.pword;
}
#ifdef _WIN32
#pragma warning( default : 4101 )
#endif

void Thread::unexpected()
{
#ifndef _WIN32
  cerr << "\nUnexpected exception, catched here: " << __FILE__ << ':' << __LINE__ << endl;
#endif
  signal_exit( SIGTERM );  // Thread::_exit( 0 );
}

void Thread::terminate()
{
#ifndef _WIN32
  cerr << "\nTerminate exception, catched here: " << __FILE__ << ':' << __LINE__<< endl;
#endif
  signal_exit( SIGTERM );  // Thread::_exit( 0 );
}

int Thread::xalloc()
{
  Locker _l( _idx_lock );
  return _idx++;
}

pid_t getpid()
{
  return detail::_pid;
}

pid_t getppid()
{
  return detail::_ppid;
}

} // namespace xmt
