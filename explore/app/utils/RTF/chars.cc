// -*- C++ -*- Time-stamp: <02/08/21 10:07:22 ptr>

/*
 *
 * Copyright (c) 2002
 * Petr Ovtchenkov
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

// #include <iostream>
// #include <sstream>

using namespace std;

void parse();

int main( int, char * const * )
{
#if 0
  cout.unsetf( ios_base::skipws );

  string s;
  string::size_type p;

  while ( cin.good() ) {
    getline( cin, s );

    // naive
    
    p = s.find( "\\'" );
    
    while ( p != string::npos ) {
      if ( p == 0 || s[p-1] != '\\' ) {
        strstream i;
        unsigned c;
        i << s.substr( p + 2, 2 );
        i >> hex >> c;
        s.erase( p, 4 );
        if ( c == 0x93 ) {
          s.insert( p, "<<" );
          p += 2;
        } else if ( c == 0x94 ) {
          s.insert( p, ">>" );
          p += 2;
        } else if ( c == 0x85 ) {
          s.insert( p, "-" );
          p += 1;
        } else {
          s.insert( p, 1, (char)c );
          p += 1;
        }
      } else {
        p += 2;
      }      
      p = s.find( "\\'", p );
    }

    cout << s << endl;
  }
#else
  parse();
#endif // 0
  return 0;
}
