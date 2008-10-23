// -*- C++ -*- Time-stamp: <03/09/29 15:21:11 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: stategraph.h,v 1.2 2003/10/06 07:56:07 ptr Exp $"
#  else
#ident "@(#)$Id: stategraph.h,v 1.2 2003/10/06 07:56:07 ptr Exp $"
#  endif
#endif

#include <iostream>
#include <sstream>
#include <map>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

//using namespace boost;
//using namespace std;

enum edge_operation_t { edge_operation };

namespace boost {
  BOOST_INSTALL_PROPERTY( edge, operation );
} // namespace boost


//typedef property<edge_operation_t,int> Operation;
//typedef property<edge_color_t,int,Operation> Color;


class StateGraph
{
  private:
    typedef boost::property<edge_operation_t,int> Operation;
    typedef boost::property<boost::edge_color_t,int,Operation> Color;
    typedef boost::property<boost::edge_weight_t,int,Color> EdgeProperty;

    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::no_property, EdgeProperty > graph_t;
    typedef boost::graph_traits<graph_t>::out_edge_iterator out_edge_iterator_t;
    typedef boost::graph_traits<graph_t>::vertex_iterator vertex_iterator_t;

    typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;

    typedef boost::property_map<graph_t,boost::edge_color_t>::type edge_color_map_t;
    typedef boost::property_map<graph_t,boost::edge_weight_t>::type edge_weight_map_t;
    typedef boost::property_map<graph_t,edge_operation_t>::type edge_operation_map_t;

    typedef std::vector<vertex_t> predecessor_map_t;
   
  public:
    StateGraph( int, std::ostream& );

    void edge( int v_from, int v_to, int weight, int operation );
    std::stringstream& operator [] ( int key )
      {
        std::stringstream *_s =  _cmd[key];
        if ( _s == 0 ) {
          _s = new std::stringstream();
          _cmd[key] = _s;
        }
        return *_s;
      }

    void girdle( int start );

  private:
    
    graph_t g;
    edge_weight_map_t weight;
    edge_color_map_t color;
    edge_operation_map_t operation;

    void shortest_path( vertex_t, vertex_t );
    vertex_t walk_from( vertex_t v );
    vertex_t vertex_with_white_edge();
    int least_cost_edge_aux( const vertex_t&, const vertex_t& );

    std::map<int,std::stringstream *> _cmd;

    std::ostream& out;
};

