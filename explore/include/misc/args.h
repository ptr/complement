// -*- C++ -*- Time-stamp: <98/03/21 18:03:40 ptr>
#ifndef __args_h
#define __args_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifndef __STRING__
#include <string>
#endif

#ifndef __SGI_STL_LIST
#include <list>
#endif

#ifndef __SGI_STL_ALGO_H
#include <algo.h>
#endif

class ArgsParser
{
  public:
    ArgsParser( int argc, char * const *argv );

    bool is_option( const string& op )
      { return find( arg.begin(), arg.end(), op ) != arg.end(); }

    string get_next( const string& op );

  protected:
    list<string> arg;
};

#endif // __args_h
