// -*- C++ -*- Time-stamp: <99/09/10 14:36:40 ptr>

#ident "$SunId$ %Q%"

#include <crc.h>

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
