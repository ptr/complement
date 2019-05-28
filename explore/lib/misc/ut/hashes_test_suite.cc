// -*- C++ -*-

/*
 * Copyright (c) 2019
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "hashes_test.h"

#include <config/feature.h>

void hashes_test_suite_init( exam::test_suite& t, hashes_test& test )
{
  exam::test_suite::test_case_type tc[10];

  tc[0] = t.add( &hashes_test::crc16_ccitt_hdsl, test, "CRC16 CCITT (ISO/IEC 13239)" );
}
