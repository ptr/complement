#define FIT_EXAM

#ifdef FIT_EXAM
#  define EXAM_CHECK(C) if ( !(C) ) { exam::report( __FILE__, __LINE__, false, #C ); } else if ( exam::base::is_trace() ) { exam::report( __FILE__, __LINE__, true, #C ); }
#  define EXAM_MESSAGE(M)
#else
#  define EXAM_CHECK(C)
#  define EXAM_MESSAGE(M)
#endif


#include <cstdio>
#include <iostream>

#include "suite.h"

namespace exam {

class base
{
  public:
    enum {
      trace = 1
    };
  
    static int flags();
    static bool is_trace();

  private:

    static int _flags;
};

void _report0( const char *file, int line, bool cnd, const char *expr )
{
  std::cerr << file << ":" << line << ": " << (cnd ? "pass" : "fail" ) << " \"" << expr << "\""
            << std::endl;
}

void _report1( const char *file, int line, bool cnd, const char *expr )
{
  printf( "%s:%d: %s \"%s\"\n", file, line, (cnd ? "pass" : "fail"), expr );
}

void _report2( const char *file, int line, bool cnd, const char *expr )
{
  fprintf( stderr, "%s:%d: %s \"%s\"\n", file, line, (cnd ? "pass" : "fail"), expr );
}

void (*report)( const char *, int, bool, const char * ) = _report0;

} // namespec exam

void func()
{
  EXAM_CHECK(false);
}

int main( int argc, char **argv )
{
  exam::test_suite t;

  t.add( func );

  t.girdle( 0 );

  return 0;
}

