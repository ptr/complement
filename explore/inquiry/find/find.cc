#include <string>
#include <map>
#include <algorithm>
#include <functional>

#include <iostream>

using namespace std;

int main()
{
  typedef multimap<int,string> container_type;

  container_type m;

  m.insert( make_pair(1,"one 1") );
  m.insert( make_pair(1,"one 2") );
  m.insert( make_pair(1,"one 3") );

  typedef container_type::const_iterator const_iterator;

  typedef pair<const_iterator,const_iterator> range_type;

  range_type r = m.equal_range( 1 );

  // bind2nd( equal_to<string>(), string("one 2") );

  const_iterator j = find_if( r.first, r.second,
                              compose1(
                                bind2nd( equal_to<string>(), string("one 2") ),
                                select2nd<container_type::value_type>() )
    );

  if ( j != m.end() ) {
    cerr << j->second << endl;
  } else {
    cerr << "Not found" << endl;
  }

  return 0;
}
