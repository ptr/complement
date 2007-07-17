// -*- C++ -*- Time-stamp: <07/07/13 11:01:52 ptr>

#ifndef __logger_h
#define __logger_h

#include <string>
#include <cstdio>
#include <ostream>

namespace exam {

class base_logger
{
  public:

    enum trace_flags {
      trace = 1,
      trace_suite = 2,
      silent = 4,
      verbose = 8
    };

    enum tc_result {
      pass = 0,
      fail,
      skip
    };

    struct stat
    {
       stat() :
         total(0),
         passed(0),
         failed(0),
         skipped(0)
       { }

       int total;
       int passed;
       int failed;
       int skipped;
    };

    base_logger() :
        _flags( 0 )
      { }
    virtual ~base_logger()
      { }

    int flags() const;
    bool is_trace() const;

    int flags( int );

    virtual void report( const char *, int, bool, const char * ) = 0;

    virtual void begin_ts() = 0;
    virtual void end_ts() = 0;
    virtual void result( const base_logger::stat&, const std::string& suite_name ) = 0;
    virtual void tc( tc_result, const std::string& ) = 0;

  protected:
    int _flags;
};

class trivial_logger :
    public base_logger
{
  public:
    explicit trivial_logger( std::ostream& str ) :
         s( &str ),
         f( 0 )
      { }

    explicit trivial_logger( FILE *fs ) :
         s( 0 ),
         f( fs )
      { }

    virtual void report( const char *, int, bool, const char * );

    virtual void begin_ts();
    virtual void end_ts();
    virtual void result( const base_logger::stat&, const std::string& );
    virtual void tc( base_logger::tc_result, const std::string& );

  private:
    std::ostream *s;
    FILE *f;
};

} // namespace exam

#endif // __logger_h