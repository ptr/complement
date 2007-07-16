// -*- C++ -*- Time-stamp: <07/07/17 00:36:02 ptr>

#include <exam/suite.h>
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
        // boost::out_edges( v, g );
        // for () {
        _suite.check_test_case( boost::source( e, g ), boost::target( e, g ) );
        // }
      }

    test_suite& _suite;
};

template <class Tag>
skip_recorder<Tag> record_skip(test_suite& ts, Tag)
{ return skip_recorder<Tag>(ts); }

template <class Tag>
struct white_recorder :
        public base_visitor<white_recorder<Tag> >
{
    typedef Tag event_filter;

    white_recorder(test_suite& ts) :
        _suite(ts)
      { }

    template <class Vertex, class Graph>
    void operator()(Vertex v, const Graph& g)
      {
        // std::cerr << "On vertex " << v << std::endl;
        // boost::put( boost::vertex_color, g, v, white_color );
        _suite.clean_test_case_state( v );
      }

    test_suite& _suite;
};

template <class Tag>
white_recorder<Tag> record_white(test_suite& ts, Tag)
{ return white_recorder<Tag>(ts); }


} // namespace detail

int EXAM_IMPL(test_suite::_root_func)
{
  throw init_exception();

  return -1;
}

test_suite::test_suite( const string& name ) :
   root( add_vertex( white_color, g ) ),
   _suite_name( name ),
   local_logger( logger )
{
  testcase = get( vertex_testcase, g );
  _test[root].tc = detail::make_test_case( detail::call( _root_func ) );
  _test[root].state = 0;
}

test_suite::test_suite( const char *name ) :
   root( add_vertex( white_color, g ) ),
   _suite_name( name ),
   local_logger( logger )
{
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

int test_suite::girdle( test_suite::test_case_type start )
{
  stack<vertex_t> buffer;
  vertex_color_map_t color = get( vertex_color, g );

  // detail::white_recorder<on_initialize_vertex> vis( *this );
  // 
  // vertex_iterator_t i, i_end;

  // for ( tie(i, i_end) = vertices(g); i != i_end; ++i ) {
  //   // vis.initialize_vertex( *i, g );
  //   put( color, *i, white_color );
  // }

  _stat = base_logger::stat();
  local_logger->begin_ts();
  breadth_first_search( g, start, buffer,
                        make_bfs_visitor(
                          make_pair( record_white(*this,on_initialize_vertex()),
                            make_pair( record_vertexes(*this,on_discover_vertex()),
                                       record_skip(*this,on_examine_edge()) ) ) ),
                        color );
  local_logger->end_ts();
  local_logger->result( _stat, _suite_name );

  return _stat.failed;
}

test_suite::test_case_type test_suite::add( test_suite::func_type f, const string& name )
{
  vertex_t v = add_vertex( white_color, g);
  add_edge( root, v, g );
  _test[v].tc = detail::make_test_case( detail::call( f ) );
  _test[v].state = 0;
  _test[v].name = name;
  // ++_stat.total;

  return v;
}

test_suite::test_case_type test_suite::add( test_suite::func_type f, const string& name, test_suite::test_case_type depends )
{
  vertex_t v = add_vertex( white_color, g);
  add_edge( depends, v, g );
  _test[v].tc = detail::make_test_case( detail::call( f ) );
  _test[v].state = 0;
  _test[v].name = name;
  // ++_stat.total;

  return v;
}

int test_suite::flags()
{
  return local_logger->flags();
}

bool test_suite::is_trace()
{
  return local_logger->is_trace();
}

int test_suite::flags( int f )
{
  return local_logger->flags( f );
}

trivial_logger __trivial_logger_inst( cerr );

base_logger *test_suite::logger = &__trivial_logger_inst;

base_logger *test_suite::set_global_logger( base_logger *new_logger )
{
  base_logger *tmp = logger;
  logger = new_logger;
  if ( tmp == local_logger ) { // if local_logger was identical to logger, switch it too
    local_logger = logger;
  }
  return tmp;
}

base_logger *test_suite::set_logger( base_logger *new_logger )
{
  base_logger *tmp = local_logger;
  local_logger = new_logger;
  return tmp;
}

void test_suite::report( const char *file, int line, bool cnd, const char *expr )
{
  local_logger->report( file, line, cnd, expr );
}

void test_suite::run_test_case( test_suite::vertex_t v )
{
  try {
    ++_stat.total;
    if ( _test[v].state == 0 ) {
      if ( (*_test[v].tc)( this, 0 ) == 0 ) {
        ++_stat.passed;
        local_logger->tc( base_logger::pass, _test[v].name );
      } else {
        _test[v].state = fail;
        ++_stat.failed;
        local_logger->tc( base_logger::fail, _test[v].name );
      }
    } else {
      ++_stat.skipped;
      local_logger->tc( base_logger::skip, _test[v].name );
    }
  }
  catch ( init_exception& ) {
    --_stat.total;
  }
  catch ( ... ) {
    ++_stat.failed;
    _test[v].state = fail;
    local_logger->tc( base_logger::fail, _test[v].name );
  }
}

void test_suite::check_test_case( test_suite::vertex_t u, test_suite::vertex_t v )
{
  if ( _test[u].state != 0 ) {
    _test[v].state = skip;
  }
}

void test_suite::clean_test_case_state( vertex_t v )
{
  _test[v].state = 0;
}

int test_suite::run( test_suite *, int )
{
  return girdle( root );
}


} // namespace exam
