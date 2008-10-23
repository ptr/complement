// -*- C++ -*- Time-stamp: <08/05/21 12:33:01 yeti>

/*
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <exam/suite.h>
#include <config/feature.h>

#include "misc_test_suite.h"

int main( int, char ** )
{
  int res1 = misc_test_suite(0);

  int res2 = options_test_suite(0);

  return res1 || res2;
}
