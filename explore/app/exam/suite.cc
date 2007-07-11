// -*- C++ -*- Time-stamp: <07/07/11 11:10:45 ptr>

#include "suite.h"
#include <boost/graph/breadth_first_search.hpp>
#include <stack>

#include <cstdio>
#include <iostream>

namespace exam {

using namespace std;
using namespace boost;
using namespace detail;

namespace detail {

template <class Tag>
struct vertex_recorder :
        public base_visitor<vertex_recorder<Tag> >
{
    typedef Tag event_filter;

    vertex_recorder(test_suite& ts) :
        _suite(ts)
      { }

    template <class Vertex, class Graph>
    void operator()(Vertex v, const Graph& g)
      { _suite.run_test_case( v ); }

    test_suite& _suite;
};

template <class Tag>
vertex_recorder<Tag> record_vertexes(test_suite& ts, Tag)
{ return vertex_recorder<Tag>(ts); }

template <class Tag>
struct skip_recorder :
        public base_visitor<skip_recorder<Tag> >
{
    typedef Tag event_filter;

    skip_recorder(test_suite& ts) :
        _suite(ts)
      { }

    template <class Edge, class Graph>
    void operator()(Edge e, const Graph& g)
      { 
        // typename graph_traits<Graph>::vertex_descriptor u = boost::source( e, g );
        // typename graph_traits<Graph>::vertex_descriptor v = boost::target( e, g );
        _suite.check_test_case( boost::source( e, g ), boost::target( e, g ) );
      }

    test_suite& _suite;
};

template <class Tag>
skip_recorder<Tag> record_skip(test_suite& ts, Tag)
{ return skip_recorder<Tag>(ts); }

int _root_func()
{
  return 0;
}

} // namespace detail

test_suite::test_suite() :
   root( add_vertex( white_color, g ) )
{
  color = get( vertex_color, g );
  testcase = get( vertex_testcase, g );
  _test[root].tc = detail::make_test_case( detail::call( _root_func ) );
  _test[root].state = 0;
}

test_suite::~test_suite()
{
  for ( test_case_map_type::iterator i = _test.begin(); i != _test.end(); ++i ) {
    delete i->second.tc;
  }
}

void test_suite::girdle()
{
  stack<vertex_t> buffer;
  breadth_first_visit( g, root, buffer,
                       make_bfs_visitor(
                         make_pair( record_vertexes(*this,on_discover_vertex()),
                                    record_skip(*this,on_examine_edge()) ) ),
                       color );
}

void test_suite::girdle( test_suite::test_case_type start )
{
  stack<vertex_t> buffer;
  list<vertex_t> v;
  breadth_first_visit( g, start, buffer,
                       make_bfs_visitor(
                         make_pair( record_vertexes(*this,on_discover_vertex()),
                                    record_skip(*this,on_examine_edge()) ) ),
                       color );
}


test_suite::test_case_type test_suite::add( test_suite::func_type f )
{
  vertex_t v = add_vertex( white_color, g);
  add_edge( root, v, g );
  _test[v].tc = detail::make_test_case( detail::call( f ) );
  _test[v].state = 0;

  return v;
}

test_suite::test_case_type test_suite::add( test_suite::func_type f, test_suite::test_case_type depends )
{
  vertex_t v = add_vertex( white_color, g);
  add_edge( depends, v, g );
  _test[v].tc = detail::make_test_case( detail::call( f ) );
  _test[v].state = 0;

  return v;
}

int test_suite::flags()
{
  return _flags;
}

bool test_suite::is_trace()
{
  return (_flags & trace) != 0;
}

void _report0( const char *file, int line, bool cnd, const char *expr )
{
  std::cerr << file << ":" << line << ": " << (cnd ? "pass" : "fail" ) << ": " << expr
            << std::endl;
}

void _report1( const char *file, int line, bool cnd, const char *expr )
{
  printf( "%s:%d: %s: %s\n", file, line, (cnd ? "pass" : "fail"), expr );
}

void _report2( const char *file, int line, bool cnd, const char *expr )
{
  fprintf( stderr, "%s:%d: %s: %s\n", file, line, (cnd ? "pass" : "fail"), expr );
}

int test_suite::_flags = 0;
void (*test_suite::_report)( const char *, int, bool, const char * ) = _report0;

void test_suite::report( const char *file, int line, bool cnd, const char *expr )
{
  (*test_suite::_report)( file, line, cnd, expr );
}

void test_suite::run_test_case( test_suite::vertex_t v )
{
  try {
    if ( _test[v].state == 0 ) {
      if ( (*_test[v].tc)() != 0 ) {
        _test[v].state = fail;
      }
    }
  }
  catch ( ... ) {
    _test[v].state = fail;
  }
}

void test_suite::check_test_case( test_suite::vertex_t u, test_suite::vertex_t v )
{
  if ( _test[u].state != 0 ) {
    _test[v].state = skip;
  }
}

} // namespace exam
