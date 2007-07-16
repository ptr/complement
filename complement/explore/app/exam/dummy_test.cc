#include "suite.h"

int EXAM_IMPL(func)
{
  EXAM_CHECK(false);

  return EXAM_RESULT;
}

class test_x
{
  public:

    int EXAM_IMPL(f)
      {
        EXAM_CHECK(false);
        EXAM_CHECK(true);

        return EXAM_RESULT;
      }

    int EXAM_IMPL(f_good)
      {
        EXAM_CHECK(true);
        EXAM_CHECK(true);

        return EXAM_RESULT;
      }
};

int EXAM_IMPL(func_good)
{
  EXAM_CHECK(true);

  return EXAM_RESULT;
}
