#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <list>
#include <string>

using namespace std;

class Arguments
{
public:
  typedef list<string> container_type;
  Arguments( const string& s );
  const container_type& args()
  {
    return _args;
  }
  bool isFlag( char c )
  {
    return _flags.find( c ) != string::npos;
  }
private:
  string _flags;
  container_type _args;
  void parseArgs( istream& is );
};
#endif //ARGUMENTS_H
