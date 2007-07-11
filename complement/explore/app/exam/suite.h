// -*- C++ -*- Time-stamp: <07/07/11 11:02:41 ptr>

#ifndef __suite_h
#define __suite_h

#include <iostream>
#include <sstream>
#include <map>
#include <boost/graph/adjacency_list.hpp>

enum vertex_testcase_t { vertex_testcase };

namespace boost {
  BOOST_INSTALL_PROPERTY( vertex, testcase );
} // namespace boost

namespace exam {

namespace detail {

struct call_impl
{
    virtual ~call_impl()
      { }
    virtual int invoke() = 0;
};

template <typename F>
class call_impl_t :
        public call_impl
{
  public:
    explicit call_impl_t( F f ) :
        _f( f )
      { }

    virtual int invoke()
      { return _f(); }

  private:
    F _f;
};

class dummy
{
  public:
    virtual int f()
      { }
  private:
    virtual ~dummy()
      { }
};

template <class TC>
class method_invoker
{
  public:
    typedef int (TC::*mf_type)();

    explicit method_invoker( TC& instance, mf_type f ) :
        _inst(instance),
        _func(f)
      { }

    method_invoker( const method_invoker<TC>& m ) :
        _inst( m._inst ),
        _func( m._func )
      { }

    int operator()()
      { return (_inst.*_func)(); }

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

    int operator()()
      { return reinterpret_cast<call_impl *>(&_buf[0])->invoke(); }

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

    int operator ()()
      { return _tc(); }

  private:
    call _tc;
};

inline test_case *make_test_case( const call& f )
{
  return new test_case( f );
}

template <class TC>
inline test_case *make_test_case( int (TC::*f)(), TC& instance )
{
  return new test_case( method_invoker<TC>(instance, f) );
}

} // namespace detail

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
    typedef int (*func_type)();
    typedef vertex_t test_case_type;

    test_suite();
    ~test_suite();

    test_case_type add( func_type );
    test_case_type add( func_type, test_case_type );

    template <class TC>
    test_case_type add( int (TC::*)(), TC& );

    template <class TC>
    test_case_type add( int (TC::*)(), TC&, test_case_type );

    void girdle();
    void girdle( test_case_type start );

    void run_test_case( vertex_t v );
    void check_test_case( vertex_t u, vertex_t v );

    enum {
      trace = 1
    };

    static int flags();
    static bool is_trace();
    static void report( const char *, int, bool, const char * );

  private:
    enum {
      fail = 1,
      skip = 2
    };

    graph_t g;
    vertex_t root;
    vertex_color_map_t color;
    vertex_testcase_map_t testcase;

    struct test_case_collect
    {
        detail::test_case *tc;
        int state;
    };

    typedef std::map<vertex_t,test_case_collect> test_case_map_type;
    test_case_map_type _test;

    static int _flags;
    static void (*_report)( const char *, int, bool, const char * );
};

template <class TC>
test_suite::test_case_type test_suite::add( int (TC::*f)(), TC& instance )
{
  vertex_t v = boost::add_vertex( boost::white_color, g);
  boost::add_edge( root, v, g );
  _test[v].tc = detail::make_test_case( f, instance );
  _test[v].state = 0;

  return v;
}

template <class TC>
test_suite::test_case_type test_suite::add( int (TC::*f)(), TC& instance, test_suite::test_case_type depends )
{
  vertex_t v = boost::add_vertex( boost::white_color, g);
  boost::add_edge( depends, v, g );
  _test[v].tc = detail::make_test_case( f, instance );
  _test[v].state = 0;

  return v;
}

typedef test_suite::test_case_type test_case_type;

} // namespace exam

#ifdef FIT_EXAM
#  define EXAM_CHECK(C) if ( !(C) ) { exam::test_suite::report( __FILE__, __LINE__, false, #C );  return 1; } else if ( exam::test_suite::is_trace() ) { exam::test_suite::report( __FILE__, __LINE__, true, #C ); }
#  define EXAM_MESSAGE(M)
#else
#  define EXAM_CHECK(C)
#  define EXAM_MESSAGE(M)
#endif


#endif // __suite_h

