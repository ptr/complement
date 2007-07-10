#define FIT_EXAM

#include "suite.h"

void func()
{
  EXAM_CHECK(false);
}

class test_x
{
  public:

    void f()
      {
        EXAM_CHECK(false);
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

