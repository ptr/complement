// -*- C++ -*- Time-stamp: <04/02/18 19:42:59 ptr>

// #include <algorithm>
// #include <string>


#include <iostream>
//#include <stl/_prolog.h>

// #include <stl/_iosfwd.h>
// #include <stl/char_traits.h>
// #include "/export/home/ptr/STLport.lab/STLport/stlport/cstddef"
//#include "/export/home/ptr/STLport.lab/STLport/stlport/using/cstring"

//#include "/export/home/ptr/STLport.lab/STLport/stlport/stl/_construct.h"
//#include "/export/home/ptr/STLport.lab/STLport/stlport/stl/_iterator_base.h"
// #include "/export/home/ptr/STLport.lab/STLport/stlport/stl/_move_construct_fwk.h"

//#include <cstring>

//#include <stl/_istream.h>
//#include <stl/_ios.h>
//#include <stdexcept>
//#include <stl/_prolog.h>
//#include <exception>
//#include <cstring>

//#include <stl/_alloc.h>
// . #include <stl/_threads.h>
//#include ""
//#include <pthread.h>
// . #include <stl/_epilog.h>

// #include <stl/_config.h>

// using namespace std;

//#include <ctype.h>

//inline char tolower_char(char a)
//{
//	return (char) tolower(a);
//}

namespace test {
  void a()
  { }
}

namespace other {
  using namespace test;
}

namespace std {
  using namespace test;
}

namespace test {
  void b() {}
}

int main( int argc, char * const *argv )
{
//  std::string s( "Hello, world!" );
  std::a(); // <<-- hang compiler when used with STLport's iostream
//  std::b();
  test::a();
  // a();

#if 0
  cerr << "Hello!" << endl;
  std::cerr << "Hello!";
  _STL::cerr << "Hello!";
//  cerr << s << endl;
//  std::transform( s.begin(), s.end(), s.begin(), tolower_char );
//  cerr << s << endl;
//  cerr << tolower_char('H') << endl;
#endif

  return 0;
}
