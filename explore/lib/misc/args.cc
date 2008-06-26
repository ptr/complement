// -*- C++ -*- Time-stamp: <04/05/21 17:47:51 ptr>

/*
 *
 * Copyright (c) 1997-1998, 2001, 2006
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

#include <config/feature.h>
#include <ostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <functional>
#include "misc/args.h"

using namespace std;

ArgsParser::ArgsParser( int argc, char * const *argv )
{
  int i = 1;

  while ( i < argc ) {
    arg.push_back( argv[i] );
    ++i;
  }
}

ArgsParser::ArgsParser( const string& in )
{
  stringstream s( in );
  string tmp;
  
  while ( s.good() ) {
    s >> tmp;
    if ( tmp.length() != 0 && tmp[0] == '"' ) {
//      s.unsetf( ios_base::skipws );
      getline( s, tmp, '"' );
    }
    arg.push_back( tmp );
  }
}

ArgsParser::ArgsParser( istream& in )
{
  string tmp;
  
  while ( in.good() ) {
    in >> tmp;
    if ( tmp.length() != 0 && tmp[0] == '"' ) {
//      s.unsetf( ios_base::skipws );
      getline( in, tmp, '"' );
    }
    arg.push_back( tmp );
  }
}

ArgsParser::ArgsParser()
{
}

void ArgsParser::parse( int argc, char * const *argv )
{
  _pname = argv[0];

  int i = 1;

  while ( i < argc ) {
    arg.push_back( argv[i] );
    ++i;
  }
}

void ArgsParser::copyright( const char *s )
{
  _copyright = s;
}

void ArgsParser::brief( const char *s )
{
  _announce = s;
}

void ArgsParser::print_help( ostream& s )
{
  s << "This is " << _pname << ", " << _announce << "\n"
    << _copyright << "\n\n"
    << "Usage: \n";
}

string ArgsParser::get_next( const string& op )
{
  container_type::iterator i = find( arg.begin(), arg.end(), op );

  if ( i != arg.end() ) {
    ++i;
    if ( i != arg.end() ) {
      return *i;
    }
  }

  return "";
}

string ArgsParser::get( int n )
{
  iterator i;
  if ( n >= 0 ) {
    i = arg.begin();
    int j = 0;
    
    while ( i != arg.end() && j < n ) {
      ++i;
      ++j;
    }
  } else {
    i = --arg.end();
    int j = -1;
    
    while ( i != arg.end() && j > n ) {
      --i;
      --j;
    }
  }

  return i != arg.end() ? (*i) : string("");
}

bool ArgsParser::is_option_X( const string& op )
{
  container_type::iterator i = arg.begin();
  while ( i != arg.end() ) {
    if ( (*i).length() > 0 && (*i)[0] == '-' &&
         (*i).find( op ) != string::npos ) {
      return true;
    }
    ++i;
  }
  return false;
}

Option<std::string>::Option( const char *n, const std::string& v, const char *h )
{
  _nm = n;
  _v = v;
  if ( h != 0 ) {
    _hlp = h;
  }
}

Option<char *>::Option( const char *n, const char *v, const char *h )
{
  _nm = n;
  _v = v;
  if ( h != 0 ) {
    _hlp = h;
  }
}

void Argv::parse( int argc, char * const *argv )
{
  _pname = argv[0];

  int i = 1;
  opt_container_type::iterator a;
  string argvI;
  string argvII;
  string::size_type p;

  while ( i < argc ) {
    if ( _stop_opt == argv[i] || (strlen( argv[i] ) > 0 && argv[i][0] != '-') ) {
      // ++i;
      break;
    }

    argvI = argv[i];
    
    if ( (p = argvI.find( '=' ) ) != string::npos ) {
      argvII = argvI.substr( p + 1 );
      argvI.erase( p );
    } else {
      argvII.clear();
    }

    // find '=' in argvI
    // strip after '='
    // after '=' is a value; search in options the string before '='
    a = std::find_if( opt.begin(), opt.end(), bind2nd( eqs, /* argv[i] */ argvI ) );
    if ( a == opt.end() ) {
      string err( "unknown option: " );
      err += argv[i];
      throw std::invalid_argument( err );
    }
    if ( (*a)->type() != typeid(bool) ) {
//      if ( ++i < argc ) {
        (*a)->read( argvII );
        (*a)->is_set = true;
//      } else {
//        string err( "option " );
//        err += argv[i-1];
//        err += " require value";
//        throw std::invalid_argument( err );
//      }
    } else {
      (*a)->read( "1" );
      (*a)->is_set = true;
    }
    ++i;
  }
  while ( i < argc ) {
    arg.push_back( argv[i] );
    ++i;
  }
}

const std::string& Argv::operator[]( int i ) const
{
  if ( i < 0 || i >= arg.size() ) {
    throw std::range_error( "out of range" );
  }
  return arg[i];
}

void Argv::stop_option( const char *s )
{
  _stop_opt = s;
}

void Argv::copyright( const char *s )
{
  _copyright = s;
}

void Argv::brief( const char *s )
{
  _announce = s;
}

void Argv::args( const char *s )
{
  _args = s;
}

void Argv::print_help( ostream& s )
{
  s << "This is " << _pname << ", " << _announce << "\n\n"
    <<  _copyright << "\n\n"
    << "Usage:\n" << "  " << _pname << "\n\n"
    << "Options:\n";
  string tmp;
  for ( opt_container_type::iterator i = opt.begin(); i != opt.end(); ++i ) {
    tmp = (*i)->_nm;
    if ( (*i)->type() == typeid(std::string) ) {
      tmp += "=<string>";
    } else if ( (*i)->type() == typeid(int) ) {
      tmp += "=<int>";
    } else if ( (*i)->type() == typeid(unsigned) ) {
      tmp += "=<uint>";
    }
    s << "  " << setw(24) << setfill( ' ' ) << setiosflags( ios_base::left )
      << tmp << (*i)->_hlp << "\n";
  }
  s << "\n";
}

bool Argv::is( const char *nm ) const
{
  return (*find( nm ))->is_set;
}

Argv::opt_container_type::iterator Argv::find( const char *nm )
{
  opt_container_type::iterator i = std::find_if( opt.begin(), opt.end(), bind2nd( eq, nm ) );
  if ( i == opt.end() ) {
    std::string err( "unknown option " );
    err += nm;
    throw std::invalid_argument( err );
  }
  return i;
}

Argv::opt_container_type::const_iterator Argv::find( const char *nm ) const
{
  opt_container_type::const_iterator i = std::find_if( opt.begin(), opt.end(), bind2nd( eq, nm ) );
  if ( i == opt.end() ) {
    std::string err( "unknown option " );
    err += nm;
    throw std::invalid_argument( err );
  }
  return i;
}

IniParser::IniParser( istream& cfg )
{
  string tmp;
  string::size_type p;

  while ( cfg.good() ) {
    tmp.clear();
    getline( cfg, tmp );
    p = tmp.find( '#' );
    if ( p != string::npos ) {
      tmp.erase( p );
    }
    if ( tmp.length() > 0 ) {
      p = tmp.find( ':' );
      if ( p != string::npos ) {
	string name = tmp.substr( 0, p );
	++p;
	string value = tmp.substr( p, tmp.size() - p );
	  
	p = name.find_first_not_of( " \t" );
	if ( p != string::npos ) {
	  if ( p > 0 ) {
	    name.erase( 0, p );
	  }
	  p = name.find_last_not_of( " \t" );
	  if ( p != string::npos ) {
	    ++p;
	    name.erase( p, name.length() - p );
	  }
	  p = value.find_first_not_of( " \t" );
	  if ( p != string::npos ) {
	    if ( p > 0 ) {
	      value.erase( 0, p );
	    }
	    p = value.find_last_not_of( " \t" );
	    if ( p != string::npos ) {
	      ++p;
	      value.erase( p, value.length() - p );
	    }
	    pars.push_back( par( name, value ) );
	  }
	}
      }
    }
  }
}

string IniParser::value( const string& key, const string& def )
{
  container_type::iterator i = find( pars.begin(), pars.end(), par( key, "" ) );

  return i != pars.end()? (*i).value : def;
}

#if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)

// #pragma weak __1cDstdMbasic_string4Ccn0ALchar_traits4Cc__n0AJallocator4Cc___Rfind_first_not_of6kMpkcII_I_
// #pragma weak unsigned std::string::find_first_not_of( const char *, unsigned, unsigned )

// void __Bug1()
// {
//   const char *_ch = 0;
//  std::basic_string<char,std::char_traits<char>,std::allocator<char> > _str;
//  _str.find( _ch, 0, 0 );
//  _str.find_first_not_of( (const char *)0, 0U, 0U );
// }
// unsigned _xxx = _str.find_first_not_of( (const char *)0, 0U, 0U );

// #pragma weak __Bug1

#endif
