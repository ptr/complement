/*
 * Cleaned up version of the md5.h header file from RFC 1321.
 */

/*
 * MD5.H - header file for MD5C.C
 */

/*
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 *
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 *
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 *
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */

#ifndef __aux_md5_h
#define __aux_md5_h

#include <stdint.h>
#include <sys/types.h>

#ifdef  __cplusplus
extern "C" {
#endif

#if defined(__hpux) && !defined(_INO32_T) // seems this is HP-UX ~10.01
typedef unsigned uint32_t;
typedef unsigned char uint8_t;
#endif

/* MD5 context. */
typedef struct {
  uint32_t state[4];        /* state (ABCD) */
  uint32_t count[2];        /* number of bits, modulo 2^64 (lsb first) */
  union {
    uint8_t  buf8[64];      /* input buffer */
    uint32_t buf32[16];     /* realigned input */
  } buf_un;
} MD5_CTX;

#ifdef __hpux /* really for cool Oracle's programmers
                 in pair with HP's wonderful linker */
#  define MD5Init   __MD5Init__
#  define MD5Update __MD5Update__
#  define MD5Final  __MD5Final__
#endif /* __hpux */

void MD5Init( MD5_CTX* );
void MD5Update( MD5_CTX*, const uint8_t*, size_t );
void MD5Final( uint8_t [16], MD5_CTX* );

#ifdef  __cplusplus
}
#endif

#endif /* __aux_md5_h */
