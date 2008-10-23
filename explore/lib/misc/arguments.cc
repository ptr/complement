#include <sstream>
//#include <iostream>
//#include <algorithm>
#include <iterator>

#include <misc/arguments.h>

Arguments::Arguments( const string& s )
{
  istringstream is( s );
  parseArgs( is );
}
  
void Arguments::parseArgs( istream& is )
{
  _flags.clear();
  _args.clear();

  string str, data;

  getline( is, str );
  if( is.good() )
  {
    typedef istream_iterator<char,char> Iter;
		is.unsetf( ios_base::skipws );
    copy( Iter( is ), Iter(), back_inserter( data ) );
    _args.push_back( data );
  }

  istringstream ss( str );
  bool before = true;
  string cur;
  
  while( ss.good() )
  {
    ss >> cur;
    if( !cur.length() )
    {
      continue;
    }

    if( cur == "--" )
    {
      before == false;
      continue;
    }

    if( !before || cur[0] != '-' )
    {
      _args.push_back( cur );
      continue;
    }

    _flags += cur.substr( 1, cur.length() - 1 );
  }
//  remove_if( _args.begin(), _args.end(), not1( mem_fun_ref( &string::length ) ) );
}
