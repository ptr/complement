// -*- C++ -*-

/*
 * Copyright (c) 2019
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "hashes_test.h"

#include "misc/crc16-ccitt.h"

using namespace std;

int EXAM_IMPL(hashes_test::crc16_ccitt_hdsl)
{
  crc16_type crc;
  crc16_bytes_selector b;

  crc = crc16_ccitt_init();

  EXAM_CHECK(crc == 0xffff);

  b.crc = crc16_ccitt_final(crc);

  EXAM_CHECK(b.byte.low == 0);
  EXAM_CHECK(b.byte.high == 0);

  crc = crc16_ccitt_byte(crc, 0);
  crc = crc16_ccitt_byte(crc, 0);
  crc = crc16_ccitt_byte(crc, 0);

  b.crc = crc16_ccitt_final(crc);

  EXAM_CHECK(b.byte.low == 0xcc);
  EXAM_CHECK(b.byte.high == 0xc6);

  return EXAM_RESULT;
}
