#define FIT_EXAM

#include "suite.h"

int func()
{
  EXAM_CHECK(false);

  return 0;
}

int func2()
{
  EXAM_CHECK(true);

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
  exam::test_suite t( "exam level 0" );

  test_x tx;

  t.add( func, "simple function" );
  t.add( &test_x::f, tx, "member function" );

  t.add( func, "simple function, depends",
         t.add( &test_x::f, tx, "member function, depends",
                t.add( func2, "simple good function" ) ) );

  t.girdle();

  return 0;
}

