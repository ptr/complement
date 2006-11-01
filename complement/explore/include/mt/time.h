// -*- C++ -*- Time-stamp: <06/10/24 09:28:28 ptr>

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

timespec operator +( const timespec& a, const timespec& b );
timespec operator -( const timespec& a, const timespec& b );
timespec operator /( const timespec& a, unsigned b );
timespec operator /( const timespec& a, unsigned long b );

// timespec& operator =( timespec& a, const timespec& b );
timespec& operator +=( timespec& a, const timespec& b );
timespec& operator -=( timespec& a, const timespec& b );
timespec& operator /=( timespec& a, unsigned b );
timespec& operator /=( timespec& a, unsigned long b );

bool operator >( const timespec& a, const timespec& b );
bool operator >=( const timespec& a, const timespec& b );
bool operator <( const timespec& a, const timespec& b );
bool operator <=( const timespec& a, const timespec& b );
bool operator ==( const timespec& a, const timespec& b );
bool operator !=( const timespec& a, const timespec& b );

namespace xmt {

// delay execution at least on time interval t
__FIT_DECLSPEC void delay( timespec *interval, timespec *remain );
// sleep at least up to time t
__FIT_DECLSPEC void sleep( timespec *abstime, timespec *real_time );
// get precise time
__FIT_DECLSPEC void gettime( timespec *t );

} // namespace xmt

#endif // __mt_time_h
