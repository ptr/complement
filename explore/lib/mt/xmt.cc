// -*- C++ -*- Time-stamp: <99/04/07 11:10:25 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <xmt.h>

#include <cstring>
#include <ostream>

#ifdef WIN32
#include <iostream>
#include <iomanip>
#include <win_config.h>

using namespace std;
#endif

extern "C" {
#ifdef WIN32
  typedef unsigned long (__stdcall *entrance_type_C)( void * );
#else
  typedef void *(*entrance_type_C)( void * );
#endif
}

namespace __impl {

using std::cerr;
using std::endl;

#ifdef WIN32
__declspec( dllexport ) int __thr_key = TlsAlloc();
#endif

__DLLEXPORT
Thread::Thread( unsigned __f ) :
#ifdef WIN32
    _id( INVALID_HANDLE_VALUE ),
#else
    _id( -1 ),
#endif
    _entrance( 0 ),
    _param( 0 ),
    _param_sz( 0 ),
    _flags( __f )
{ }

__DLLEXPORT
Thread::Thread( Thread::entrance_type entrance, const void *p, size_t psz, unsigned __f ) :
    _entrance( entrance ),
    _param( 0 ),
    _param_sz( 0 ),
    _flags( __f )
{ _create( p, psz ); }

__DLLEXPORT
Thread::~Thread()
{
#ifdef WIN32
  __stl_assert( _id == INVALID_HANDLE_VALUE );
#else
  __stl_assert( _id == -1 );
#endif
}

__DLLEXPORT
void Thread::launch( entrance_type entrance, const void *p, size_t psz )
{
#ifdef WIN32
  if ( _id == INVALID_HANDLE_VALUE ) {
#else
  if ( _id == -1 ) {
#endif
    _entrance = entrance;
    _create( p, psz );
  }
}

__DLLEXPORT
int Thread::join()
{
#ifdef WIN32
  unsigned long ret_code = 0;
  if ( _id != INVALID_HANDLE_VALUE ) {
    WaitForSingleObject( _id, -1 );
    GetExitCodeThread( _id, &ret_code );
    _id = INVALID_HANDLE_VALUE;
  }
#else // !WIN32
  int ret_code = 0;
  if ( _id != -1 && (_flags & (daemon | detached) ) == 0 ) {
#  ifdef _PTHREADS
    pthread_join( _id, (void **)(&ret_code) );
#  endif
#  ifdef _SOLARIS_THREADS
    thr_join( _id, 0, (void **)(&ret_code) );
#  endif
    _id = -1;
  }
#endif // !WIN32

  return ret_code;
}

__DLLEXPORT
int Thread::suspend()
{
#ifdef WIN32
  if ( _id != INVALID_HANDLE_VALUE ) {
    return SuspendThread( _id );
  }
#else
  if ( _id != -1 ) {
#  ifdef _PTHREADS
    // sorry, POSIX threads don't have suspend/resume calls, so it should
    // be simulated via condwait
    if ( _id != pthread_self() ) {
      throw domain_error( "Thread::suspend() for POSIX threads work only while call from the same thread." );
      // May be signalling pthread_kill( _id, SIG??? ) will be good workaround?
    }
    _suspend.wait();
#  endif
#  ifdef _SOLARIS_THREADS
    return thr_suspend( _id );
#  endif
  }
#endif

  return -1;
}

__DLLEXPORT
int Thread::resume()
{
#ifdef WIN32
  if ( _id != INVALID_HANDLE_VALUE ) {
    return ResumeThread( _id );
  }
#else
  if ( _id != -1 ) {
#  ifdef _PTHREADS
    // sorry, POSIX threads don't have suspend/resume calls, so it should
    // be simulated via condwait
    _suspend.set( true ); // less syscall than _suspend.signal();
#  endif
#  ifdef _SOLARIS_THREADS
    return thr_continue( _id );
#  endif
  }
#endif

  return -1;
}

__DLLEXPORT
int Thread::kill( int sig )
{
#ifdef __unix
  if ( _id != -1 ) {
#ifdef _SOLARIS_THREADS
    return thr_kill( _id, sig );
#endif
#ifdef _PTHREADS
    return pthread_kill( _id, sig );
#endif
  }
#endif
#ifdef WIN32
  // The behavior of TerminateThread significant differ from SOLARIS and POSIX
  // threads, and I don't find analogs to workaround...
  if ( _id != INVALID_HANDLE_VALUE ) {
    return TerminateThread( _id, 0 ) ? 0 : -1;
  }
#endif
  return -1;
}

__DLLEXPORT
void Thread::exit( int code )
{
#ifdef _PTHREADS
  pthread_exit( (void *)code );
#endif
#ifdef _SOLARIS_THREADS
  thr_exit( (void *)code );
#endif
#ifdef WIN32
  ExitThread( code );
#endif
}

void Thread::_create( const void *p, size_t psz ) throw(runtime_error)
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

  int err;
#ifdef _PTHREADS
  err = pthread_create( &_id, 0, entrance_type_C(_call), this );
#endif
#ifdef _SOLARIS_THREADS
  err = thr_create( 0, 0, entrance_type_C(_call), this, _flags, &_id );
#endif
#ifdef WIN32
  _id = CreateThread( 0, 0, entrance_type_C(_call), this, _flags, &_thr_id );
  err = GetLastError();
#endif
  if ( err != 0 ) {
    if ( psz > sizeof( void *) ) { // clear allocated here
      delete [] _param;
    }
    throw runtime_error( "Thread creation error" );
  }
}

void *Thread::_call( void *p )
{
  Thread *me = static_cast<Thread *>(p);

  // After exit of me->_entrance, there is may be no more *me itself,
  // so it's members may be unaccessable. Don't use me->"*" after call
  // of me->_entrance!!!
  void *_param     = me->_param;
  size_t _param_sz = me->_param_sz;
  int ret;

#ifdef _PTHREADS
  if ( me->_flags & (daemon | detached) ) {
    pthread_detach( me->_id );
  }
#endif

#ifdef WIN32
  set_unexpected( unexpected );
  set_terminate( terminate );
#else
  std::set_unexpected( unexpected );
  std::set_terminate( terminate );
#endif
	
  try {
    ret = me->_entrance( me->_param );
    // I should be make me->_id = -1; here...
    // Next line is in conflict what I say in this function begin.
    // So don't delete Thread before it termination!
#ifdef WIN32
    me->_id = INVALID_HANDLE_VALUE;
#else
    me->_id = -1;
#endif
    // 
  }
  catch ( std::exception& e ) {
    cerr << e.what() << endl;
    ret = -1;
  }
  catch ( ... ) {
    cerr << "Oh, oh, say baby Sally. Dick and Jane launch." << endl;
    ret = -1;
  }

  try {
    if ( _param_sz > sizeof(void *) ) { // that's allocated
      delete [] _param;
      _param_sz = 0;
      _param = 0;
    }
  }
  catch ( ... ) {
    cerr << "(+)" << endl;
    ret = -1;
  }

#if defined( __SUNPRO_CC ) && defined( __i386 )
  Thread::exit( ret );
#endif
  return (void *)ret;
}

void Thread::unexpected()
{
  cerr << "\nUnexpected exception" << endl;
  Thread::exit( -1 );
}

void Thread::terminate()
{
  cerr << "\nTerminated" << endl;
  Thread::exit( -2 );
}

} // namespace __impl
