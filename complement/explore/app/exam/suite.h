// -*- C++ -*- Time-stamp: <07/07/06 09:58:06 ptr>

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

/*
struct invoker
{
    template <typename F>
    void invoke( F& f )
      { f(); }
};
*/

struct call_impl
{
    virtual ~call_impl()
      { }
    virtual void invoke() = 0;
};

template <typename F>
class call_impl_t :
        public call_impl
{
  public:
    explicit call_impl_t( F f ) :
        _f( f )
      { }

    virtual void invoke()
      { /* invoker().invoke( _f ); */ _f(); }

  private:
    F _f;
};

class call
{
  public:
    call()
      { }

    template <class F>
    call( F f ) :
        _f( new call_impl_t<F>(f) )
      { }

    void operator()()
      { _f->invoke(); }

  private:
    call_impl *_f;
};

template <class TC>
class method_invoker
{
  public:
    typedef void (TC::*mf_type)();
    method_invoker( TC& instance, mf_type f ) :
        _inst(instance),
        _func(f)
      { }

    void operator()()
      { _inst.*_func(); }

  private:
    TC& _inst;
    mf_type _func;
};

class test_case
{
  public:
    test_case( const call& f ) :
        _tc( f )
      { }

    void operator ()()
      { _tc(); }

  private:
    call _tc;
};

inline test_case *make_test_case( const call& f )
{
  return new test_case( f );
}

template <class TC>
inline test_case *make_test_case( void (TC::*f)(), TC& instance )
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
    typedef void (*func_type)();
    typedef vertex_t test_case_type;

    test_suite();

    test_case_type add( func_type );
    test_case_type add( func_type, test_case_type );

    void girdle( int start );

  private:
    graph_t g;
    vertex_t root;
    vertex_color_map_t color;
    vertex_testcase_map_t testcase;

    std::map<vertex_t,detail::test_case *> _test;
};

typedef test_suite::test_case_type test_case_type;

} // namespace exam

#endif // __suite_h

