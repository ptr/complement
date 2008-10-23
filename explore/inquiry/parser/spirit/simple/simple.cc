// -*- c++ -*- Time-stamp: <03/04/01 17:29:15 ptr>
// $Id$

#include <iostream>
#include <boost/spirit.hpp>

#include <string>

using namespace std;
using namespace boost::spirit;

// template <class T>
class print_actor
{
  public:
    explicit print_actor( ostream& o ) :
        _o( o )
      { }

    template <class V>
    void operator ()( const V& v ) const
      { _o << v; }

    template <class Iterator>
    void operator ()( const Iterator& s, const Iterator& e ) const
      {
        while( s != e ) {
          _o << *s++;
        }
      }

  private:
    std::ostream& _o;
};

void fch()
{
}

int main( int argc, char * const *argv )
{
  cout << "Hello" << endl;

  string s( "hi everybody!" );

  parse( s.c_str(),
         alpha_p[print_actor(cout)] >> *(alpha_p[print_actor(cout)]),
         space_p ).full;

  return 0;
}
