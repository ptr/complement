//$Id$
#ifndef _CPPUNITMINIFILEREPORTERINTERFACE_H_
#   define _CPPUNITMINIFILEREPORTERINTERFACE_H_

#include <stdio.h>
//
// CppUnit mini file(stream) reporter
//
class FileReporter : public CPPUNIT_NS::Reporter
{
private:
    FileReporter(const FileReporter& mr) {}
    const FileReporter& operator=(const FileReporter& mr) { return *this; }
public:
    FileReporter() : m_numErrors(0), m_numTests(0), _myStream(false) { _file=stderr; }
    FileReporter(const char* file) : m_numErrors(0), m_numTests(0), _myStream(true) { _file=fopen(file, "w");  }
    FileReporter(FILE* stream) : m_numErrors(0), m_numTests(0), _myStream(false) { _file=stream;  }

    virtual ~FileReporter() { if(_myStream) fclose(_file); else fflush(_file);  }
    
    virtual void error(char *in_macroName, char *in_macro, char *in_file, int in_line) 
    {
        m_numErrors++;
        fprintf(_file, "\n%s(%d) : %s(%s);\n", in_file, in_line, in_macroName, in_macro);
    }
  
    virtual void progress(char *in_className, char *in_shortTestName) 
    {
        m_numTests++;
        fprintf(_file, "%s::%s\n", in_className, in_shortTestName);
    }
    virtual void printSummary() 
    {
        if(m_numErrors > 0) {
            fprintf(_file, "There were errors! (%d of %d)\n", m_numErrors, m_numTests);
        }
        else {
            fprintf(_file, "\nOK (%d)\n\n", m_numTests);
        }
    }
private:
    int m_numErrors;
    int m_numTests;
    bool  _myStream;
    FILE* _file;
};

#endif /*_CPPUNITMINIFILEREPORTERINTERFACE_H_*/
