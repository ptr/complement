#include "cppunit_proxy.h"
#include "file_reporter.h"

#ifdef CPPUNIT_DUMMY
int main( int argc, char* argv[] ) 
{ 
  return 0; 
}
#elif CPPUNIT_MINI

namespace CPPUNIT_NS
{
  int CPPUNIT_NS::TestCase::m_numErrors = 0;
  int CPPUNIT_NS::TestCase::m_numTests = 0;

  CPPUNIT_NS::TestCase *CPPUNIT_NS::TestCase::m_root = 0;
  CPPUNIT_NS::Reporter *CPPUNIT_NS::TestCase::m_reporter = 0;

  void CPPUNIT_NS::TestCase::registerTestCase(TestCase *in_testCase)
  {
    in_testCase->m_next = m_root;
    m_root = in_testCase;
  }

  int CPPUNIT_NS::TestCase::run(Reporter *in_reporter, const char *in_testName)
  {
    TestCase::m_reporter = in_reporter;

    m_numErrors = 0;
    m_numTests = 0;

    TestCase *tmp = m_root;
    while(tmp != 0)
    {
      tmp->myRun(in_testName);
      tmp = tmp->m_next;
    }
    return m_numErrors;
  }  
}
# ifdef UNDER_CE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  int argc=1;
  size_t size=wcslen(lpCmdLine);
  char* buff=new char[size+1];
  wcstombs(buff, lpCmdLine, size);
  buff[size]=0;
  char* bp=buff;

  char* argv[3];  // limit to three args
  argv[0]=0;
  argv[1]=0;
  argv[2]=0;
  while(*bp && argc<3) {
    if(*bp==' ' ||* bp=='\t') {
      bp++;
      continue;
    }
    if(*bp=='-') {
      char* start=bp;
      while(*bp && *bp!=' ' && *bp!='\t')
        bp++;
      argv[argc]=new char[bp-start+1];
      strncpy(argv[argc], start, bp-start);
      argv[argc][bp-start]=0;
      argc++;
    }
  }
  
  delete[] buff;

# else
int main(int argc, char** argv)
{
# endif

  // CppUnit(mini) test launcher
  // command line option syntax:
  // test [OPTIONS]
  // where OPTIONS are
  //  -t=CLASS[::TEST]    run the test class CLASS or member test CLASS::TEST
  //  -f=FILE             save output in file FILE instead of stdout

  int num_errors=0;
  char *fileName=0;
  char *testName="";

  for(int i=1; i<argc; i++) {
    if(argv[i][0]!='-')
      break;
    if(!strncmp(argv[i], "-t=", 3)) {
      testName=argv[i]+3;
    }
    else if(!strncmp(argv[i], "-f=", 3)) {
      fileName=argv[i]+3;
    }
  }
  CPPUNIT_NS::Reporter*   reporter=0;
  if(fileName)
      reporter=new FileReporter(fileName);
  else
      reporter=new FileReporter(stdout);

  num_errors=CPPUNIT_NS::TestCase::run(reporter, testName);
  reporter->printSummary();
  delete reporter;

# ifdef UNDER_CE
  // free args
  delete[] argv[1];
  delete[] argv[2];
# endif  

  return num_errors;
}
#endif
