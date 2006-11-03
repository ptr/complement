// -*- C++ -*- Time-stamp: <06/10/24 09:24:01 ptr>

/*
 * Copyright (c) 1997-1999, 2002-2006
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __XMT_H
#define __XMT_H

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <memory>
#include <cstddef>
#include <stdexcept>

#if !defined(_STLPORT_VERSION) && !defined(__STATIC_CAST)
# define __STATIC_CAST(t,v) static_cast<t>(v)
#endif

#ifdef WIN32
# include <windows.h>
# include <memory>
# include <limits>
# define ETIME   62      /* timer expired */
# pragma warning( disable : 4290)
#endif // WIN32

#ifdef __unix
# if defined( _REENTRANT ) && !defined(_NOTHREADS)
#  ifdef _PTHREADS
#   include <pthread.h>
#   include <semaphore.h>
#  else
#   include <thread.h>
#   include <synch.h>
#  endif
# elif !defined(_NOTHREADS) // !_REENTRANT
#  define _NOTHREADS
# endif
// #  define __DLLEXPORT
#endif // __unix

#ifdef __FIT_NOVELL_THREADS // Novell NetWare
# if defined( _REENTRANT ) && !defined(_NOTHREADS)
#  include <nwthread.h>
#  include <nwsemaph.h>
#  include <nwproc.h>
# elif !defined(_NOTHREADS) // !_REENTRANT
#  define _NOTHREADS
# endif
#endif

#include <cerrno>

#include <mt/time.h>

#ifdef _REENTRANT

# define MT_REENTRANT(point,nm) xmt::Locker nm(point)
# define MT_REENTRANT_RS(point,nm) xmt::LockerRS nm(point)
# define MT_REENTRANT_SDS(point,nm) xmt::LockerSDS nm(point) // obsolete, use MT_REENTRANT_RS
# define MT_LOCK(point)         point.lock()
# define MT_UNLOCK(point)       point.unlock()
# ifdef __FIT_RWLOCK
#  define MT_REENTRANT_RD(point,nm) xmt::LockerRd nm(point)
#  define MT_REENTRANT_WR(point,nm) xmt::LockerWr nm(point)
#  define MT_LOCK_RD(point)      point.rdlock()
#  define MT_LOCK_WR(point)      point.wrlock()
# else // !__FIT_RWLOCK
#  define MT_REENTRANT_RD(point,nm) ((void)0)
#  define MT_REENTRANT_WR(point,nm) ((void)0)
#  define MT_LOCK_RD(point) ((void)0)
#  define MT_LOCK_WR(point) ((void)0)
# endif // __FIT_RWLOCK

#else // !_REENTRANT

# define MT_REENTRANT(point,nm) ((void)0)
# define MT_REENTRANT_RS(point,nm) ((void)0)
# define MT_REENTRANT_SDS(point,nm) ((void)0) // obsolete, use MT_REENTRANT_RS
# define MT_LOCK(point)         ((void)0)
# define MT_UNLOCK(point)       ((void)0)
# define MT_REENTRANT_RD(point,nm) ((void)0)
# define MT_REENTRANT_WR(point,nm) ((void)0)
# define MT_LOCK_RD(point) ((void)0)
# define MT_LOCK_WR(point) ((void)0)

#endif // _REENTRANT

#include <signal.h>

extern "C" {

#ifndef SIG_PF // sys/signal.h

# ifdef WIN32
typedef void __cdecl SIG_FUNC_TYP(int);
# else
typedef void SIG_FUNC_TYP(int);
# endif
typedef SIG_FUNC_TYP *SIG_TYP;
# define SIG_PF SIG_TYP

# ifndef SIG_DFL
#  define SIG_DFL (SIG_PF)0
# endif
# ifndef SIG_ERR
#  define SIG_ERR (SIG_PF)-1
# endif
# ifndef SIG_IGN
#  define SIG_IGN (SIG_PF)1
# endif
# ifndef SIG_HOLD
#  define SIG_HOLD (SIG_PF)2
# endif
#endif // SIG_PF

} // extern "C"

namespace xmt {

namespace detail {

#ifdef __FIT_PSHARED_MUTEX
extern std::string _notpshared;
#endif

#ifdef __FIT_XSI_THR
extern std::string _notrecursive;
#endif

} // namespace detail

// extern __FIT_DECLSPEC void signal_throw( int sig ) throw( int );
// extern __FIT_DECLSPEC void signal_thread_exit( int sig );

#ifdef __unix
extern "C"  void *_xcall( void * ); // forward declaration
#endif
#ifdef WIN32
extern "C" unsigned long __stdcall _xcall( void *p ); // forward declaration
#endif

#ifndef WIN32
// using std::size_t;
#endif
#ifdef __GNUC__
  // using std::runtime_error;
#else
using std::runtime_error;
#endif

#ifndef _WIN32
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
#endif // !_WIN32


template <bool SCOPE> class __Condition;

// if parameter SCOPE (process scope) true, PTHREAD_PROCESS_SHARED will
// be used; otherwise PTHREAD_PROCESS_PRIVATE.
// Of cause, system must support process scope...
// Linux at 2003-01-19 NOT SUPPORT PTHREAD_PROCESS_SHARED mutex!
// And Windows too!
// 
template <bool RECURSIVE_SAFE, bool SCOPE>
class __mutex_base
{
  public:
    __mutex_base()
      {
#ifdef _PTHREADS
        if ( SCOPE || RECURSIVE_SAFE ) {
          pthread_mutexattr_t att;
          pthread_mutexattr_init( &att );
# ifdef __FIT_PSHARED_MUTEX
          if ( SCOPE ) {
            if ( pthread_mutexattr_setpshared( &att, PTHREAD_PROCESS_SHARED ) != 0 ) {
              throw std::invalid_argument( detail::_notpshared );
            }
          }
# endif // __FIT_PSHARED_MUTEX
# ifdef __FIT_XSI_THR  // Unix 98 or X/Open System Interfaces Extention
          if ( RECURSIVE_SAFE ) {
            if ( pthread_mutexattr_settype( &att, PTHREAD_MUTEX_RECURSIVE ) != 0 ) {
              throw std::invalid_argument( detail::_notrecursive );
            }
          }
# endif
          pthread_mutex_init( &_M_lock, &att );
          pthread_mutexattr_destroy( &att );
        } else {
          pthread_mutex_init( &_M_lock, 0 );
        }
#endif // _PTHREADS
#ifdef __FIT_UITHREADS
        if ( SCOPE ) {
          // or USYNC_PROCESS_ROBUST to detect already initialized mutex
          // in process scope
          mutex_init( &_M_lock, USYNC_PROCESS, 0 );
        } else {
          mutex_init( &_M_lock, 0, 0 );
        }
#endif
#ifdef __FIT_WIN32THREADS
        InitializeCriticalSection( &_M_lock );
#endif
#ifdef __FIT_NOVELL_THREADS
        _M_lock = OpenLocalSemaphore( 1 );
#endif
      }

    ~__mutex_base()
      {
#ifdef _PTHREADS
        pthread_mutex_destroy( &_M_lock );
#endif
#ifdef __FIT_UITHREADS
        mutex_destroy( &_M_lock );
#endif
#ifdef WIN32
        DeleteCriticalSection( &_M_lock );
#endif
#ifdef __FIT_NOVELL_THREADS
        CloseLocalSemaphore( _M_lock );
#endif
      }

  private:
    __mutex_base( const __mutex_base& )
      { }

  protected:
#ifdef _PTHREADS
    pthread_mutex_t _M_lock;
#endif
#ifdef __FIT_UITHREADS
    mutex_t _M_lock;
#endif
#ifdef __FIT_WIN32THREADS
    CRITICAL_SECTION _M_lock;
#endif
#ifdef __FIT_NOVELL_THREADS
    // This is for ...LocalSemaphore() calls
    // Alternative is EnterCritSec ... ExitCritSec; but ...CritSec in Novell
    // block all threads except current
    LONG _M_lock;
#endif

#ifndef __FIT_WIN32THREADS
  private:
    friend class __Condition<SCOPE>;
#endif
};

#ifdef __FIT_PTHREAD_SPINLOCK

// The IEEE Std. 1003.1j-2000 introduces functions to implement spinlocks.
template <bool SCOPE>
class __spinlock_base
{
  public:
    __spinlock_base()
      {
#ifdef _PTHREADS
        pthread_spin_init( &_M_lock, SCOPE ? 1 : 0 );
#endif // _PTHREADS
      }

    ~__spinlock_base()
      {
#ifdef _PTHREADS
        pthread_spin_destroy( &_M_lock );
#endif
      }
  protected:
#ifdef _PTHREADS
    pthread_spinlock_t _M_lock;
#endif
};

#endif // __FIT_PTHREAD_SPINLOCK

// Portable Mutex implementation. If the parameter RECURSIVE_SAFE
// is true, Mutex will be recursive safe (detect deadlock).
// If RECURSIVE_SAFE is false, implementation may not to be
// recursive-safe.
// The SCOPE parameter designate Mutex scope---shared between
// processes (true), or only inside threads of one process (false).
// Note, that not all OS support interprocess mutex scope
// (for example, Windows and Linux).
template <bool RECURSIVE_SAFE, bool SCOPE>
class __Mutex :
    public __mutex_base<RECURSIVE_SAFE,SCOPE>
{
  public:
    __Mutex()
      { }

    ~__Mutex()
      { }

    void lock()
      {
#ifdef _PTHREADS
        pthread_mutex_lock( &this->_M_lock );
#endif
#ifdef __FIT_UITHREADS
        mutex_lock( &this->_M_lock );
#endif
#ifdef __FIT_WIN32THREADS
        EnterCriticalSection( &this->_M_lock );
#endif
#ifdef __FIT_NOVELL_THREADS
        WaitOnLocalSemaphore( this->_M_lock );
#endif
      }

#if !defined( WIN32 ) || (defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400)
    int trylock()
      {
#ifdef _PTHREADS
        return pthread_mutex_trylock( &this->_M_lock );
#endif
#ifdef __FIT_UITHREADS
        return mutex_trylock( &this->_M_lock );
#endif
#ifdef __FIT_WIN32THREADS
        return TryEnterCriticalSection( &this->_M_lock ) != 0 ? 0 : -1;
#endif
#ifdef __FIT_NOVELL_THREADS
        return ExamineLocalSemaphore( this->_M_lock ) > 0 ? WaitOnLocalSemaphore( this->_M_lock ) : -1;
#endif
#ifdef _NOTHREADS
        return 0;
#endif
      }
#endif // !WIN32 || _WIN32_WINNT >= 0x0400

    void unlock()
      {
#ifdef _PTHREADS
        pthread_mutex_unlock( &this->_M_lock );
#endif
#ifdef __FIT_UITHREADS
        mutex_unlock( &this->_M_lock );
#endif
#ifdef __FIT_WIN32THREADS
        LeaveCriticalSection( &this->_M_lock );
#endif
#ifdef __FIT_NOVELL_THREADS
        SignalLocalSemaphore( this->_M_lock );
#endif
      }

  private:
    __Mutex( const __Mutex& )
      { }

#ifndef __FIT_WIN32THREADS
  private:
    friend class __Condition<SCOPE>;
#endif
};

#ifdef __FIT_PTHREAD_SPINLOCK
// Spinlock-based locks (IEEE Std. 1003.1j-2000)

template <bool RS, bool SCOPE> class __Spinlock;

template <bool SCOPE>
class __Spinlock<false,SCOPE> :
    public __spinlock_base<SCOPE>
{
  public:
    __Spinlock()
      { }

    ~__Spinlock()
      { }

    void lock()
      {
# ifdef _PTHREADS
        pthread_spin_lock( &this->_M_lock );
# endif
      }

    int trylock()
      {
# ifdef _PTHREADS
        return pthread_spin_trylock( &this->_M_lock );
# endif
# ifdef _NOTHREADS
        return 0;
# endif
      }

    void unlock()
      {
# ifdef _PTHREADS
        pthread_spin_unlock( &this->_M_lock );
# endif
      }
};

template <bool SCOPE>
class __Spinlock<true,SCOPE> : //  Recursive safe
    public __spinlock_base<SCOPE>
{
  public:
    __Spinlock()
      { }

    ~__Spinlock()
      { }

    void lock()
      {
# ifndef _NOTHREADS
#  ifdef _PTHREADS
        pthread_t _c_id = pthread_self();
#  endif
#  ifdef __FIT_UITHREADS
        thread_t _c_id = thr_self();
#  endif
#  ifdef __FIT_NOVELL_THREADS
        int _c_id = GetThreadID();
#  endif
        if ( _c_id == _id ) {
          ++_count;
          return;
        }
#  ifdef _PTHREADS
        pthread_spin_lock( &this->_M_lock );
#  endif
        _id = _c_id;
        _count = 0;
# endif // !_NOTHREADS
      }

    int trylock()
      {
# ifdef _NOTHREADS
        return 0;
# else // _NOTHREADS
#  ifdef _PTHREADS
        pthread_t _c_id = pthread_self();
#  endif
#  ifdef __FIT_UITHREADS
        thread_t _c_id = thr_self();
#  endif
#  ifdef __FIT_NOVELL_THREADS
        int _c_id = GetThreadID();
#  endif
        if ( _c_id == _id ) {
          ++_count;
          return 0;
        }
#  ifdef _PTHREADS
        int res = pthread_spin_trylock( &this->_M_lock );
#  endif
        if ( res != 0 ) {
          return res;
        }

        _id = _c_id;
        _count = 0;

        return 0;
# endif // !_NOTHREADS
      }

    void unlock()
      {
# ifndef _NOTHREADS
        if ( --_count == 0 ) {
#  ifdef _PTHREADS
          _id =  __STATIC_CAST(pthread_t,-1);
          pthread_spin_unlock( &this->_M_lock );
#  endif
# endif // !_NOTHREADS
        }
      }
  protected:
# ifndef _NOTHREADS
    unsigned _count;
# endif // !_NOTHREADS

# ifdef _PTHREADS
    pthread_t _id;
# endif
# ifdef __FIT_UITHREADS
    thread_t  _id;
# endif
# ifdef __FIT_NOVELL_THREADS
    int _id;
# endif
};
#endif // __FIT_PTHREAD_SPINLOCK

// Recursive Safe mutex.

// This specialization need for POSIX and DCE threads,
// because Windows CriticalSection is recursive safe.
// By the way, point for enhancement:  __UNIX_98
// (or XSI---X/Open System Interfaces Extention) has recursive mutex option.
// Another specialization?

#if (defined(__unix) && !defined(__FIT_XSI_THR)) || defined(__FIT_NOVELL_THREADS)

// This specialization need for old POSIX and DCE threads,
// before XSI (X/Open System Interfaces Extention) or Unix 98.
// because Windows CriticalSection is recursive safe, and
// XSI implementation has appropriate mutex parameter (see
// __mutex_base above).

template <bool SCOPE>
class __Mutex<true,SCOPE> : // Recursive Safe
    public __mutex_base<true,SCOPE>
{
  public:
    __Mutex() :
        _count( 0 ),
# ifdef __FIT_UITHREADS
        _id( __STATIC_CAST(thread_t,-1) )
# endif
# ifdef _PTHREADS
        _id( __STATIC_CAST(pthread_t,-1) )
# endif
# ifdef __FIT_NOVELL_THREADS
        _id( -1 )
# endif
      { }

    ~__Mutex()
      { }

    void lock()
      {
# ifndef _NOTHREADS
#  ifdef _PTHREADS
        pthread_t _c_id = pthread_self();
#  endif
#  ifdef __FIT_UITHREADS
        thread_t _c_id = thr_self();
#  endif
#  ifdef __FIT_NOVELL_THREADS
        int _c_id = GetThreadID();
#  endif
        if ( _c_id == _id ) {
          ++_count;
          return;
        }
#  ifdef _PTHREADS
        pthread_mutex_lock( &_M_lock );
#  endif
#  ifdef __FIT_UITHREADS
        mutex_lock( &_M_lock );
#  endif
#  ifdef __FIT_NOVELL_THREADS
        WaitOnLocalSemaphore( this->_M_lock );
#  endif
        _id = _c_id;
        _count = 0;
# endif // !_NOTHREADS
      }

    // Equivalent to lock(), except that if the mutex object referenced
    // by mutex is currently locked the call return immediately.
    // If mutex is currently owned by the calling thread, the mutex lock count
    // incremented by one and the trylock() function immediately return success
    // (value 0). Otherwise, if mutex is currently owned by another thread,
    // return error (non-zero).

    int trylock()
      {
# ifdef _NOTHREADS
        return 0;
# else // _NOTHREADS
#  ifdef _PTHREADS
        pthread_t _c_id = pthread_self();
#  endif
#  ifdef __FIT_UITHREADS
        thread_t _c_id = thr_self();
#  endif
#  ifdef __FIT_NOVELL_THREADS
        int _c_id = GetThreadID();
#  endif
        if ( _c_id == _id ) {
          ++_count;
          return 0;
        }
#  ifdef _PTHREADS
        int res = pthread_mutex_trylock( &_M_lock );
#  endif
#  ifdef __FIT_UITHREADS
        int res = mutex_trylock( &_M_lock );
#  endif
#  ifdef __FIT_NOVELL_THREADS
        int res = ExamineLocalSemaphore( this->_M_lock ) > 0 ? WaitOnLocalSemaphore( this->_M_lock ) : -1;
#  endif
        if ( res != 0 ) {
          return res;
        }

        _id = _c_id;
        _count = 0;

        return 0;
# endif // !_NOTHREADS
      }

    void unlock()
      {
# ifndef _NOTHREADS
        if ( --_count == 0 ) {
#  ifdef __FIT_UITHREADS
          _id = __STATIC_CAST(thread_t,-1);
          mutex_unlock( &_M_lock );
#  endif
#  ifdef _PTHREADS
          _id =  __STATIC_CAST(pthread_t,-1);
          pthread_mutex_unlock( &_M_lock );
#  endif
#  ifdef __FIT_NOVELL_THREADS
          _id = -1;
          SignalLocalSemaphore( this->_M_lock );
#  endif
# endif // !_NOTHREADS
        }
      }

  private:
    __Mutex( const __Mutex& )
      { }

  protected:
# ifndef _NOTHREADS
    unsigned _count;
# endif // !_NOTHREADS

# ifdef _PTHREADS
    pthread_t _id;
# endif
# ifdef __FIT_UITHREADS
    thread_t  _id;
# endif
# ifdef __FIT_NOVELL_THREADS
    int _id;
# endif
};
#endif // __unix && !__FIT_XSI_THR

#ifdef __FIT_RWLOCK
// Read-write mutex: IEEE Std 1003.1, 2001, 2004 Editions

template <bool SCOPE>
class __mutex_rw_base
{
  public:
    __mutex_rw_base()
      {
#ifdef _PTHREADS
        if ( SCOPE ) {
          pthread_rwlockattr_t att;
          pthread_rwlockattr_init( &att );
# ifdef __FIT_PSHARED_MUTEX
          if ( pthread_rwlockattr_setpshared( &att, PTHREAD_PROCESS_SHARED ) != 0 ) {
            throw std::invalid_argument( xmt::detail::_notpshared );
          }
# endif // __FIT_PSHARED_MUTEX
          pthread_rwlock_init( &_M_lock, &att );
          pthread_rwlockattr_destroy( &att );
        } else {
          pthread_rwlock_init( &_M_lock, 0 );
        }
#endif // _PTHREADS
#ifdef __FIT_UITHREADS
#error Fix me!
        if ( SCOPE ) {
          // or USYNC_PROCESS_ROBUST to detect already initialized mutex
          // in process scope
          mutex_init( &_M_lock, USYNC_PROCESS, 0 );
        } else {
          mutex_init( &_M_lock, 0, 0 );
        }
#endif
#ifdef __FIT_WIN32THREADS
#error Fix me!
        InitializeCriticalSection( &_M_lock );
#endif
#ifdef __FIT_NOVELL_THREADS
#error Fix me!
        _M_lock = OpenLocalSemaphore( 1 );
#endif
      }

    ~__mutex_rw_base()
      {
#ifdef _PTHREADS
        pthread_rwlock_destroy( &_M_lock );
#endif
#ifdef __FIT_UITHREADS
#error Fix me!
        mutex_destroy( &_M_lock );
#endif
#ifdef WIN32
#error Fix me!
        DeleteCriticalSection( &_M_lock );
#endif
#ifdef __FIT_NOVELL_THREADS
#error Fix me!
        CloseLocalSemaphore( _M_lock );
#endif
      }

  private:
    __mutex_rw_base( const __mutex_rw_base& )
      { }

  protected:
#ifdef _PTHREADS
    pthread_rwlock_t _M_lock;
#endif
#ifdef __FIT_UITHREADS
#error Fix me!
    mutex_t _M_lock;
#endif
#ifdef __FIT_WIN32THREADS
#error Fix me!
    CRITICAL_SECTION _M_lock;
#endif
#ifdef __FIT_NOVELL_THREADS
    // This is for ...LocalSemaphore() calls
    // Alternative is EnterCritSec ... ExitCritSec; but ...CritSec in Novell
    // block all threads except current
#error Fix me!
    LONG _M_lock;
#endif
};

template <bool SCOPE>
class __MutexRW :
    public __mutex_rw_base<SCOPE>
{
  public:
    __MutexRW()
      { }

    ~__MutexRW()
      { }

    void rdlock()
      {
#ifdef _PTHREADS
        pthread_rwlock_rdlock( &this->_M_lock );
#endif
#ifdef __FIT_UITHREADS
#error Fix me!
        mutex_lock( &this->_M_lock );
#endif
#ifdef __FIT_WIN32THREADS
#error Fix me!
        EnterCriticalSection( &this->_M_lock );
#endif
#ifdef __FIT_NOVELL_THREADS
#error Fix me!
        WaitOnLocalSemaphore( this->_M_lock );
#endif
      }

    void wrlock()
      {
#ifdef _PTHREADS
        pthread_rwlock_wrlock( &this->_M_lock );
#endif
#ifdef __FIT_UITHREADS
#error Fix me!
        mutex_lock( &this->_M_lock );
#endif
#ifdef __FIT_WIN32THREADS
#error Fix me!
        EnterCriticalSection( &this->_M_lock );
#endif
#ifdef __FIT_NOVELL_THREADS
#error Fix me!
        WaitOnLocalSemaphore( this->_M_lock );
#endif
      }

#if !defined( WIN32 ) || (defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400)
    int tryrdlock()
      {
#ifdef _PTHREADS
        return pthread_rwlock_tryrdlock( &this->_M_lock );
#endif
#ifdef __FIT_UITHREADS
#error Fix me!
        return mutex_trylock( &this->_M_lock );
#endif
#ifdef __FIT_WIN32THREADS
#error Fix me!
        return TryEnterCriticalSection( &this->_M_lock ) != 0 ? 0 : -1;
#endif
#ifdef __FIT_NOVELL_THREADS
#error Fix me!
        return ExamineLocalSemaphore( this->_M_lock ) > 0 ? WaitOnLocalSemaphore( this->_M_lock ) : -1;
#endif
#ifdef _NOTHREADS
#error Fix me!
        return 0;
#endif
      }

    int trywrlock()
      {
#ifdef _PTHREADS
        return pthread_rwlock_trywrlock( &this->_M_lock );
#endif
#ifdef __FIT_UITHREADS
#error Fix me!
        return mutex_trylock( &this->_M_lock );
#endif
#ifdef __FIT_WIN32THREADS
#error Fix me!
        return TryEnterCriticalSection( &this->_M_lock ) != 0 ? 0 : -1;
#endif
#ifdef __FIT_NOVELL_THREADS
#error Fix me!
        return ExamineLocalSemaphore( this->_M_lock ) > 0 ? WaitOnLocalSemaphore( this->_M_lock ) : -1;
#endif
#ifdef _NOTHREADS
#error Fix me!
        return 0;
#endif
      }

#endif // !WIN32 || _WIN32_WINNT >= 0x0400

    void unlock()
      {
#ifdef _PTHREADS
        pthread_rwlock_unlock( &this->_M_lock );
#endif
#ifdef __FIT_UITHREADS
#error Fix me!
        mutex_unlock( &this->_M_lock );
#endif
#ifdef __FIT_WIN32THREADS
#error Fix me!
        LeaveCriticalSection( &this->_M_lock );
#endif
#ifdef __FIT_NOVELL_THREADS
#error Fix me!
        SignalLocalSemaphore( this->_M_lock );
#endif
      }

  private:
    __MutexRW( const __MutexRW& )
      { }
};

#endif // __FIT_RWLOCK

template <class M>
class __Locker
{
  public:
    __Locker( const M& point ) :
      m( point )
      { const_cast<M&>(m).lock(); }
    ~__Locker()
      { const_cast<M&>(m).unlock(); }

  private:
    __Locker( const __Locker& )
      { }
    const M& m;
};

#ifdef __FIT_RWLOCK
template <bool SCOPE>
class __LockerRd
{
  public:
    __LockerRd( const __MutexRW<SCOPE>& point ) :
      m( point )
      { const_cast<__MutexRW<SCOPE>&>(m).rdlock(); }
    ~__LockerRd()
      { const_cast<__MutexRW<SCOPE>&>(m).unlock(); }

  private:
    __LockerRd( const __LockerRd& )
      { }
    const __MutexRW<SCOPE>& m;
};

template <bool SCOPE>
class __LockerWr
{
  public:
    __LockerWr( const __MutexRW<SCOPE>& point ) :
      m( point )
      { const_cast<__MutexRW<SCOPE>&>(m).wrlock(); }
    ~__LockerWr()
      { const_cast<__MutexRW<SCOPE>&>(m).unlock(); }

  private:
    __LockerWr( const __LockerWr& )
      { }
    const __MutexRW<SCOPE>& m;
};
#endif // __FIT_RWLOCK

typedef __Mutex<false,false>  Mutex;
typedef __Mutex<true,false>   MutexRS;
typedef __Mutex<true,false>   MutexSDS; // obsolete, use instead MutexRS
#ifdef __FIT_RWLOCK
typedef __MutexRW<false>      MutexRW;
#endif // __FIT_RWLOCK
#ifdef __FIT_PTHREAD_SPINLOCK
typedef __Spinlock<false,false> Spinlock;
typedef __Spinlock<true,false>  SpinlockRS;
#endif // __FIT_RWLOCK

typedef __Locker<Mutex>       Locker;
typedef __Locker<MutexRS>     LockerRS;
typedef __Locker<MutexRS>     LockerSDS; // obsolete, use instead LockerRS
#ifdef __FIT_RWLOCK
typedef __LockerRd<false>     LockerRd;
typedef __LockerWr<false>     LockerWr;
#endif // __FIT_RWLOCK
#ifdef __FIT_PTHREAD_SPINLOCK
typedef __Locker<Spinlock>    LockerSpin;
typedef __Locker<SpinlockRS>  LockerSpinRS;
#endif // __FIT_RWLOCK

class LockerExt
{
  public:
#ifdef _PTHREADS
    explicit LockerExt( const pthread_mutex_t& m ) :
#endif
#ifdef __FIT_UITHREADS
    explicit LockerExt( const mutex_t& m ) :
#endif
#ifdef __FIT_WIN32THREADS
    explicit LockerExt( const CRITICAL_SECTION& m ) :
#endif
#ifdef __FIT_NOVELL_THREADS
    explicit LockerExt( const LONG& m ) :
#endif
        _M_lock( m )
      {
#ifdef _PTHREADS
        pthread_mutex_lock( const_cast<pthread_mutex_t *>(&_M_lock) );
#endif
#ifdef __FIT_UITHREADS
        mutex_lock( const_cast<mutex_t *>(&_M_lock) );
#endif
#ifdef __FIT_WIN32THREADS
        EnterCriticalSection( const_cast<CRITICAL_SECTION *>(&_M_lock) );
#endif
#ifdef __FIT_NOVELL_THREADS
        WaitOnLocalSemaphore( const_cast<LONG&>(_M_lock) );
#endif
      }

    ~LockerExt()
      {
#ifdef _PTHREADS
        pthread_mutex_unlock( const_cast<pthread_mutex_t *>(&_M_lock) );
#endif
#ifdef __FIT_UITHREADS
        mutex_unlock( const_cast<mutex_t *>(&_M_lock) );
#endif
#ifdef __FIT_WIN32THREADS
        LeaveCriticalSection( const_cast<CRITICAL_SECTION *>(&_M_lock) );
#endif
#ifdef __FIT_NOVELL_THREADS
        SignalLocalSemaphore( const_cast<LONG&>(_M_lock) );
#endif
      }

  private:
    LockerExt( const LockerExt& m ) :
        _M_lock( m._M_lock )
      { }
#ifdef _PTHREADS
    const pthread_mutex_t& _M_lock;
#endif
#ifdef __FIT_UITHREADS
    const mutex_t& _M_lock;
#endif
#ifdef __FIT_WIN32THREADS
    const CRITICAL_SECTION& _M_lock;
#endif
#ifdef __FIT_NOVELL_THREADS
    // This is for ...LocalSemaphore() calls
    // Alternative is EnterCritSec ... ExitCritSec; but ...CritSec in Novell
    // block all threads except current
    const LONG& _M_lock;
#endif
    
};

template <bool SCOPE>
class __Condition
{
  public:
    __Condition() :
        _val( true )
      {
#ifdef __FIT_WIN32THREADS
        _cond = CreateEvent( 0, TRUE, TRUE, 0 );
#endif
#ifdef _PTHREADS
        if ( SCOPE ) {
          pthread_condattr_t attr;
          pthread_condattr_init( &attr );
          pthread_condattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
          pthread_cond_init( &_cond, 0 );
          pthread_condattr_destroy( &attr );
        } else {
          pthread_cond_init( &_cond, 0 );
        }
#endif
#ifdef __FIT_UITHREADS
        cond_init( &_cond, 0, 0 );
#endif
#ifdef __FIT_NOVELL_THREADS
        _cond = OpenLocalSemaphore( 0 );
#endif
      }

    ~__Condition()
      {
#ifdef __FIT_WIN32THREADS
        CloseHandle( _cond );
#endif
#ifdef _PTHREADS
        pthread_cond_destroy( &_cond );
#endif
#ifdef __FIT_UITHREADS
        cond_destroy( &_cond );
#endif
#ifdef __FIT_NOVELL_THREADS
        CloseLocalSemaphore( _cond );
#endif
      }

    bool set( bool __v, bool _broadcast = false )
      {
        __Locker<__Mutex<false,SCOPE> > _x1( _lock );

        bool tmp = _val;
        _val = __v;
#ifdef __FIT_WIN32THREADS
         if ( __v == true && tmp == false ) {
           SetEvent( _cond );
         } else if ( __v == false && tmp == true ) {
           ResetEvent( _cond );
         }
#endif
#ifdef __FIT_UITHREADS
        if ( __v == true && tmp == false ) {
          if ( _broadcast ) {
            cond_broadcast( &_cond );
          } else {
            cond_signal( &_cond );
          }
        }
#endif
#ifdef _PTHREADS
        if ( __v == true && tmp == false ) {
          if ( _broadcast ) {
            pthread_cond_broadcast( &_cond );
          } else {
            pthread_cond_signal( &_cond );
          }
        }
#endif
#ifdef __FIT_NOVELL_THREADS
        if ( __v == true && tmp == false ) {
          if ( _broadcast ) {
            // Unimplemented
            // pthread_cond_broadcast( &_cond );
          } else {
            SignalLocalSemaphore( _cond );
          }
        }
#endif
        return tmp;
      }

    bool set() const
      { return _val; }

    int try_wait()
      {
#if defined(__FIT_WIN32THREADS) || defined(__FIT_NOVELL_THREADS)
        _lock.lock();
#endif
#if defined(__FIT_UITHREADS) || defined(_PTHREADS)
        __Locker<__Mutex<false,SCOPE> > _x1( _lock );
#endif
        if ( _val == false ) {
#ifdef __FIT_WIN32THREADS
          _lock.unlock();
          if ( WaitForSingleObject( _cond, -1 ) == WAIT_FAILED ) {
            return -1;
          }
          return 0;
#endif
#ifdef __FIT_NOVELL_THREADS
          _lock.unlock();
          return WaitOnLocalSemaphore( _cond );
#endif
#if defined(__FIT_UITHREADS) || defined(_PTHREADS)
          int ret = 0;
          while ( !_val ) {
            ret =
#ifdef _PTHREADS
              pthread_cond_wait( &_cond, &_lock._M_lock );
#endif
#ifdef __FIT_UITHREADS
              cond_wait( &_cond, &_lock._M_lock );
#endif
          }
          return ret;
#endif
        }
#if defined(__FIT_WIN32THREADS) || defined(__FIT_NOVELL_THREADS)
        _lock.unlock();
#endif
        return 0;
      }

    int wait()
      {
#ifdef __FIT_WIN32THREADS
        MT_LOCK( _lock );
        _val = false;
        ResetEvent( _cond );
        MT_UNLOCK( _lock );
        if ( WaitForSingleObject( _cond, -1 ) == WAIT_FAILED ) {
          return -1;
        }
        return 0;
#endif
#if defined(_PTHREADS) || defined(__FIT_UITHREADS)
        MT_REENTRANT( _lock, _x1 );
        _val = false;
        int ret;
        while ( !_val ) {
          ret =
#ifdef _PTHREADS
            pthread_cond_wait( &_cond, &_lock._M_lock );
#endif
#ifdef __FIT_UITHREADS
            cond_wait( &_cond, &_lock._M_lock );
#endif
        }

        return ret;
#endif
#ifdef __FIT_NOVELL_THREADS
        _lock.lock();
        _val = false;
        _lock.unlock();
        return WaitOnLocalSemaphore( _cond );
#endif
#ifdef _NOTHREADS
        return 0;
#endif
      }

    int wait_time( const timespec *abstime );
    int wait_delay( const timespec *abstime );
    int try_wait_time( const timespec *abstime );
    int try_wait_delay( const timespec *abstime );

    int signal( bool _broadcast = false )
      {
        __Locker<__Mutex<false,SCOPE> > _x1( _lock );

        _val = true;
#ifdef __FIT_WIN32THREADS
        return SetEvent( _cond ) == FALSE ? -1 : 0;
#endif
#ifdef _PTHREADS
        return _broadcast ? pthread_cond_broadcast( &_cond ) : pthread_cond_signal( &_cond );
#endif
#ifdef __FIT_UITHREADS
        return _broadcast ? cond_broadcast( &_cond ) : cond_signal( &_cond );
#endif
#ifdef __FIT_NOVELL_THREADS
        return SignalLocalSemaphore( _cond );
#endif
#ifdef _NOTHREADS
        return 0;
#endif
      }

  protected:
#ifdef __FIT_WIN32THREADS
    HANDLE _cond;
#endif
#ifdef _PTHREADS
    pthread_cond_t _cond;
#endif
#ifdef __FIT_UITHREADS
    cond_t _cond;
#endif
#ifdef __FIT_NOVELL_THREADS
    LONG _cond;
#endif
    __Mutex<false,SCOPE> _lock;
    bool _val;

  private:
    __Condition( const __Condition& )
      { }
};

typedef __Condition<false> Condition;

class Semaphore
{
  public:
    Semaphore( int cnt = 1, bool ipc = false )
      {
#ifdef __FIT_WIN32THREADS
        _sem = CreateSemaphore( NULL, cnt, INT_MAX, 0 ); // check!
        _cnt = cnt;
#endif
#ifdef __FIT_UITHREADS
        sema_init( &_sem, cnt, ipc ? USYNC_PROCESS : USYNC_THREAD, 0 );
#endif
#ifdef _PTHREADS
        sem_init( &_sem, ipc ? 1 : 0, cnt );
#endif
#ifdef __FIT_NOVELL_THREADS
        _sem = OpenLocalSemaphore( cnt );
#endif
      }

    ~Semaphore()
      {
#ifdef __FIT_WIN32THREADS
        CloseHandle( _sem );
#endif
#ifdef __FIT_UITHREADS
        sema_destroy( &_sem );
#endif
#ifdef _PTHREADS
        sem_destroy( &_sem );
#endif        
#ifdef __FIT_NOVELL_THREADS
        CloseLocalSemaphore( _sem );
#endif
      }

    int wait()
      {
#ifdef __FIT_WIN32THREADS
        --_cnt;
        if ( WaitForSingleObject( _sem, -1 ) == WAIT_FAILED ) {
          ++_cnt;
          return -1;
        }
        return 0;
#endif
#ifdef __FIT_UITHREADS
        return sema_wait( &_sem );
#endif
#ifdef _PTHREADS
        return sem_wait( &_sem );
#endif
#ifdef __FIT_NOVELL_THREADS
        return WaitOnLocalSemaphore( _sem );
#endif
      }

    __FIT_DECLSPEC int wait_time( const timespec *t ); // wait for time t, or signal
    __FIT_DECLSPEC int wait_delay( const timespec *t ); // wait, timeout is delay t, or signal

    int try_wait()
      {
#ifdef __FIT_WIN32THREADS
        return _cnt > 0 ? (--_cnt, this->wait()) : -1;
#endif
#ifdef __FIT_UITHREADS
        return sema_trywait( &_sem );
#endif
#ifdef _PTHREADS
        return sem_trywait( &_sem );
#endif        
#ifdef __FIT_NOVELL_THREADS
        return ExamineLocalSemaphore( _sem ) > 0 ? WaitOnLocalSemaphore( _sem ) : -1;
#endif
      }

    int post()
      {
#ifdef __FIT_WIN32THREADS
        return ReleaseSemaphore( _sem, 1, &_cnt ) != 0 ? (++_cnt, 0) : -1;
#endif
#ifdef __FIT_UITHREADS
        return sema_post( &_sem );
#endif
#ifdef _PTHREADS
        return sem_post( &_sem );
#endif        
#ifdef __FIT_NOVELL_THREADS
        return SignalLocalSemaphore( _sem );
#endif
      }

    int value()
      {
#ifdef __FIT_WIN32THREADS
        return static_cast<int>(_cnt);
#endif
#ifdef __FIT_UITHREADS
# warning "No semaphore value for Solaris threads!"
#endif
#ifdef _PTHREADS
        int v;
        int e = sem_getvalue( &_sem, &v );
        
        return e == 0 ? v : -1;
#endif        
#ifdef __FIT_NOVELL_THREADS
        return ExamineLocalSemaphore( _sem );
#endif        
      }

  protected:
#ifdef __FIT_WIN32THREADS
    HANDLE _sem;
    long _cnt;
#endif
#ifdef __FIT_UITHREADS
    sema_t _sem;
#endif
#ifdef _PTHREADS
    sem_t _sem;
#endif
#ifdef __FIT_NOVELL_THREADS
    LONG _sem;
#endif
  private:
    Semaphore( const Semaphore& )
      { }
};

class Thread
{
  public:
    union ret_code
    {
        void *pword;
        long  iword;
    };

    typedef ret_code (*entrance_type)( void * );
#ifdef __FIT_WIN32THREADS
    typedef unsigned long thread_key_type;
    typedef HANDLE thread_id_type;
    // typedef unsigned long thread_id_type;
#endif
#ifdef _PTHREADS
    typedef pthread_key_t thread_key_type;
    typedef pthread_t     thread_id_type;
#endif
#ifdef __FIT_UITHREADS
    typedef thread_key_t thread_key_type;
    typedef thread_t     thread_id_type;
#endif
#ifdef __FIT_NOVELL_THREADS
    typedef void * thread_key_type;
    typedef int    thread_id_type;
#endif

    enum {
      // thread mode flags
#ifdef __FIT_UITHREADS // __STL_SOLARIS_THREADS
      bound     = THR_BOUND,
      detached  = THR_DETACHED,
      new_lwp   = THR_NEW_LWP,
      suspended = THR_SUSPENDED,
      daemon    = THR_DAEMON,
#endif
#if defined(_PTHREADS)
      bound     = PTHREAD_SCOPE_SYSTEM,   // otherwise, PTHREAD_SCOPE_PROCESS, default
      detached  = PTHREAD_CREATE_DETACHED,// otherwise, PTHREAD_CREATE_JOINABLE, default
      new_lwp   = 0, // pthread_setconcurrency( pthread_getconcurrency() + 1 );
      suspended = 0,
      daemon    = detached,
#endif
#ifdef __FIT_WIN32THREADS
      bound     = 0,
      detached  = 0x2,
      new_lwp   = 0,
      suspended = CREATE_SUSPENDED,
      daemon    = detached,
#endif
#ifdef __FIT_NOVELL_THREADS
      bound     = 0,
      detached  = 0x2,
      new_lwp   = 0,
      suspended = 0,
      daemon    = detached,
#endif
      // state flags
      goodbit = 0x00,
      badbit  = 0x01
    };

    class Init
    {
      public:
        Init();
        ~Init();
      private:
        static int& _count;
    };

    __FIT_DECLSPEC Thread( unsigned flags = 0 );

    explicit __FIT_DECLSPEC Thread( entrance_type entrance, const void *p = 0, size_t psz = 0, unsigned flags = 0, size_t stack_sz = 0 );

    __FIT_DECLSPEC ~Thread();

    __FIT_DECLSPEC
    void launch( entrance_type entrance, const void *p = 0, size_t psz = 0, size_t stack_sz = 0 );

    __FIT_DECLSPEC ret_code join();
    __FIT_DECLSPEC int suspend();
    __FIT_DECLSPEC int resume();
    __FIT_DECLSPEC int kill( int sig );
#ifdef __FIT_UITHREADS
    static __FIT_DECLSPEC int join_all();
#endif
    static __FIT_DECLSPEC void block_signal( int sig );
    static __FIT_DECLSPEC void unblock_signal( int sig );
    static __FIT_DECLSPEC void signal_handler( int sig, SIG_PF );
    static __FIT_DECLSPEC void signal_exit( int sig ); // signal handler

    // sleep at least up to time t
    static void sleep( timespec *t, timespec *e = 0 )
      { xmt::sleep( t, e ); }
    // delay execution at least on time interval t
    static void delay( timespec *t, timespec *e = 0 )
      { xmt::delay( t, e ); }
    // get precise time
    static void gettime( timespec *t )
      { xmt::gettime( t ); }

#ifndef _WIN32
    static __FIT_DECLSPEC void fork() throw( fork_in_parent, std::runtime_error );
    static __FIT_DECLSPEC void become_daemon() throw( fork_in_parent, std::runtime_error );
#endif

    bool good() const
      { return (_state == goodbit) && (_id != bad_thread_id); }
    bool bad() const
      { return (_state != goodbit) || (_id == bad_thread_id); }
    bool is_join_req() // if true, you can (and should) use join()
      { return (_id != bad_thread_id) && ((_flags & (daemon | detached)) == 0); }

    __FIT_DECLSPEC bool is_self();

    static __FIT_DECLSPEC int xalloc();
    long&  iword( int __idx )
      {
        // _STLP_ASSERT( is_self() );
        return *static_cast<long *>(_alloc_uw( __idx ));
      }
    void*& pword( int __idx )
      {
        // _STLP_ASSERT( is_self() );
        return *reinterpret_cast<void **>(_alloc_uw( __idx ));
      }

    static thread_key_type mtkey()
      { return _mt_key; }

    static const thread_id_type bad_thread_id;

  protected:
    static __FIT_DECLSPEC void _exit( int code = 0 );

  private:
    Thread( const Thread& )
      { }

    void _create( const void *p, size_t psz ) throw( std::runtime_error);
    static void *_call( void *p );

    static void unexpected();
    static void terminate();

    // assume that sizeof( long ) >= sizeof( void * );
    // otherwise, #ifdef workaround should be here.
    // At present, I don't know such OS.
#if 1
    typedef long _uw_alloc_type;
#endif
    typedef std::allocator<_uw_alloc_type> alloc_type;
    __FIT_DECLSPEC void _dealloc_uw();
    __FIT_DECLSPEC _uw_alloc_type *_alloc_uw( int __idx );

    static alloc_type alloc;
    static int _idx; // user words index
    static int _self_idx; // user words index, that word point to self
    static Mutex _idx_lock;
    static Mutex _start_lock;
    static thread_key_type& _mt_key;
    size_t uw_alloc_size;

    thread_id_type _id;
    int _state; // state flags
#ifdef _PTHREADS
# ifndef __hpux
    // sorry, POSIX threads don't have suspend/resume calls, so it should
    // be simulated via cond_wait
    __Condition<false> _suspend;
# endif
#endif
#ifdef __FIT_WIN32THREADS
    unsigned long _thr_id;
#endif
#ifdef __FIT_NOVELL_THREADS
    __Condition<false> _thr_join;
#endif
    entrance_type _entrance;
    void *_param;
    size_t _param_sz;
    unsigned _flags;
    size_t _stack_sz; // stack size, if not 0
    //  Mutex _params_lock; --- no needs
    friend class Init;
    // extern "C", wrap for thread_create
#ifdef __unix
    friend void *_xcall( void * );
#endif
#ifdef __FIT_WIN32THREADS
    friend unsigned long __stdcall _xcall( void *p );
#endif
#ifdef __FIT_NOVELL_THREADS
    friend void _xcall( void * );
#endif
};

template <bool SCOPE>
int __Condition<SCOPE>::try_wait_time( const timespec *abstime )
{
#if defined(__FIT_WIN32THREADS) || defined(__FIT_NOVELL_THREADS)
  MT_LOCK( _lock );
#endif
#if defined(__FIT_UITHREADS) || defined(_PTHREADS)
  MT_REENTRANT( _lock, _x1 );
#endif
  if ( _val == false ) {
#ifdef __FIT_WIN32THREADS
    ResetEvent( _cond );
    time_t ct = time( 0 );
    unsigned ms = abstime->tv_sec >= ct ? (abstime->tv_sec - ct) * 1000 + abstime->tv_nsec / 1000000 : 1;
    MT_UNLOCK( _lock );
    int ret = WaitForSingleObject( _cond, ms );
    if ( ret == WAIT_FAILED ) {
      return -1;
    }
    if ( ret == WAIT_TIMEOUT ) {
      SetEvent( _cond );
      return ETIME;
    }
    return 0;
#endif
#if defined(__FIT_UITHREADS) || defined(_PTHREADS)
    int ret = 0;
    timespec _abstime = *abstime;
    while ( !_val ) {
# ifdef _PTHREADS
      ret = pthread_cond_timedwait( &_cond, &_lock._M_lock, &_abstime );
      if ( ret == ETIMEDOUT ) {
        break;
      }
# endif
# ifdef __FIT_UITHREADS
      ret = cond_timedwait( &_cond, /* &_lock.mutex */ &_lock._M_lock, &_abstime );
      if ( ret == ETIME ) {
        ret = ETIMEDOUT;
      } else if ( ret == ETIMEDOUT ) {
        break;
      }
# endif
    }

    return ret;
#endif // _PTHREADS || __FIT_UITHREADS
#ifdef __FIT_NOVELL_THREADS
    time_t ct = time( 0 );
    unsigned ms = abstime->tv_sec >= ct ? (abstime->tv_sec - ct) * 1000 + abstime->tv_nsec / 1000000 : 1;
    MT_UNLOCK( _lock );
    return TimedWaitOnLocalSemaphore( _cond, ms );
#endif
#ifdef _NOTHREADS
    return 0;
#endif
  }
#if defined(__FIT_WIN32THREADS) || defined(__FIT_NOVELL_THREADS)
  MT_UNLOCK( _lock );
#endif
  return 0;
}

template <bool SCOPE>
int __Condition<SCOPE>::try_wait_delay( const timespec *interval )
{
#if defined(__FIT_WIN32THREADS) || defined(__FIT_NOVELL_THREADS)
  MT_LOCK( _lock );
#endif
#if defined(__FIT_UITHREADS) || defined(_PTHREADS)
  MT_REENTRANT( _lock, _x1 );
#endif
  if ( _val == false ) {
#ifdef WIN32
    _val = false;
    ResetEvent( _cond );
    MT_UNLOCK( _lock );
    unsigned ms = interval->tv_sec * 1000 + interval->tv_nsec / 1000000;
    int ret = WaitForSingleObject( _cond, ms );
    if ( ret == WAIT_FAILED ) {
      return -1;
    }
    if ( ret == WAIT_TIMEOUT ) {
      SetEvent( _cond );
      return ETIME;
    }
    return 0;
#endif
#if defined(__FIT_UITHREADS) || defined(_PTHREADS)
    timespec ct;
    xmt::gettime( &ct );
    ct += *interval;

    int ret = 0;
    timespec _abstime = ct;
    while ( !_val ) {
# ifdef _PTHREADS
      ret = pthread_cond_timedwait( &_cond, &_lock._M_lock, &_abstime );
      if ( ret == ETIMEDOUT ) {
        break;
      }
# endif
# ifdef __FIT_UITHREADS
      ret = cond_timedwait( &_cond, /* &_lock.mutex */ &_lock._M_lock, &_abstime );
      if ( ret == ETIME ) {
        ret = ETIMEDOUT;
      } else if ( ret == ETIMEDOUT ) {
        break;
      }
# endif
    }

    return ret;
#endif // _PTHREADS || __FIT_UITHREADS
#ifdef __FIT_NOVELL_THREADS
    MT_UNLOCK( _lock );
    return TimedWaitOnLocalSemaphore( _cond, interval->tv_sec * 1000 + interval->tv_nsec / 1000000 );
#endif

#ifdef _NOTHREADS
    return 0;
#endif
  }

#if defined(__FIT_WIN32THREADS) || defined(__FIT_NOVELL_THREADS)
  MT_UNLOCK( _lock );
#endif
  return 0;
}

template <bool SCOPE>
int __Condition<SCOPE>::wait_time( const timespec *abstime )
{
#ifdef __FIT_WIN32THREADS
  MT_LOCK( _lock );
  _val = false;
  ResetEvent( _cond );
  time_t ct = time( 0 );
  unsigned ms = abstime->tv_sec >= ct ? (abstime->tv_sec - ct) * 1000 + abstime->tv_nsec / 1000000 : 1;
  MT_UNLOCK( _lock );
  int ret = WaitForSingleObject( _cond, ms );
  if ( ret == WAIT_FAILED ) {
    return -1;
  }
  if ( ret == WAIT_TIMEOUT ) {
    SetEvent( _cond );
    return ETIME;
  }
  return 0;
#endif
#ifdef _PTHREADS
  MT_REENTRANT( _lock, _x1 ); // ??
  _val = false;
  timespec _abstime = *abstime;
  int ret = pthread_cond_timedwait( &_cond, &_lock._M_lock, &_abstime );
  if ( ret == ETIMEDOUT ) {
    _val = true;
  }
  return ret;
#endif // _PTHREADS
#ifdef __FIT_UITHREADS
  MT_REENTRANT( _lock, _x1 );
  _val = false;
  int ret;
  timespec _abstime = *abstime;
  while ( !_val ) {
    ret = cond_timedwait( &_cond, /* &_lock.mutex */ &_lock._M_lock, &_abstime );
    if ( ret == ETIME ) {
      _val = true;
      ret = ETIMEDOUT;
    } else if ( ret == ETIMEDOUT ) {
      _val = true;
    }
  }

  return ret;
#endif
#ifdef __FIT_NOVELL_THREADS
  MT_LOCK( _lock );
  _val = false;
  time_t ct = time( 0 );
  unsigned ms = abstime->tv_sec >= ct ? (abstime->tv_sec - ct) * 1000 + abstime->tv_nsec / 1000000 : 1;
  MT_UNLOCK( _lock );
  return TimedWaitOnLocalSemaphore( _cond, ms );
#endif
#ifdef _NOTHREADS
  return 0;
#endif
}

template <bool SCOPE>
int __Condition<SCOPE>::wait_delay( const timespec *interval )
{
#ifdef __FIT_WIN32THREADS
  MT_LOCK( _lock );
  _val = false;
  ResetEvent( _cond );
  unsigned ms = interval->tv_sec * 1000 + interval->tv_nsec / 1000000;
  MT_UNLOCK( _lock );
  int ret = WaitForSingleObject( _cond, ms );
  if ( ret == WAIT_FAILED ) {
    return -1;
  }
  if ( ret == WAIT_TIMEOUT ) {
    SetEvent( _cond );
    return ETIME;
  }
  return 0;
#endif
#if defined(__FIT_UITHREADS) || defined(_PTHREADS)
  timespec ct;
  xmt::gettime( &ct );
  ct += *interval;

  return this->wait_time( &ct );
#endif
#ifdef __FIT_NOVELL_THREADS
  MT_LOCK( _lock );
  _val = false;
  unsigned ms = interval->tv_sec * 1000 + interval->tv_nsec / 1000000;
  MT_UNLOCK( _lock );
  return TimedWaitOnLocalSemaphore( _cond, ms );
#endif
#ifdef _NOTHREADS
  return 0;
#endif
}

} // namespace xmt

namespace __impl = xmt; // compatibility

#endif // __XMT_H