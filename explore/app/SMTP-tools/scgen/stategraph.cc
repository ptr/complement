// -*- C++ -*- Time-stamp: <03/09/29 15:22:19 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: stategraph.cc,v 1.2 2003/10/06 07:56:07 ptr Exp $"
#  else
#ident "@(#)$Id: stategraph.cc,v 1.2 2003/10/06 07:56:07 ptr Exp $"
#  endif
#endif

#include "stategraph.h"

using namespace boost;
using namespace std;

StateGraph::StateGraph( int nv, std::ostream& _out ) :
    g( nv ),
    out( _out )
{
  weight = get( edge_weight, g );
  color = get( edge_color, g );
  operation = get( edge_operation, g );
}

void StateGraph::edge( int v_from, int v_to, int weight, int operation )
{
  // the '0' parameter in Color(... is definition of initial color '0' for edge
  add_edge( v_from, v_to, EdgeProperty(weight,Color(0,operation)), g );
}

StateGraph::vertex_t StateGraph::walk_from( StateGraph::vertex_t v )
{
  out_edge_iterator_t ei;
  out_edge_iterator_t eend;

  for ( tie(ei,eend) = out_edges(v,g); ei != eend; ++ei ) {
    if ( color[*ei] == 0 ) {
      color[*ei] = 1;
      
      out << (*this)[operation[*ei]].str();
      return target(*ei, g);
    }
  }

  return vertex_t(-1);
}

StateGraph::vertex_t StateGraph::vertex_with_white_edge()
{
  vertex_iterator_t vi;
  vertex_iterator_t vie;
  for ( tie(vi,vie) = vertices(g); vi != vie; ++vi ) {
    out_edge_iterator_t ei;
    out_edge_iterator_t eend;

    for ( tie(ei,eend) = out_edges(*vi,g); ei != eend; ++ei ) {
      if ( color[*ei] == 0 ) {
        return *vi;
      }
    }
  }

  return vertex_t(-1);
}

int StateGraph::least_cost_edge_aux( const vertex_t& x, const vertex_t& u )
{
  out_edge_iterator_t ei;
  out_edge_iterator_t eend;
  out_edge_iterator_t least_cost;

  tie(ei,eend) = out_edges(x,g);
  least_cost = eend;
  for ( ; ei != eend; ++ei ) {
    if ( target(*ei, g) == u ) {
      if ( (least_cost == eend) || (weight[*least_cost] > weight[*ei]) ) {
        least_cost = ei;
      }
    }
  }
  return operation[*least_cost];
}

void StateGraph::shortest_path( StateGraph::vertex_t v, StateGraph::vertex_t u )
{
  vector<vertex_t> p(num_vertices(g));
  vector<int> ops;

  dijkstra_shortest_paths( g, v, predecessor_map(&p[0]) );

  vertex_t x = p[u];
  while ( x != v ) {
    ops.push_back( least_cost_edge_aux(x,u) );
    u = x;
    x = p[u];
  }
  ops.push_back( least_cost_edge_aux(x,u) ); // last step

  for ( vector<int>::reverse_iterator i = ops.rbegin(); i != ops.rend(); ++i ) {
    out << (*this)[*i].str();
  }
}

void StateGraph::girdle( int start )
{
  vertex_t v = walk_from( start );
  vertex_t u = v;

  do {
    while ( v != vertex_t(-1) ) {
      u = v;
      v = walk_from( u );
    }
    v = vertex_with_white_edge();
    if ( v == vertex_t(-1) ) {
      break;
    }
    shortest_path( u, v );
  } while ( true );
  if ( u != start ) {
    shortest_path( u, start );
  }
}
