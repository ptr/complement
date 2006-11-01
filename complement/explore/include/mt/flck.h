
#if defined(__unix) || defined(__unix__)
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#ifndef __flck_h
#define __flck_h

#ifdef __cplusplus
extern "C" {
#endif /* __c_plus_plus */

/* flags for following flck call: */

#define _F_LCK_W 0   /* lock file for writing (exclusive lock) */
#define _F_LCK_R 1   /* lock file for reading (shared lock) */
#define _F_UNLCK 2   /* unlock file */

/* #if defined(__unix) || defined(__unix__) */
int flck( int fd, int operation );  /* lock/unlock file, use flags above */
/* #endif */

/* #ifdef WIN32 */
/* int flck( HANDLE fd, int operation ); */ /* lock/unlock file, use flags above */
/* #endif */

#ifdef __cplusplus
}
#endif /* __c_plus_plus */

#endif /* __flck_h */
