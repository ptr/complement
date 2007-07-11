#define FIT_EXAM

#include "suite.h"

int func()
{
  EXAM_CHECK(false);

  return 0;
}

class test_x
{
  public:

    int f()
      {
        EXAM_CHECK(false);

        return 0;
      }
};


int main( int argc, char **argv )
{
  exam::test_suite t;

  test_x tx;

  t.add( func );
  t.add( &test_x::f, tx );

  t.girdle( 0 );

  return 0;
}

