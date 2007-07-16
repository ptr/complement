// -*- C++ -*- Time-stamp: <07/07/16 23:07:02 ptr>

#ifndef __suite_h
#define __suite_h

#include <iostream>
#include <sstream>
#include <map>
#include <boost/graph/adjacency_list.hpp>
#include <string>
#include <exception>

#include "logger.h"

enum vertex_testcase_t { vertex_testcase };

namespace boost {
  BOOST_INSTALL_PROPERTY( vertex, testcase );
} // namespace boost

namespace exam {

class test_suite;

namespace detail {

struct call_impl
{
    virtual ~call_impl()
      { }
    // virtual int invoke() = 0;
    virtual int invoke( test_suite *, int = 0 ) = 0;
};

template <typename F>
class call_impl_t :
        public call_impl
{
  public:
    explicit call_impl_t( F f ) :
        _f( f )
      { }

    // virtual int invoke()
    //  { return _f(); }

    virtual int invoke( test_suite *s, int count = 0 )
      { return _f( s, count ); }


  private:
    F _f;
};

class dummy
{
  public:
    // virtual int f()
    //   { return 0; }

    virtual int f( test_suite *, int count = 0 )
      { return count; }

  private:
    virtual ~dummy()
      { }
};

template <class TC>
class method_invoker
{
  public:
    // typedef int (TC::*mf_type_a)();
    typedef int (TC::*mf_type)( test_suite *, int );

    explicit method_invoker( TC& instance, mf_type f ) :
        _inst(instance),
        _func(f)
      { }

    method_invoker( const method_invoker<TC>& m ) :
        _inst( m._inst ),
        _func( m._func )
      { }

    // int operator()()
    //   { return (_inst.*_func)(); }

    int operator()( test_suite *ts, int count = 0 )
      { return (_inst.*_func)( ts, count ); }

  private:
    method_invoker& operator =( const method_invoker<TC>& )
      { return *this; }

    TC& _inst;
    mf_type _func;
};

class call
{
  public:
    call()
      { }

    template <class F>
    call( F f )
      { new (&_buf[0]) call_impl_t<F>(f); }

    // int operator()()
    //   { return reinterpret_cast<call_impl *>(&_buf[0])->invoke(); }

    int operator()( test_suite *ts, int count = 0 )
      { return reinterpret_cast<call_impl *>(&_buf[0])->invoke( ts, count ); }

  private:
    // call_impl *_f;
    char _buf[((sizeof(call_impl_t<method_invoker<dummy> >)+64) / 64) << 6];
};


class test_case
{
  public:
    test_case( const call& f ) :
        _tc( f )
      { }

    // int operator ()()
    //   { return _tc(); }

    int operator ()( test_suite *ts, int count = 0 )
      { return _tc( ts, count ); }

  private:
    call _tc;
};

inline test_case *make_test_case( const call& f )
{
  return new test_case( f );
}

template <class TC>
inline test_case *make_test_case( int (TC::*f)( test_suite *, int ), TC& instance )
{
  return new test_case( method_invoker<TC>(instance, f) );
}

} // namespace detail

class init_exception :
  public std::exception
{
};

class test_suite
{
  private:
    typedef boost::property<vertex_testcase_t,int> TestCaseProperty;
    typedef boost::property<boost::vertex_color_t, boost::default_color_type, TestCaseProperty> VColorProperty;

    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VColorProperty > graph_t;
    typedef boost::graph_traits<graph_t>::vertex_iterator vertex_iterator_t;

    typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
    typedef boost::property_map<graph_t,boost::vertex_color_t>::type vertex_color_map_t;
    typedef boost::property_map<graph_t,vertex_testcase_t>::type vertex_testcase_map_t;

  public:
    typedef int (*func_type)( test_suite *, int );
    typedef vertex_t test_case_type;

    test_suite( const std::string& name );
    test_suite( const char *name );
    ~test_suite();

    test_case_type add( func_type, const std::string& name );
    test_case_type add( func_type, const std::string& name, test_case_type );

    template <class InputIter>
    test_case_type add( func_type, const std::string& name, InputIter, InputIter );

    template <class TC>
    test_case_type add( int (TC::*)( test_suite *, int ), TC&, const std::string& name );

    template <class TC>
    test_case_type add( int (TC::*)( test_suite *, int ), TC&, const std::string& name, test_case_type );

    template <class TC, class InputIter>
    test_case_type add( int (TC::*)( test_suite *, int ), TC&, const std::string& name, InputIter, InputIter );

    int girdle( test_case_type start );
    int girdle()
      { return girdle( root ); }
    int run( test_suite *, int count = 0 );

    void run_test_case( vertex_t v );
    void check_test_case( vertex_t u, vertex_t v );
    void clean_test_case_state( vertex_t v );

    int flags();
    int flags( int );
    bool is_trace();
    void report( const char *, int, bool, const char * );
    base_logger *set_global_logger( base_logger * );
    base_logger *set_logger( base_logger * );

  private:
    enum {
      pass = 0,
      fail = 1,
      skip = 2
    };

    graph_t g;
    vertex_t root;
    vertex_testcase_map_t testcase;
    base_logger *local_logger;

    struct test_case_collect
    {
        detail::test_case *tc;
        int state;
        std::string name;
    };

    typedef std::map<vertex_t,test_case_collect> test_case_map_type;
    test_case_map_type _test;
    base_logger::stat _stat;
    std::string _suite_name;

    static int _root_func( test_suite *, int = 0 );

    static base_logger *logger;
};

template <class TC>
test_suite::test_case_type test_suite::add( int (TC::*f)( test_suite *, int ), TC& instance, const std::string& name )
{
  vertex_t v = boost::add_vertex( boost::white_color, g);
  boost::add_edge( root, v, g );
  _test[v].tc = detail::make_test_case( f, instance );
  _test[v].state = 0;
  _test[v].name = name;
  // ++_stat.total;

  return v;
}

template <class InputIter>
test_suite::test_case_type test_suite::add( test_suite::func_type f, const std::string& name, InputIter first, InputIter last )
{
  vertex_t v = boost::add_vertex( boost::white_color, g);
  while ( first != last ) {
    boost::add_edge( *first++, v, g );
  }
  _test[v].tc = detail::make_test_case( detail::call( f ) );
  _test[v].state = 0;
  _test[v].name = name;
  // ++_stat.total;

  return v;
}

template <class TC>
test_suite::test_case_type test_suite::add( int (TC::*f)( test_suite *, int ), TC& instance, const std::string& name, test_suite::test_case_type depends )
{
  vertex_t v = boost::add_vertex( boost::white_color, g);
  boost::add_edge( depends, v, g );
  _test[v].tc = detail::make_test_case( f, instance );
  _test[v].state = 0;
  _test[v].name = name;
  // ++_stat.total;

  return v;
}

template <class TC, class InputIter>
test_suite::test_case_type test_suite::add( int (TC::*f)( test_suite *, int ), TC& instance, const std::string& name, InputIter first, InputIter last )
{
  vertex_t v = boost::add_vertex( boost::white_color, g);
  while ( first != last ) {
    boost::add_edge( *first++, v, g );
  }
  _test[v].tc = detail::make_test_case( f, instance );
  _test[v].state = 0;
  _test[v].name = name;
  // ++_stat.total;

  return v;
}

typedef test_suite::test_case_type test_case_type;

} // namespace exam

#ifdef FIT_EXAM
#  define EXAM_IMPL(F) F( exam::test_suite *__exam_ts, int __exam_counter )
#  define EXAM_DECL(F) F( exam::test_suite *, int = 0 )
#  define EXAM_RESULT __exam_counter
#  define EXAM_CHECK(C) if ( !(C) ) { __exam_ts->report( __FILE__, __LINE__, false, #C );  __exam_counter |= 1; } else __exam_ts->report( __FILE__, __LINE__, true, #C )
#  define EXAM_MESSAGE(M) __exam_ts->report( __FILE__, __LINE__, true, M )
#  define EXAM_REQUIRE(C) if ( !(C) ) { __exam_ts->report( __FILE__, __LINE__, false, #C );  return 1; } else __exam_ts->report( __FILE__, __LINE__, true, #C )
#  define EXAM_FAIL(M) __exam_ts->report( __FILE__, __LINE__, false, M ); return 1
#  define EXAM_ERROR(M) __exam_ts->report( __FILE__, __LINE__, false, M ); __exam_counter |= 1
#else
#  define EXAM_IMPL(F) F( exam::test_suite *, int )
#  define EXAM_DECL(F) F( exam::test_suite *, int = 0 )
#  define EXAM_RESULT 0
#  define EXAM_CHECK(C)
#  define EXAM_MESSAGE(M)
#  define EXAM_REQUIRE(C)
#  define EXAM_FAIL(M)
#  define EXAM_ERROR(M)
#endif


#endif // __suite_h

