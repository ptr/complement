// -*- C++ -*- Time-stamp: <01/03/19 16:31:44 ptr>

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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

#ifndef __config_feature_h
#include <config/feature.h>
#endif

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
