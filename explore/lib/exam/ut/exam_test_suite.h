// -*- C++ -*- Time-stamp: <07/07/16 23:40:09 ptr>

#ifndef __exam_test_suite_h
#define __exam_test_suite_h

#include <exam/suite.h>
#include <string>
#include <sstream>

class exam_basic_test
{
   public:
     exam_basic_test() :
         buff(),
         logger( buff )
       {  }

     int EXAM_DECL(function_good);
     int EXAM_DECL(function);
     int EXAM_DECL(dep);
     int EXAM_DECL(trace);
     int EXAM_DECL(dep_test_suite);
     int EXAM_DECL(multiple_dep);
     int EXAM_DECL(multiple_dep_complex);

  private:
     std::stringstream buff;
     exam::trivial_logger logger;

     static const std::string r0;
     static const std::string r1;
     static const std::string r2;
     static const std::string r3;
     static const std::string r4;
     static const std::string r5;
     static const std::string r6;
     static const std::string r7;
     static const std::string r8;
     static const std::string r9;
};

int EXAM_DECL(exam_self_test);

#endif // __exam_test_suite_h
