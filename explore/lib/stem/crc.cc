// -*- C++ -*- Time-stamp: <03/11/06 07:58:48 ptr>

/*
 *
 * Copyright (c) 1998-1999, 2002, 2003
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 2.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <config/feature.h>
#include "crc.h"

namespace std {

crc24_type crc24_update( crc24_type crc, unsigned char *octets, size_t len )
{
  int i;

  while ( len-- ) {
    crc ^= (*octets++) << 16;
    for ( i = 0; i < 8; i++ ) {
      crc <<= 1;
      if ( crc & 0x1000000L )
        crc ^= 0x1864cfbL;
    }
  }
  return crc & 0xffffffL;
}

/* Table of CRCs of all 8-bit messages. */
crc32_type crc_table[256];

/* Flag: has the table been computed? Initially false. */
bool crc_table_computed = false;

/* Make the table for a fast CRC. */
void make_crc_table()
{
  crc32_type c;

  int n, k;
  for (n = 0; n < 256; n++) {
    c = (crc32_type) n;
    for ( k = 0; k < 8; k++ ) {
      if (c & 1) {
        c = 0xedb88320L ^ (c >> 1);
      } else {
        c = c >> 1;
      }
    }
    crc_table[n] = c;
  }
  crc_table_computed = true;
}

crc32_type crc32_update( crc32_type crc, unsigned char *buf, int len )
{
  crc32_type c = crc ^ 0xffffffffL;

  if ( !crc_table_computed )
    make_crc_table();
  while ( len-- ) {
    c = crc_table[(c ^ *buf++) & 0xff] ^ (c >> 8);
  }
  return c ^ 0xffffffffL;
}

#define BASE 65521 /* largest prime smaller than 65536 */

adler32_type adler32_update( adler32_type adler, unsigned char *buf, int len )
{
  adler32_type s1 = adler & 0xffff;
  adler32_type s2 = (adler >> 16) & 0xffff;

  while ( len-- ) {
    s1 = (s1 + *buf++) % BASE;
    s2 = (s2 + s1)     % BASE;
  }
  return (s2 << 16) + s1;
}

} // namespace std
