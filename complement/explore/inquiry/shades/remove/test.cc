#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>

using namespace std;

int main()
{
  vector<int> v;

  v.push_back( 1 );
  v.push_back( 2 );
  v.push_back( 3 );
  v.push_back( 4 );
  v.push_back( 1 );

  for ( vector<int>::iterator i = v.begin(); i != v.end(); ++i ) {
    cout << *i << ", ";
  }
  cout << endl;

  vector<int>::iterator j = remove( v.begin(), v.end(), 1 );

  for ( vector<int>::iterator i = v.begin(); i != v.end(); ++i ) {
    cout << *i << ", ";
  }
  cout << endl;

  cout << (j - v.begin()) << endl;

  v.clear();

  v.push_back( 1 );
  v.push_back( 2 );
  v.push_back( 3 );
  v.push_back( 4 );
  v.push_back( 1 );

  j = remove_copy( v.begin(), v.end(), v.begin(), 1 );
  for ( vector<int>::iterator i = v.begin(); i != v.end(); ++i ) {
    cout << *i << ", ";
  }
  cout << endl;

  v.clear();

  v.push_back( 1 );
  v.push_back( 2 );
  v.push_back( 3 );
  v.push_back( 4 );
  v.push_back( 1 );

  j = remove_if( v.begin(), v.end(), bind2nd( not_equal_to<int>(), 1 ) );

  for ( vector<int>::iterator i = v.begin(); i != v.end(); ++i ) {
    cout << *i << ", ";
  }
  cout << endl;


  return 0;
}
