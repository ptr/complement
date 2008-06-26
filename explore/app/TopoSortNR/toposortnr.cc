// -*- C++ -*- Time-stamp: <02/02/26 19:03:52 ptr>

/*
 * $Id$
 *
 */

#include <vector>
#include <list>
#include <utils>
#include <iostream>

using namespace std;

typedef int vertex_index_type;

struct vertex
{
    vertex() :
        out_count( 0 )
      { }

    typedef list<vertex_index_type> vertex_index_container;

    vertex_index_container in;
    vertex_index_type out_count;
    vertex_index_type index;
};

class vertex_container
{
  public:
    typedef int size_type;

    vertex_container( const size_type );
    void add_edge( vertex_index_type, vertex_index_type );

    bool sequence( /* vector<vertex_index_type>& */ );

  private:
    typedef vertex vertex_type;
    typedef vector<vertex_type> vertex_container_type;
//    typedef vector<vertex_index_type> vi_container_type;

    vertex_container_type V;
//    vi_container_type p;
};

vertex_container::vertex_container( const size_type n )
  V( n )
{
  vertex_index_type k = 0;
  for ( vertex_container_type::iterator i = V.begin(); i != V.end(); ++i, ++k ) {
    (*i).index = k;
  }
}

void vertex_container::add_edge( vertex_index_type v, vertex_index_type u )
{
  V[v].out_count++;
  V[u].in.push_pack( v );
}

bool vertex_container::sequence( /* vector<vertex_index_type>& r */ )
{
//  r.clear();
//  r.reserve( V.size() );

  bool rmflag;
  do {
    rmflag = false;
    vertex_container_type::iterator::distance_type d = 0;
    for ( vertex_container_type::iterator i = V.begin(); i != V.end(); ) {
      if ( (*i).out_count == 0 ) {
        for ( vertex::vertex_index_container::iterator j = (*i).in.begin();
              j != (*i).in.end(); ++j ) {
          --V[*j].out_count;
          // recursive call may be here
        }
        // vertex_container_type::iterator::distance_type d = i - V.begin();
        cout << /* d */ (*i).index << ", "; // or (*i).index, if erase used below
        V.erase( i ); //
        rmflag = true;
        i = V.begin() + d;
      } else {
        ++i;
        ++d;
      }
    }
  } while ( rmflag );

  if ( V.size() > 0 ) {
    return false;
  }

  return true;
}

#if 0
typedef int vertex_type;
typedef pair<vertex_type,vertex_type> edge_type;
typedef list<edge_type> edge_container_type;
// typedef list<edge_type> edge_container_type;
#endif

int main( int argc, char * const *argv )
{
  int ex = 0;

  const int N = 10;

#if 0
  edge_container_type edge;
  vector<vertex_index_type> p( N, 0 ); // array p <- 0


  edge.push_back( edge_type( 1, 2 ) );
  edge.push_back( edge_type( 2, 3 ) );

  for ( edge_container_type::iterator i = edge.begin(); i != edge.end(); ++i ) {
    ++p[(*i).first];
  }

  bool rmflag;
  do {
    rmflag = false;
    for ( vertex_index_type v = 0; v < N; ++v ) {
      if ( p[v] == 0 ) {
        cout << v << ",";
        p[v] = -1;
        for ( edge_container_type::iterator i = edge.begin(); i != edge.end(); ++i ) {
          if ( (*i).second == v ) {
            --p[(*i).first];
            edge.erase( i );
          }
        }
        rmflag = true;
      }
    }
  } while ( rmflag );

  for ( vertex_index_type v = 0; v < N; ++v ) {
    if ( p[v] != -1 ) {
      return -1; // fail
    }
  }
#endif

  vertex_container v( N );

  v.add_edge( 1, 2 );
  v.add_edge( 2, 3 );
  

  v.sequence();
  
  return ex;
}
