// -*- C++ -*- Time-stamp: <99/08/18 20:20:03 ptr>
#ifndef __args_h
#define __args_h

#ident "$SunId$ %Q%"

#include <string>
#include <list>
#include <algorithm>
#include <istream>

class ArgsParser
{
    typedef std::list<std::string> container_type;
  public:
    typedef container_type::iterator iterator;

    ArgsParser( int argc, char * const *argv );
    ArgsParser( const std::string& in );
    ArgsParser( std::istream& in );

    bool is_option( const std::string& op )
      { return std::find( arg.begin(), arg.end(), op ) != arg.end(); }
    bool is_option_X( const std::string& op );

    std::string get_next( const std::string& op );
    std::string get( int n );

    unsigned size() const
      { return arg.size(); }

  protected:
    container_type arg;
};

class IniParser
{
  public:
    IniParser( std::istream& );
    std::string value( const std::string& key, const std::string& def );

  protected:
    struct par
    {
	par() 
	  { }
	par( const std::string& n, const std::string& v ) :
	    name( n ),
	    value( v )
	  { }

	std::string name;
	std::string value;

	bool operator ==( const par& p ) const
	  { return name == p.name; }
	bool operator !=( const par& p ) const
	  { return name != p.name; }
    };

    typedef std::list<par> container_type;

    container_type pars;
};

#endif // __args_h
