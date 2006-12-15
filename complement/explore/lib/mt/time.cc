// -*- C++ -*- Time-stamp: <06/12/15 10:35:48 ptr>

/*
 * Copyright (c) 2002, 2003, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <mt/time.h>
#include <mt/xmt.h>
#include <cmath>
#include <sys/time.h>

#ifdef _WIN32
xmt::Mutex _l;
#endif

std::string calendar_time( time_t t )
{
#ifndef _WIN32
  char buff[32];
// #ifdef __sun
#if (_POSIX_C_SOURCE - 0 >= 199506L) || defined(_POSIX_PTHREAD_SEMANTICS) || defined(N_PLAT_NLM) || \
    defined(__FreeBSD__) || defined(__OpenBSD__)
  ctime_r( &t, buff );
#else
  ctime_r( &t, buff, 32 );
#endif
// #endif
// #ifdef __linux
//  ctime_r( &t, buff );
// #endif
// #if !defined(__sun) && !defined(__linux)
// #  error( "Fix me!" )
// #endif

  // buff[24] = 0; // null terminate
  std::string s;

  s.assign( buff, 24 );

  return s;
#else // _WIN32
  MT_REENTRANT( _l, _1 );
  return std::string( ctime( &t ) );
#endif
}

timespec operator +( const timespec& a, const timespec& b )
{
  timespec c;

  c.tv_sec = a.tv_sec;
  c.tv_nsec = a.tv_nsec;
  c.tv_sec += b.tv_sec;
  c.tv_nsec += b.tv_nsec;
  c.tv_sec += c.tv_nsec / 1000000000;
  c.tv_nsec %= 1000000000;

  return c;
}

timespec operator -( const timespec& a, const timespec& b )
{
  timespec c;

  c.tv_sec = a.tv_sec > b.tv_sec ? a.tv_sec - b.tv_sec : 0; // out_of_range?
  if ( a.tv_nsec >= b.tv_nsec ) {
    c.tv_nsec = a.tv_nsec - b.tv_nsec;
  } else if ( c.tv_sec > 0 ) {
    --c.tv_sec;
    c.tv_nsec = 1000000000 - b.tv_nsec + a.tv_nsec;
  } else {
    c.tv_nsec = 0; // out_of_range?
  }

  return c;
}

timespec operator /( const timespec& a, unsigned b )
{
  timespec c;
  double d = a.tv_sec + 1.0e-9 * a.tv_nsec;
  d /= b;

  c.tv_nsec = static_cast<time_t>(1.0e9 * modf( d, &d ) + 0.5);
  c.tv_sec = static_cast<long>(d);

  return c;
}

timespec operator /( const timespec& a, unsigned long b )
{
  timespec c;
  double d = a.tv_sec + 1.0e-9 * a.tv_nsec;
  d /= b;

  c.tv_nsec = static_cast<time_t>(1.0e9 * modf( d, &d ) + 0.5);
  c.tv_sec = static_cast<long>(d);

  return c;
}

timespec operator *( const timespec& a, unsigned b )
{
  timespec c;
#if 0
  double d = a.tv_sec + 1.0e-9 * a.tv_nsec;
  d *= b;

  c.tv_nsec = static_cast<time_t>(1.0e9 * modf( d, &d ) + 0.5);
  c.tv_sec = static_cast<long>(d);
#else
  c.tv_sec = a.tv_sec * b;
  unsigned long long _tmp = c.tv_nsec;
  _tmp *= b;
  c.tv_sec += static_cast<time_t>(_tmp / 1000000000UL);
  c.tv_nsec = static_cast<long>(_tmp % 1000000000UL);
#endif
  return c;
}

timespec operator *( const timespec& a, unsigned long b )
{
  timespec c;
#if 0
  double d = a.tv_sec + 1.0e-9 * a.tv_nsec;
  d *= b;

  c.tv_nsec = static_cast<time_t>(1.0e9 * modf( d, &d ) + 0.5);
  c.tv_sec = static_cast<long>(d);
#else
  c.tv_sec = a.tv_sec * b;
  unsigned long long _tmp = c.tv_nsec;
  _tmp *= b;
  c.tv_sec += static_cast<time_t>(_tmp / 1000000000UL);
  c.tv_nsec = static_cast<long>(_tmp % 1000000000UL);
#endif
  return c;
}

timespec& operator +=( timespec& a, const timespec& b )
{
  a.tv_sec += b.tv_sec;
  a.tv_nsec += b.tv_nsec;
  a.tv_sec += a.tv_nsec / 1000000000;
  a.tv_nsec %= 1000000000;

  return a;
}

timespec& operator -=( timespec& a, const timespec& b )
{
  a.tv_sec = a.tv_sec > b.tv_sec ? a.tv_sec - b.tv_sec : 0; // out_of_range?
  if ( a.tv_nsec >= b.tv_nsec ) {
    a.tv_nsec -= b.tv_nsec;
  } else if ( a.tv_sec > 0 ) {
    --a.tv_sec;
    a.tv_nsec += 1000000000 - b.tv_nsec;
  } else {
    a.tv_nsec = 0; // out_of_range?
  }

  return a;
}

timespec& operator /=( timespec& a, unsigned b )
{
  double d = a.tv_sec + 1.0e-9 * a.tv_nsec;
  d /= b;

  a.tv_nsec = static_cast<time_t>(1.0e9 * modf( d, &d ) + 0.5);
  a.tv_sec = static_cast<long>(d);

  return a;
}

timespec& operator /=( timespec& a, unsigned long b )
{
  double d = a.tv_sec + 1.0e-9 * a.tv_nsec;
  d /= b;

  a.tv_nsec = static_cast<time_t>(1.0e9 * modf( d, &d ) + 0.5);
  a.tv_sec = static_cast<long>(d);

  return a;
}

timespec& operator *=( timespec& a, unsigned b )
{
  a.tv_sec *= b;
  unsigned long long _tmp = a.tv_nsec;
  _tmp *= b;
  a.tv_sec += static_cast<time_t>(_tmp / 1000000000UL);
  a.tv_nsec = static_cast<long>(_tmp % 1000000000UL);

  return a;
}

timespec& operator *=( timespec& a, unsigned long b )
{
  a.tv_sec *= b;
  unsigned long long _tmp = a.tv_nsec;
  _tmp *= b;
  a.tv_sec += static_cast<time_t>(_tmp / 1000000000UL);
  a.tv_nsec = static_cast<long>(_tmp % 1000000000UL);

  return a;
}

bool operator ==( const timespec& a, const timespec& b )
{
  return (a.tv_sec == b.tv_sec) && (a.tv_nsec == b.tv_nsec);
}

bool operator !=( const timespec& a, const timespec& b )
{
  return (a.tv_sec != b.tv_sec) || (a.tv_nsec != b.tv_nsec);
}

bool operator >( const timespec& a, const timespec& b )
{
  if ( a.tv_sec > b.tv_sec ) {
    return true;
  } else if ( b.tv_sec > a.tv_sec ) {
    return false;
  }
  
  return a.tv_nsec > b.tv_nsec;
}

bool operator >=( const timespec& a, const timespec& b )
{
  if ( a.tv_sec > b.tv_sec ) {
    return true;
  } else if ( b.tv_sec > a.tv_sec ) {
    return false;
  }
  
  return a.tv_nsec >= b.tv_nsec;
}

bool operator <( const timespec& a, const timespec& b )
{
  if ( a.tv_sec < b.tv_sec ) {
    return true;
  } else if ( b.tv_sec < a.tv_sec ) {
    return false;
  }
  
  return a.tv_nsec < b.tv_nsec;
}

bool operator <=( const timespec& a, const timespec& b )
{
  if ( a.tv_sec < b.tv_sec ) {
    return true;
  } else if ( b.tv_sec < a.tv_sec ) {
    return false;
  }
  
  return a.tv_nsec <= b.tv_nsec;
}

namespace xmt {

__FIT_DECLSPEC
void delay( const ::timespec& interval, ::timespec& remain )
{
#ifdef __unix
  nanosleep( &interval, &remain );
#endif
#ifdef WIN32
  Sleep( interval.tv_sec * 1000 + interval.tv_nsec / 1000000 );
  remain.tv_sec = 0;
  remain.tv_nsec = 0;
#endif
#ifdef __FIT_NETWARE
  ::delay( interval.tv_sec * 1000 + interval.tv_nsec / 1000000 );
  remain.tv_sec = 0;
  remain.tv_nsec = 0;
#endif
}

__FIT_DECLSPEC
void delay( const ::timespec& interval )
{
#ifdef __unix
  nanosleep( &interval, 0 );
#endif
#ifdef WIN32
  Sleep( interval.tv_sec * 1000 + interval.tv_nsec / 1000000 );
#endif
#ifdef __FIT_NETWARE
  ::delay( interval.tv_sec * 1000 + interval.tv_nsec / 1000000 );
#endif
}

__FIT_DECLSPEC
void sleep( const ::timespec& abstime, ::timespec& real_time )
{
#ifdef __unix
  ::timespec ct;
  gettime( ct );

  if ( abstime > ct ) {
    xmt::timespec st( abstime );
    st -= ct;
    nanosleep( static_cast<const ::timespec *>(&st), &real_time );
    real_time += ct;
  } else {
    real_time.tv_sec = ct.tv_sec;
    real_time.tv_nsec = ct.tv_nsec;
  }
#endif
#ifdef WIN32
  time_t ct = time( 0 );
  time_t _conv = abstime.tv_sec * 1000 + abstime.tv_nsec / 1000000;

  Sleep( _conv >= ct ? _conv - ct : 1 );
  real_time.tv_sec = abstime.tv_sec;
  real_time.tv_nsec = abstime.tv_nsec;
#endif
#ifdef __FIT_NETWARE
  time_t ct = time( 0 );
  time_t _conv = abstime.tv_sec * 1000 + abstime.tv_nsec / 1000000;

  ::delay( _conv >= ct ? _conv - ct : 1 );
  // Novell not return elapsed time interval
  real_time.tv_sec = abstime.tv_sec;
  real_time.tv_nsec = abstime.tv_nsec;
#endif
}

__FIT_DECLSPEC
void sleep( const ::timespec& abstime )
{
#ifdef __unix
  ::timespec ct;
  gettime( ct );

  if ( abstime > ct ) {
    xmt::timespec st( abstime );
    st -= ct;
    nanosleep( static_cast<const ::timespec *>(&st), 0 );
  }
#endif
#ifdef WIN32
  time_t ct = time( 0 );
  time_t _conv = abstime.tv_sec * 1000 + abstime.tv_nsec / 1000000;

  Sleep( _conv >= ct ? _conv - ct : 1 );
#endif
#ifdef __FIT_NETWARE
  time_t ct = time( 0 );
  time_t _conv = abstime.tv_sec * 1000 + abstime.tv_nsec / 1000000;

  ::delay( _conv >= ct ? _conv - ct : 1 );
#endif
}

__FIT_DECLSPEC
void gettime( ::timespec& t )
{
#if defined(__linux) || defined(__FreeBSD__) || defined(__OpenBSD__)
  timeval tv;
  gettimeofday( &tv, 0 );
  TIMEVAL_TO_TIMESPEC( &tv, &t );
#elif defined( WIN32 )
  union {
    FILETIME ft; // 100 ns intervals since Jan 1 1601 (UTC)
      __int64 t;
  } ft;
  GetSystemTimeAsFileTime( &ft.ft );
  t.tv_sec = int(ft.t / (__int64)10000000 - (__int64)(11644473600)); // 60 * 60 * 24 * 134774, 1970 - 1601
  t.tv_nsec = int(ft.t %  (__int64)(10000000)) * 100;
    
  //time_t ct = time( 0 );
  //t->tv_sec = ct; // ct / 1000;
  //t->tv_nsec = 0; // (ct % 1000) * 1000000;
#elif defined(__sun) || defined(__hpux)
  clock_gettime( CLOCK_REALTIME, &t );
#elif defined(__FIT_NETWARE)
  time_t ct = time(0); // GetHighResolutionTimer (ret current time in 100 microsec increments)
                       // GetSuperHighResolutionTimer() (ret current time in 838 nanosec increments)
  t.tv_sec = ct;
  t.tv_nsec = 0;
#else
#error "You should implement OS-dependent precise clock"
#endif
}

} // namespace xmt
