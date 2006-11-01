// -*- C++ -*- Time-stamp: <04/07/01 12:29:26 ptr>

/*
 *
 * Copyright (c) 1997-1998, 2001
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
#include <vector>
#include <algorithm>
#include <istream>
#include <ostream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <typeinfo>
#include <sstream>

class Option_base
{
  public:
    Option_base() :
        is_set( false )
      { }

    virtual ~Option_base()
      { }

    virtual bool assign( int& ) const
      { throw std::domain_error( "wrong type" ); }
    virtual bool assign( long& ) const
      { throw std::domain_error( "wrong type" ); }
    virtual bool assign( short& ) const
      { throw std::domain_error( "wrong type" ); }
    virtual bool assign( std::string& ) const
      { throw std::domain_error( "wrong type" ); }
    virtual bool assign( unsigned& ) const
      { throw std::domain_error( "wrong type" ); }
    virtual bool assign( unsigned long& ) const
      { throw std::domain_error( "wrong type" ); }
    virtual bool assign( unsigned short& ) const
      { throw std::domain_error( "wrong type" ); }
    virtual bool assign( bool& ) const
      { throw std::domain_error( "wrong type" ); }

    virtual const std::type_info& type() const = 0;

    bool operator ==( const Option_base& s ) const
      { return _nm == s._nm; }
    bool operator ==( const std::string& s ) const
      { return _nm == s; }
    bool operator ==( const char *s ) const
      { return _nm == s; }

  protected:
    std::string _nm;
    std::string _hlp;
    bool is_set;

    virtual void read( const char *str ) = 0;
    virtual void read( const std::string& str ) = 0;
    friend class Argv;
};

template <class P, class V>
class deref_equal :
    public std::binary_function<P,V,bool>
{
  public:
    bool operator()(const P& __x, const V& __y) const
      { return *__x == __y; }
};

inline void Option_base_destroyer( Option_base *p )
{ delete p; }

template <class T>
class Option :
    public Option_base
{
  public:
    Option( const char *, const T&, const char * = 0 );
    virtual bool assign( T& v ) const
      {
        v = _v;
        return is_set;
      }
    virtual const std::type_info& type() const
      { return typeid( T ); }
  private:
    T _v;

    void read( const char *str )
      {
        std::istringstream s( str );
        s >> _v;
      }

    void read( const std::string& str )
      {
        std::istringstream s( str );
        s >> _v;
      }

    friend class Argv;
};

template <class T>
Option<T>::Option( const char *n, const T& v, const char *h )
{
  _nm = n;
  _v = v;
  if ( h != 0 ) {
    _hlp = h;
  }
}

template <>
class Option<std::string> :
    public Option_base
{
  public:
    Option( const char *, const std::string&, const char * = 0 );
    bool assign( std::string& v ) const
      {
        v = _v;
        return is_set;
      }

    virtual const std::type_info& type() const
      { return typeid( std::string ); }
  private:
    std::string _v;

    void read( const char *str )
      { _v = str; }

    void read( const std::string& str )
      { _v = str; }

    friend class Argv;
};

template <>
class Option<char *> :
    public Option_base
{
  public:
    Option( const char *, const char *, const char * = 0 );
    bool assign( std::string& v ) const
      {
        v = _v;
        return is_set;
      }

    virtual const std::type_info& type() const
      { return typeid( std::string ); }
  private:
    std::string _v;

    void read( const char *str )
      { _v = str; }

    void read( const std::string& str )
      { _v = str; }

    friend class Argv;
};

class Argv
{
  private:
    typedef std::list<Option_base *> opt_container_type;
    typedef std::vector<std::string> arg_container_type;
    deref_equal<Option_base *,char *> eq;
    deref_equal<Option_base *,std::string> eqs;

  public:
    Argv() :
        _stop_opt( "--" )
      { };
    ~Argv()
      { std::for_each( opt.begin(), opt.end(), Option_base_destroyer ); }

    template <class T>
    void option( const char *n, const T& v, const char *h )
      { opt.push_back( new Option<T>( n, v, h ) ); }
//    void option( const char *n, const char *v, const char *h = 0 )
//      { opt.push_back( new Option<std::string>( n, std::string(v), h ) ); }
    void parse( int argc, char * const *argv );
    template <class T>
    bool assign( const char *nm, T& v ) const
      { 
        try {
          return (*find( nm ))->assign( v );
        }
        catch ( std::domain_error& err ) {
          std::string _err( err.what() );
          _err += " for option ";
          _err += nm;
          throw std::domain_error( _err );
        }
      }
    bool is( const char *nm ) const;

    void print_help( std::ostream& s );
    void copyright( const char * );
    void brief( const char * );
    void args( const char * );
    void stop_option( const char * );
    const std::string& operator[]( int i ) const;

    arg_container_type::size_type size() const
      { return arg.size(); }

  protected:
    opt_container_type opt;
    arg_container_type arg;
    std::string _pname;
    std::string _copyright;
    std::string _announce;
    std::string _stop_opt;
    std::string _args;

  private:
    opt_container_type::const_iterator find( const char *nm ) const;
    opt_container_type::iterator find( const char *nm );
};

class ArgsParser
{
    typedef std::list<std::string> container_type;
  public:
    typedef container_type::iterator iterator;

    ArgsParser( int argc, char * const *argv );
    ArgsParser( const std::string& in );
    ArgsParser( std::istream& in );
    ArgsParser();

    bool is_option( const std::string& op )
      { return std::find( arg.begin(), arg.end(), op ) != arg.end(); }
    bool is_option_X( const std::string& op );

    std::string get_next( const std::string& op );
    std::string get( int n );

    unsigned size() const
      { return arg.size(); }

    void print_help( std::ostream& s );
    void copyright( const char * );
    void brief( const char * );
    void parse( int argc, char * const *argv );

  protected:
    container_type arg;
    std::string _pname;
    std::string _copyright;
    std::string _announce;
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
