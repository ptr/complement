#ifndef __la_test_h
#define __la_test_h

#include <exam/suite.h>

class la_test
{
  public:
    int EXAM_DECL(one_char);
    int EXAM_DECL(one_short);
    int EXAM_DECL(two_chars);
    int EXAM_DECL(boundary_32bit);
};


#endif // __la_test_h
