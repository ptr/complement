/* -*- C -*- Time-stamp: <03/05/07 19:48:03 ptr> */

/*
 *
 * Copyright (c) 2002
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 */

// #define __USE_POSIX199309
#define __need_timespec

#ifdef __linux
#  ifndef __USE_GNU
#    define __USE_GNU
#    define _GNU_SOURCE  /* __USE_GNU may be undef in the features.h */
#  endif
#  include <sys/time.h>
#endif

#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

/*
 * Take arguments as command line, run it and print resource usage time
 * in the format:
 *   <user time> <system time> <time elapsed>
 *
 * time [ [-a] -o file] command ...
 *   -o  write result in file
 *   -a  add result to file, raise then overwrite
 *
 */

#ifdef __CYGWIN32__ // but defined in Linux headers
/* Macros for converting between `struct timeval' and `struct timespec'.  */
# define TIMEVAL_TO_TIMESPEC(tv, ts) {                                   \
        (ts)->tv_sec = (tv)->tv_sec;                                    \
        (ts)->tv_nsec = (tv)->tv_usec * 1000;                           \
}
# define TIMESPEC_TO_TIMEVAL(tv, ts) {                                   \
        (tv)->tv_sec = (ts)->tv_sec;                                    \
        (tv)->tv_usec = (ts)->tv_nsec / 1000;                           \
}
#endif

int main( int argc, char * const * argv )
{
  int app_mode = 0;
  char *fname = 0;

  if ( argc > 1 ) {
    struct timespec ts;

    while ( argc > 1 ) {
      if ( strcmp( argv[1], "-a" ) == 0 ) {
        app_mode = 1;
        --argc;
        ++argv;
      } else if ( strcmp( argv[1], "-o" ) == 0 ) {
        --argc;
        ++argv;
        fname = argv[1];
        --argc;
        ++argv;
      } else {
        break;
      }
    }
    if ( argc <= 1 ) {
      return 0;
    }

    /* read precise time */
#if defined(__linux) || defined(__CYGWIN32__)
    struct timeval tv;
    gettimeofday( &tv, 0 );
    TIMEVAL_TO_TIMESPEC( &tv, &ts );
#elif defined( WIN32 )
    time_t ct = time( 0 );
    ts->tv_sec = ct / 1000;
    ts->tv_nsec = (ct % 1000) * 1000000;
#elif defined(__sun) || defined(__hpux)
    clock_gettime( CLOCK_REALTIME, &ts );
#else
#error "You should implement OS-dependent precise clock"
#endif

    pid_t ch_pid = fork();
    if ( ch_pid == 0 ) {
      execvp( argv[1], argv + 1 ); /* start command with ones args */
    } else {
      struct rusage r;
      struct timespec te;
#if defined(__linux) || defined(__CYGWIN32__)
      struct timeval tve;
#elif defined( WIN32 )
      time_t cte;
#endif

      wait3( 0, 0, &r ); /* read process statistic */
      /* take precise time */
#if defined(__linux) || defined(__CYGWIN32__)
      gettimeofday( &tve, 0 );
      TIMEVAL_TO_TIMESPEC( &tve, &te );
#elif defined( WIN32 )
      ct = time( 0 );
      te->tv_sec = ct / 1000;
      te->tv_nsec = (ct % 1000) * 1000000;
#elif defined(__sun) || defined(__hpux)
      clock_gettime( CLOCK_REALTIME, &te );
#else
#error "You should implement OS-dependent precise clock"
#endif

      /* difference between stop and start times */
      te.tv_sec = te.tv_sec > ts.tv_sec ? te.tv_sec - ts.tv_sec : 0; // out_of_range?
      if ( te.tv_nsec > ts.tv_nsec ) {
        te.tv_nsec -= ts.tv_nsec;
      } else if ( te.tv_sec > 0 ) {
        --te.tv_sec;
        te.tv_nsec = 1000000000 - ts.tv_nsec + te.tv_nsec;
      } else {
        te.tv_nsec = 0; // out_of_range?
      }
      if ( fname != 0 ) {
        FILE *f;
        if ( app_mode ) {
          f = fopen( fname, "a" );
        } else {
          f = fopen( fname, "w" );
        }
        fprintf( f, "%d.%.6d %d.%.6d %d.%.6d\n",
                 r.ru_utime.tv_sec, r.ru_utime.tv_usec,
                 r.ru_stime.tv_sec, r.ru_stime.tv_usec,
                 te.tv_sec, te.tv_nsec / 1000 );
        fclose( f );        
      } else {
        printf( "%d.%.6d %d.%.6d %d.%.6d\n",
                r.ru_utime.tv_sec, r.ru_utime.tv_usec,
                r.ru_stime.tv_sec, r.ru_stime.tv_usec,
                te.tv_sec, te.tv_nsec / 1000 );
      }
    }
  }

  return 0;
}
