// -*- C++ -*- Time-stamp: <03/07/23 01:44:34 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <boost/test/minimal.hpp>

int add( int a, int b )
{
  return a + b;
}

int test_main( int, char ** )
{
  BOOST_CHECK( add( 1, 2) == 4 );
  BOOST_REQUIRE( add( 1, 2) == 3 );

  if ( add( 1, 2) != 4 ) {
    BOOST_ERROR( "Hmm..." );
  }

  if ( add( 1, 2) != 4 ) {
    BOOST_FAIL( "Hmm..., throw" );
  }

  return (add( 1, 2) == 4 ? 0 : 1);
}
