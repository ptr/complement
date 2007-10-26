// -*- C++ -*- Time-stamp: <07/03/07 16:38:24 ptr>

#include "intercessor_test_suite.h"

int main( int, char ** )
{
  // dir = boost::filesystem::system_complete( boost::filesystem::path( "/tmp",  boost::filesystem::native ) );
  int ret = 0;

  if ( ret = http_test_suite(0) ) {
    return ret;
  }

  return intercessor_test_suite(0);
}
