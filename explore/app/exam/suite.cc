// -*- C++ -*- Time-stamp: <07/07/08 23:39:10 ptr>

#include "suite.h"
#include <boost/graph/breadth_first_search.hpp>
#include <stack>

#include <cstdio>
#include <iostream>

namespace exam {

using namespace std;
using namespace boost;


template <class VertexList, class Tag>
struct vertex_recorder :
        public base_visitor<vertex_recorder<VertexList,Tag> >
{
    typedef Tag event_filter;

    vertex_recorder(VertexList& pa) :
        m_vertex(pa)
      { }

    template <class Vertex, class Graph>
    void operator()(Vertex v, const Graph& g)
      { m_vertex.push_back( v ); }

    VertexList& m_vertex;
};

template <class VertexList, class Tag>
vertex_recorder<VertexList, Tag> record_vertexes(VertexList& pa, Tag)
{ return vertex_recorder<VertexList, Tag>(pa); }

test_suite::test_suite() :
   root( add_vertex( white_color, g ) )
{
  color = get( vertex_color, g );
  testcase = get( vertex_testcase, g );
  _test[root] = 0;
}

test_suite::~test_suite()
{
  for ( test_case_map_type::iterator i = _test.begin(); i != _test.end(); ++i ) {
    delete i->second;
  }
}

void test_suite::girdle()
{
  stack<vertex_t> buffer;
  list<vertex_t> v;
  breadth_first_visit( g, root, buffer,
                       make_bfs_visitor(record_vertexes(v,on_discover_vertex())),
                       color );

  v.pop_front(); // remove root, it empty

  for ( list<vertex_t>::const_iterator i = v.begin(); i != v.end(); ++i ) {
    try {
      (*_test[*i])();
    }
    catch ( ... ) {
    }
  }
}

void test_suite::girdle( test_suite::test_case_type start )
{
  stack<vertex_t> buffer;
  list<vertex_t> v;
  breadth_first_visit( g, start, buffer,
                       make_bfs_visitor(record_vertexes(v,on_discover_vertex())),
                       color );

  v.pop_front(); // remove root, it empty

  for ( list<vertex_t>::const_iterator i = v.begin(); i != v.end(); ++i ) {
    try {
      (*_test[*i])();
    }
    catch ( ... ) {
    }
  }
}


test_suite::test_case_type test_suite::add( test_suite::func_type f )
{
  vertex_t v = add_vertex( white_color, g);
  add_edge( root, v, g );
  _test[v] = detail::make_test_case( detail::call( f ) );

  return v;
}

test_suite::test_case_type test_suite::add( test_suite::func_type f, test_suite::test_case_type depends )
{
  vertex_t v = add_vertex( white_color, g);
  add_edge( depends, v, g );
  _test[v] = detail::make_test_case( detail::call( f ) );

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

} // namespace exam
