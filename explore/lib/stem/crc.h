// -*- C++ -*- Time-stamp: <03/11/06 07:59:03 ptr>

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

#ifndef __crc_h
#define __crc_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <cstdlib>

namespace std {

enum CRC_const
{
  CRC24_INIT = 0x0b704ceL,
  CRC32_INIT = 0L,
  ADLER32_INIT = 1L
};

typedef unsigned long crc24_type;

/*
  Update a running crc with the bytes buf[0..len-1] and return
  the updated crc. The crc should be initialized to zero. Pre- and
  post-conditioning (one's complement) is performed within this
  function so it shouldn't be done by the caller. Usage example:

  crc24_type crc = CRC24_INIT;

  while ( read_buffer( buffer, length) != EOF ) {
    crc = crc24_update( crc, buffer, length );
  }
  if ( crc != original_crc ) error();
*/

crc24_type crc24_update( crc24_type crc, unsigned char *octets, size_t len );

/* Return the CRC24 of the bytes buf[0..len-1]. */
inline crc24_type crc24( unsigned char *octets, size_t len )
{ return crc24_update( CRC24_INIT, octets, len ); }

typedef unsigned long crc32_type;

/*
  Update a running crc with the bytes buf[0..len-1] and return
  the updated crc. The crc should be initialized to zero. Pre- and
  post-conditioning (one's complement) is performed within this
  function so it shouldn't be done by the caller. Usage example:

  crc32_type crc = 0L;

  while ( read_buffer( buffer, length) != EOF ) {
    crc = crc32_update( crc, buffer, length );
  }
  if ( crc != original_crc ) error();
*/

crc32_type crc32_update( crc32_type crc, unsigned char *buf, int len );

/* Return the CRC32 of the bytes buf[0..len-1]. */
inline crc32_type crc( unsigned char *buf, int len )
{ return crc32_update( 0L, buf, len ); }


typedef unsigned long adler32_type;

/*
  Update a running Adler-32 checksum with the bytes buf[0..len-1]
  and return the updated checksum. The Adler-32 checksum should be
  initialized to 1.

  Usage example:

  adler32_type adler = 1L;

  while ( read_buffer(buffer, length) != EOF ) {
    adler = adler32_update( adler, buffer, length );
  }
  if ( adler != original_adler ) error();
*/

adler32_type adler32_update( adler32_type adler, unsigned char *buf, int len );

/* Return the adler32 of the bytes buf[0..len-1] */
inline adler32_type adler32( unsigned char *buf, int len )
{ return adler32_update( 1L, buf, len ); }

} // namespace std

#endif // __crc.h
