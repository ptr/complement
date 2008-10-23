
#include <iostream>
#include <boost/graph/adjacency_list.hpp>

using namespace boost;
using namespace std;

#if 0
int main( int, char * const * )
{
  typedef adjacency_list<vecS, vecS, directedS, no_property, property<edge_color_t,int> > graph_t;

  typedef pair<int,int> edge_t;

  enum {
    NotConnected,
    Connected,
    Greeting,
    Mail,
    Recipient,
    Data,
    EndOfData,
    NVertices
  };

  edge_t edge_array[] =
    {
      edge_t(NotConnected,Connected),
      edge_t(Connected,NotConnected)
    };

  const int n_edges = sizeof(edge_array) / sizeof(edge_t);

  graph_t g(edge_array, edge_array + n_edges, NVertices );

  typedef property_map<graph_t,vertex_index_t>::type index_map_t;
  index_map_t index = get( vertex_index, g );

  typedef graph_traits<graph_t>::vertex_iterator vertex_iterator_t;
  pair<vertex_iterator_t,vertex_iterator_t> vp;

  for ( vp = vertices(g); vp.first != vp.second; ++vp.first ) {
    cout << index[*vp.first] << " ";
  }
  cout << endl;

  cout << "Walk through graph\n";

  typedef graph_traits<graph_t>::vertex_descriptor vertex_t;

  vertex_t v = vertex( NotConnected, g );

  typedef graph_traits<graph_t>::out_edge_iterator out_edge_iterator_t;
  typedef property_map<graph_t,edge_color_t>::type edge_color_map_t;
  edge_color_map_t color = get( edge_color, g );
  
  out_edge_iterator_t ei;
  out_edge_iterator_t eend;

  for ( tie(ei,eend) = out_edges(v,g); ei != eend; ++ei ) {
    color[*ei] = 0;
  }

  for ( tie(ei,eend) = out_edges(v,g); ei != eend; ++ei ) {
    cout << color[*ei] << " ";
  }
  cout << endl;

  return 0;
}

#endif

typedef property<edge_color_t,int> Color;
typedef property<edge_weight_t,int> Weight;

// struct color_weight_t {
//     enum { num = 30000 };
//    typedef edge_property_tag kind;
// };

enum edge_cw_t { edge_cw };

namespace boost {

template <>
struct property_kind<edge_cw_t>
{
   typedef edge_property_tag type;
};
} // namespace boost

int main( int, char * const * )
{  
  // typedef property<edge_cw_t,Color,Weight> EdgeProperty;
  typedef property<edge_weight_t,int,Color> EdgeProperty;
  typedef adjacency_list<vecS, vecS, directedS, no_property, EdgeProperty > graph_t;

  enum {
    NotConnected,
    Connected,
    Greeting,
    Mail,
    Recipient,
    Data,
    EndOfData,
    NVertices
  };

  graph_t g( NVertices );

/* (valid)
  add_edge( vertex(NotConnected,g), vertex(Connected,g), EdgeProperty(0,1), g );
  add_edge( vertex(Connected,g), vertex(NotConnected,g), EdgeProperty(0,1), g );
*/
  // yet another:
  add_edge( NotConnected, Connected, EdgeProperty(1,0), g );
  add_edge( Connected, NotConnected, EdgeProperty(1,0), g );

  cout << "Walk through graph\n";

  typedef graph_traits<graph_t>::vertex_descriptor vertex_t;

  vertex_t v = vertex( NotConnected, g );

  typedef graph_traits<graph_t>::out_edge_iterator out_edge_iterator_t;
  typedef property_map<graph_t,edge_color_t>::type edge_color_map_t;
  typedef property_map<graph_t,edge_weight_t>::type edge_weight_map_t;
  // typedef property_map<graph_t,EdgeProperty>::type edge_property_map_t;
  edge_weight_map_t weight = get( edge_weight, g );
  edge_color_map_t color = get( edge_color, g );
  // edge_property_map_t color = get( edge_color, g );
  
  out_edge_iterator_t ei;
  out_edge_iterator_t eend;

//  for ( tie(ei,eend) = out_edges(v,g); ei != eend; ++ei ) {
    // color[*ei] = 0;
//    weight[*ei] = 0;
//  }

  for ( tie(ei,eend) = out_edges(v,g); ei != eend; ++ei ) {
    // cout << color[*ei] << " ";
    // cout << /* color[*ei] */ get(edge_cw, g, *ei ).m_value << ", " << weight[*ei] << " ";
    cout << color[*ei] << ", " << weight[*ei] << " ";
    // cout << weight[*ei] << " ";
  }
  cout << endl;


  return 0;
}
