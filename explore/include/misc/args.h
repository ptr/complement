// -*- C++ -*- Time-stamp: <99/04/30 19:19:43 ptr>
#ifndef __args_h
#define __args_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <string>
#include <list>
#include <algorithm>


#include <stl_config.h>

#ifdef __STL_USE_NEW_STYLE_HEADERS
#include <istream>
#ifdef __STL_USE_NAMESPACES
using __STD::istream;
#endif
#else
#include <iostream.h>
#endif

#ifdef __STL_USE_NAMESPACES
using __STD::string;
#endif

class ArgsParser
{
#ifdef __STL_LIMITED_DEFAULT_TEMPLATES
    typedef __STD::list<string,__STL_DEFAULT_ALLOCATOR(string) > container_type;
#else 
    typedef __STD::list<string> container_type;
#endif
  public:
    typedef container_type::iterator iterator;

    ArgsParser( int argc, char * const *argv );
    ArgsParser( const string& in );
    ArgsParser( istream& in );

    bool is_option( const string& op )
      { return __STD::find( arg.begin(), arg.end(), op ) != arg.end(); }
    bool is_option_X( const string& op );

    string get_next( const string& op );
    string get( int n );

    unsigned size() const
      { return arg.size(); }

  protected:
    container_type arg;
};

class IniParser
{
  public:
    IniParser( istream& );
    string value( const string& key, const string& def );

  protected:
    struct par
    {
	par() 
	  { }
	par( const string& n, const string& v ) :
	    name( n ),
	    value( v )
	  { }

	string name;
	string value;

	operator ==( const par& p ) const
	  { return name == p.name; }
	operator !=( const par& p ) const
	  { return name != p.name; }
    };

#ifdef __STL_LIMITED_DEFAULT_TEMPLATES
    typedef __STD::list<par,__STL_DEFAULT_ALLOCATOR(par) > container_type;
#else 
    typedef __STD::list<par> container_type;
#endif

    container_type pars;
};

#endif // __args_h
