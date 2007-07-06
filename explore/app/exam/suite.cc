// -*- C++ -*- Time-stamp: <07/07/06 09:55:44 ptr>

#include "suite.h"
#include <boost/graph/breadth_first_search.hpp>
#include <stack>

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

void test_suite::girdle( int start )
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

} // namespace exam
