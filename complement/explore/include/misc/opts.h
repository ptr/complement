// -*- C++ -*- Time-stamp: <08/05/21 12:17:39 yeti>

#ifndef __OPTS_H__
#define __OPTS_H__

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <typeinfo>
#include <cctype>
#include <exception>
#include <stdexcept>

const char* OPT_DEFAULT_DESCRIPTION = "(no description)";
const char* OPT_DEFAULT_LONGNAME = "(no longname)";

class Opt
{
  public:
    Opt() { cnt = 0 , shortname = ' '; }
    char shortname;
    std::string longname;
    std::string desc;
    std::string default_v;
    int token;

    std::vector< std::string > args;
    std::vector< int > pos;

    bool has_arg;
    bool is_set;
    int cnt; // number of times this option was encounterd in command line
    bool operator==( const std::string& _longname ) const { return longname == _longname; }
    bool operator==( char _shortname ) const              { return shortname == _shortname; }
    bool operator==( int _token ) const                   { return token == _token; } 
};

class Opts
{
  private:
    typedef std::vector< Opt > options_container_type;
    options_container_type storage;  
  public:
    Opts( const std::string& _brief = "", const std::string& _author = "", const std::string& _copyright = "") :
        brief(_brief),
        author(_author),
        copyright(_copyright) 
      { free_token = 0; }

    // adding option / flag (option that doesn't need arguments)
    template <class T>
    int add( const std::string& _longname,T _default_value,const std::string& _desc = OPT_DEFAULT_DESCRIPTION );

    template <class T>
    int add( char _shortname,T _default_value,const std::string& _longname = OPT_DEFAULT_LONGNAME, const std::string& _desc = OPT_DEFAULT_DESCRIPTION );

    int addflag( const std::string& _longname,const std::string& desc = OPT_DEFAULT_DESCRIPTION);

    int addflag( char _shortname,const std::string& _longname = OPT_DEFAULT_LONGNAME,const std::string& _desc = OPT_DEFAULT_DESCRIPTION );
 
    // getting option
  
    template < class T ,class V >
    T get( V field );

    template <class T , class V>
    T get_default( V field );

    template < class V , class BackInsertIterator>
    void getemall( V field , BackInsertIterator bi);


    template < class T >
    bool is_set( T field ) const;

    template < class T >
    int get_cnt( T field ) const;

    template < class V , class BackInsertIterator >
    void get_pos( V field , BackInsertIterator bi);

    // parse
    void parse(int& ac, const char** av);

    // stuff
    void help(std::ostream& out);
    std::string get_pname() const;

    // error handling
    struct unknown_option :
        public std::invalid_argument
    {
        unknown_option( const std::string& _optname ) :
            std::invalid_argument( std::string("unknown option ").append(_optname) )
          { }
    };
  
    struct missing_arg :
        public std::invalid_argument
    {
        missing_arg( const std::string& _optname ) :
            std::invalid_argument( std::string("missing argument for option ").append(_optname) )
          { }
    };

    struct arg_typemismatch :
        public std::invalid_argument
    {
        arg_typemismatch( const std::string& _optname, const std::string& _argname) :
            std::invalid_argument(std::string("argument [").append(_argname).append("] doesn't match by type for option ").append(_optname))
          { }
    };
    
    int free_token;
  private:
    std::string pname;
    std::string brief;
    std::string author;
    std::string copyright;

    bool isterm( const std::string& s );
    bool is_opt_name( const std::string& s );
    bool is_flag_group( const std::string& s );
    bool is_substr(const std::string& small, const std::string& big );
    options_container_type::iterator get_opt_index( std::string s );
};

template <class T>
int Opts::add(char _shortname,T _default_value,const std::string& _longname,const std::string& _desc)
{
  addflag(_shortname,_longname,_desc);
  std::stringstream ss;
  ss << _default_value;
  storage[storage.size() - 1].default_v = ss.str();
  storage[storage.size() - 1].has_arg = true;
  return storage[storage.size() - 1].token;
}

template <class T>
int Opts::add(const std::string& _longname,T _default_value,const std::string& _desc)
{
  addflag(_longname,_desc);
  std::stringstream ss;
  ss << _default_value;
  storage[storage.size() - 1].default_v = ss.str();
  storage[storage.size() - 1].has_arg = true;
  return storage[storage.size() - 1].token;
}

template <class T>
bool Opts::is_set(T field) const
{
  options_container_type::const_iterator i;
  for (i = storage.begin();i != storage.end();++i)
    if (*i == field)
      return i->is_set;
  return false;
}

template <class T>
int Opts::get_cnt(T field) const
{
  options_container_type::const_iterator i;
  for (i = storage.begin();i != storage.end();++i)
    if (*i == field)
      return i->cnt;
  return 0;
}

template < class T , class V >
T Opts::get( V field )
{
  options_container_type::const_iterator i;
  T res;
  for ( i = storage.begin(); i != storage.end(); ++i )
  {
    if ( *i == field )
    {
      if ( !i->has_arg )
      {
        throw std::logic_error("using Opts::get for option without arguments");
      }
    
      std::stringstream ss;
      ss << (i->args.empty() ? i->default_v : i->args[0]);
      ss >> res;

      if (ss.fail())
      {
        std::stringstream ss1;
        ss1 << field;
        throw arg_typemismatch(ss1.str(),i->args[0]);
      }
  
      break;
    }
  }

  if ( i == storage.end() )
  {
    std::stringstream ss1;
    ss1 << field;
    throw unknown_option( ss1.str() );
  }

  return res;
}


template <class T , class V>
T Opts::get_default( V field )
{
  options_container_type::const_iterator i;
  T res;
  for (i = storage.begin();i != storage.end();++i)
    if (*i == field)
    {
      if (!i->has_arg)
        throw std::logic_error("using Opts::get for option without arguments");
    
      std::stringstream ss;
      ss << i->default_v;

      ss >> res;

      if (ss.fail())
      {
        std::stringstream ss1;
        ss1 << field;
        throw arg_typemismatch(ss1.str(),i->default_v);
      }
  
      break;
    }

  if (i == storage.end())
  {
    std::stringstream ss1;
    ss1 << field;
    throw unknown_option( ss1.str() );
  }
  return res;
}

template <class V , class BackInsertIterator>
void Opts::getemall( V field , BackInsertIterator bi)
{
  options_container_type::const_iterator i;
  for (i = storage.begin();i != storage.end();++i)
    if (*i == field)
    {
      if (!i->has_arg)
        throw std::logic_error("using Opts::getemall for option without arguments");
    
      if (!i->args.empty())
        for (int j = 0;j < i->args.size();j++)
        {

          std::stringstream ss(i->args[j]);
          ss >> *bi++;
          
          if (ss.fail())
          {
            std::stringstream ss1;
            ss1 << field;
            throw arg_typemismatch(ss1.str(),i->args[j]);
          }
        }
     
      break;
    }

  if (i == storage.end())
  {
    std::stringstream ss1;
    ss1 << field;
    throw unknown_option(ss1.str());
  }
}

template <class V , class BackInsertIterator>
void Opts::get_pos( V field , BackInsertIterator bi)
{
  options_container_type::const_iterator i;
  for (i = storage.begin();i != storage.end();++i)
    if (*i == field)
    {
      if (i->is_set)
        for (int j = 0;j < i->pos.size();j++)
        {
          *bi++ = i->pos[j];
        }
     
      break;
    }

  if (i == storage.end())
  {
    std::stringstream ss1;
    ss1 << field;
    throw unknown_option(ss1.str());
  }
}



#endif
