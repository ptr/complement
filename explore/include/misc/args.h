// -*- C++ -*- Time-stamp: <00/05/23 17:12:31 ptr>
/*
 *
 * Copyright (c) 1997-1998
 * Petr Ovchenkov
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

#ifndef __args_h
#define __args_h

#ident "$SunId$"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <list>
#include <algorithm>
#include <istream>

class ArgsParser
{
    typedef __STD::list<__STD::string> container_type;
  public:
    typedef container_type::iterator iterator;

    ArgsParser( int argc, char * const *argv );
    ArgsParser( const __STD::string& in );
    ArgsParser( __STD::istream& in );

    bool is_option( const __STD::string& op )
      { return __STD::find( arg.begin(), arg.end(), op ) != arg.end(); }
    bool is_option_X( const __STD::string& op );

    __STD::string get_next( const __STD::string& op );
    __STD::string get( int n );

    unsigned size() const
      { return arg.size(); }

  protected:
    container_type arg;
};

class IniParser
{
  public:
    IniParser( __STD::istream& );
    __STD::string value( const __STD::string& key, const __STD::string& def );

  protected:
    struct par
    {
	par() 
	  { }
	par( const __STD::string& n, const __STD::string& v ) :
	    name( n ),
	    value( v )
	  { }

	__STD::string name;
	__STD::string value;

	bool operator ==( const par& p ) const
	  { return name == p.name; }
	bool operator !=( const par& p ) const
	  { return name != p.name; }
    };

    typedef __STD::list<par> container_type;

    container_type pars;
};

#endif // __args_h
