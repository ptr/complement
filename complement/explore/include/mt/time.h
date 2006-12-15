// -*- C++ -*- Time-stamp: <06/12/15 10:21:37 ptr>

/*
 * Copyright (c) 2002, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __mt_time_h
#define __mt_time_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifdef WIN32
# include <windows.h>
# include <memory>
# include <ctime>
# include <limits>
# pragma warning( disable : 4290)
#endif // WIN32

#include <string>
#include <ctime>

#ifdef N_PLAT_NLM
# include <sys/time.h> // timespec, timespec_t
#endif

#ifdef _WIN32
extern "C" {

typedef struct  timespec {              /* definition per POSIX.4 */
        time_t          tv_sec;         /* seconds */
        long            tv_nsec;        /* and nanoseconds */
} timespec_t;

} // extern "C"
#endif // _WIN32

#if defined(_WIN32) || defined(N_PLAT_NLM)
extern "C" {

typedef struct timespec timestruc_t;    /* definition per SVr4 */

} // extern "C"
#endif

std::string calendar_time( time_t t );

::timespec operator +( const ::timespec& a, const ::timespec& b );
::timespec operator -( const ::timespec& a, const ::timespec& b );
::timespec operator /( const ::timespec& a, unsigned b );
::timespec operator /( const ::timespec& a, unsigned long b );
::timespec operator *( const ::timespec& a, unsigned b );
::timespec operator *( const ::timespec& a, unsigned long b );
inline ::timespec operator *( unsigned b, const ::timespec& a )
{ return a * b; }
inline ::timespec operator *( unsigned long b, const ::timespec& a )
{ return a * b; }

// timespec& operator =( timespec& a, const timespec& b );
::timespec& operator +=( ::timespec& a, const ::timespec& b );
::timespec& operator -=( ::timespec& a, const ::timespec& b );
::timespec& operator /=( ::timespec& a, unsigned b );
::timespec& operator /=( ::timespec& a, unsigned long b );
::timespec& operator *=( ::timespec& a, unsigned b );
::timespec& operator *=( ::timespec& a, unsigned long b );

bool operator >( const ::timespec& a, const ::timespec& b );
bool operator >=( const ::timespec& a, const ::timespec& b );
bool operator <( const ::timespec& a, const ::timespec& b );
bool operator <=( const ::timespec& a, const ::timespec& b );
bool operator ==( const ::timespec& a, const ::timespec& b );
bool operator !=( const ::timespec& a, const ::timespec& b );

namespace xmt {

struct timespec :
        public ::timespec
{
    timespec()
      { tv_sec = 0; tv_nsec = 0; }

    timespec( time_t s, long ns )
      { tv_sec = s; tv_nsec = ns; }

    timespec( time_t s )
      { tv_sec = s; tv_nsec = 0; }

    timespec( const timespec& t )
      { tv_sec = t.tv_sec; tv_nsec = t.tv_nsec; }

    timespec( const ::timespec& t )
      { tv_sec = t.tv_sec; tv_nsec = t.tv_nsec; }

    timespec& operator =( const timespec& t )
      { tv_sec = t.tv_sec; tv_nsec = t.tv_nsec; return *this; }

    timespec& operator =( const ::timespec& t )
      { tv_sec = t.tv_sec; tv_nsec = t.tv_nsec; return *this; }

    timespec& operator =( time_t t )
      { tv_sec = t; tv_nsec = 0; return *this; }

    operator ::timespec() const
      { return *this; }

    operator const ::timespec&() const
      { return *this; }

    operator ::timespec&()
      { return *this; }

    operator time_t() const
      { return tv_sec; }

    operator double() const
      { return tv_nsec == 0 ? static_cast<double>(tv_sec) : tv_sec + 1.0e-9 * tv_nsec; }
};

// delay execution at least on time interval t
__FIT_DECLSPEC void delay( const ::timespec& interval, ::timespec& remain );
__FIT_DECLSPEC void delay( const ::timespec& interval );
inline void delay( const ::timespec *interval, ::timespec *remain )
{ delay( *interval, *remain ); }
inline void delay( const ::timespec *interval )
{ delay( *interval ); }
// sleep at least up to time t
__FIT_DECLSPEC void sleep( const ::timespec& abstime, ::timespec& real_time );
__FIT_DECLSPEC void sleep( const ::timespec& abstime );
inline void sleep( const ::timespec *abstime, ::timespec *real_time )
{ sleep( *abstime, *real_time ); }
inline void sleep( const ::timespec *abstime )
{ sleep( *abstime ); }
// get precise time
__FIT_DECLSPEC void gettime( ::timespec& t );
inline void gettime( ::timespec *t )
{ gettime( *t ); }

} // namespace xmt

#endif // __mt_time_h
