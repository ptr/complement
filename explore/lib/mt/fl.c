/* -*- C -*- Time-stamp: <04/04/21 12:22:20 ptr> */

#if defined(__unix) || defined(__unix__)
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#if defined(__unix) || defined(__unix__)
# include <fcntl.h>
# include <sys/file.h>
# include <sys/types.h>
# include <unistd.h>
# include <errno.h>
#endif /* __unix || __unix__ */

#ifdef WIN32
# include <io.h>
# include <windows.h>
#endif /* WIN32 */

#include "mt/flck.h"

#if defined(__unix) || defined(__unix__)

/*
 * Note: POSIX fcntl-based file locking don't provide
 * file lock within same process; we prefer pthread_rwlock
 * for this, but if one not available, we will use
 * two mutexes.
 *
 * The BSD-style flock is free from fcntl's disadvantage,
 * but not work properly in MT-applications on FreeBSD
 * (ironic!) and depends upon file system (not work with
 * NFS in most cases, SMBFS, FAT*, may be with NTFS too).
 *
 */

# if !defined(_REENTRANT) && !defined(_THREAD_SAFE) && !defined(_POSIX_THREADS)
/* 
 * In case of non-MT-safe application the fcntl approach
 * has advantage (FS-independent). So we use fcntl call
 * in this case too, only without mutexes; but if you prefer
 * flock(), just uncomment line below.
 */
/* #  define __USE_FLOCK */
# else /* _REENTRANT || _THREAD_SAFE */
#  if defined(__USE_GNU) && !defined(__USE_UNIX98)
#   include <pthread.h>

pthread_mutex_t __mw = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t __mr = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static int __mr_flag = 0;

#   define __USE_TWO_MTX
#  endif

#  if defined(__USE_UNIX98) || defined(__FreeBSD__) || defined(__OpenBSD__)
#   include <pthread.h>

pthread_rwlock_t __ml = PTHREAD_RWLOCK_INITIALIZER;

#   define __USE_RW_MTX
#  endif
# endif /* _REENTRANT || _THREAD_SAFE */
# ifndef __USE_FLOCK

#if !defined(__FreeBSD__) && !defined(__OpenBSD__)
static struct flock _flw = { F_WRLCK, SEEK_SET, 0, 0, 0 };
static struct flock _flr = { F_RDLCK, SEEK_SET, 0, 0, 0 };
static struct flock _flu = { F_UNLCK, SEEK_SET, 0, 0, 0 };
#else
static struct flock _flw = { 0, 0, 0, F_WRLCK, SEEK_SET };
static struct flock _flr = { 0, 0, 0, F_RDLCK, SEEK_SET };
static struct flock _flu = { 0, 0, 0, F_UNLCK, SEEK_SET };
#endif

# endif /* !__USE_FLOCK */
#endif /* __unix || __unix__ */

#ifdef WIN32
static OVERLAPPED _fl = { 0, 0, 0, 0, 0 };
#endif /* WIN32 */

/* #if defined(__unix) || defined(__unix__) */
int flck( int fd, int operation )
/* #endif */ /* __unix || __unix__ */
/*
#ifdef WIN32
int flck( HANDLE fd, int operation )
#endif */ /* WIN32 */
{
#if defined(__unix) || defined(__unix__)
  struct flock fl;
  switch ( operation ) {
    case _F_LCK_W:
# ifdef __USE_FLOCK
      return flock( fd, LOCK_EX );
# else /* __USE_FLOCK */
#  ifdef __USE_TWO_MTX
      pthread_mutex_lock( &__mr );
      ++__mr_flag;
      pthread_mutex_lock( &__mw );
#  endif
#  ifdef __USE_RW_MTX
      pthread_rwlock_wrlock( &__ml );
#  endif
      return fcntl( fd, F_SETLKW, &_flw );
# endif /* __USE_FLOCK */
    case _F_LCK_R:
# ifdef __USE_FLOCK
      return flock( fd, LOCK_SH );
# else /* __USE_FLOCK */
#  ifdef __USE_TWO_MTX
      pthread_mutex_lock( &__mw );
#  endif
#  ifdef __USE_RW_MTX
      pthread_rwlock_rdlock( &__ml );
#  endif
      return fcntl( fd, F_SETLKW, &_flr );
# endif /* __USE_FLOCK */
    case _F_UNLCK:
# ifdef __USE_FLOCK
      return flock( fd, LOCK_UN );
# else /* __USE_FLOCK */
#  ifdef __USE_TWO_MTX
      pthread_mutex_unlock( &__mw );
      if ( __mr_flag ) {
        --__mr_flag;
        pthread_mutex_unlock( &__mr );
      }
#  endif
#  ifdef __USE_RW_MTX
      pthread_rwlock_unlock( &__ml );
#  endif
      return fcntl( fd, F_SETLKW, &_flu );
# endif /* __USE_FLOCK */
    default:
      errno = EOPNOTSUPP;
      return -1;
  }
#endif /* __unix || __unix__ */

#ifdef WIN32
  switch ( operation ) {
    case _F_LCK_W:
      /*
       * if you use 'fd = open( "myfile", O_CREAT, 00666 )',
       * pass first argument as (HANDLE)_get_osfhandle(fd)
       */
      return LockFileEx( (HANDLE)_get_osfhandle(fd), LOCKFILE_EXCLUSIVE_LOCK, 0, (DWORD)-1, (DWORD)-1, &_fl ) == TRUE ? 0 : -1;
    case _F_LCK_R:
      return LockFileEx((HANDLE) _get_osfhandle(fd), 0, 0, (DWORD)-1, (DWORD)-1, &_fl ) == TRUE ? 0 : -1;
    case _F_UNLCK:
      return UnlockFileEx( (HANDLE)_get_osfhandle(fd), 0, (DWORD)-1, (DWORD)-1, &_fl ) == TRUE ? 0 : -1;
    default:
      return -1;
  }
#endif /* WIN32 */
  /* errno = EOPNOTSUPP; */
  return -1;
}
