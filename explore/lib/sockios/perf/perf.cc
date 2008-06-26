// -*- C++ -*- Time-stamp: <07/09/05 22:46:17 ptr>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_perf_suite.h"

int main( int, char ** )
{
  int flag = 0;

  flag += sockios_perf_suite_b(0);
  flag += sockios_perf_suite_c(0);
  flag += sockios_perf_suite_d(0);

  return flag;
}
