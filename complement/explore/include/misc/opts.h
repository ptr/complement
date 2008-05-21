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

class Opt
{
  public:
    Opt() { cnt = 0; }
    char shortname;
    std::string longname;
    std::string desc;
    std::vector< std::string > args;
    std::string default_v;

    bool has_arg;
    bool is_set;
    int cnt; // number of times this option was encounterd in command line
};

class Opts
{
  public:
    Opts( const std::string& _brief = "", const std::string& _author = "", const std::string& _copyright = "") :
        brief(_brief),
        author(_author),
        copyright(_copyright)
      { }

    // adding option / flag
    template <class T>
    void add( char _shortname,T _default_value,const std::string& _longname = "", const std::string& _desc = "" );
    void addflag( char _shortname,const std::string& _longname = "",const std::string& _desc = "" );

    // getting option
    template <class T>
    T get( char _shortname );

    template <class T>
    T get( const std::string& _longname );

    template <class T>
    T get_default( char _shorname );

    template <class T>
    T get_default( const std::string& _longname );

    template <class BackInsertIterator>
    void getemall(char _shortname,BackInsertIterator bi);

    template <class BackInsertIterator>
    void getemall(const std::string& _longname,BackInsertIterator bi);

    bool is_set( char _shortname );
    bool is_set( const std::string& _longname );

    int get_cnt( char _shortname ) const;
    int get_cnt( const std::string& _longname ) const;

    // parse
    void parse(int& ac, const char** av);

    // stuff
    void help(std::ostream& out);
    std::string get_pname() const;
    std::string get_brief() const;
    std::string get_author() const;
    std::string get_copyright() const;

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

    struct invalid_arg :
        public std::invalid_argument
    {
        invalid_arg( const std::string& _optname, const std::string& _argname) :
            std::invalid_argument(std::string("invalid argument [").append(_argname).append("] for option ").append(_optname))
          { }
    };
    
    struct bad_usage :
        public std::invalid_argument
    {
        bad_usage( const std::string& descr ) :
            std::invalid_argument( descr )
          { }
    };

    //std::vector< std::string > args;
  private:
    // data  
    typedef std::vector< Opt > options_container_type;
    options_container_type storage;  
  
    std::string pname;
    std::string brief;
    std::string author;
    std::string copyright;

    bool isterm( const std::string& s );
    bool is_opt_name( const std::string& s );
    bool is_flag_group( const std::string& s );
    bool is_substr(const std::string& small, const std::string& big );
    int get_opt_index( std::string s );
};

template <class T>
void Opts::add(char _shortname,T _default_value,const std::string& _longname,const std::string& _desc)
{
  addflag(_shortname,_longname,_desc);
  std::stringstream ss;
  ss << _default_value;
  storage[storage.size() - 1].default_v = ss.str();
  storage[storage.size() - 1].has_arg = true;
}

template <class T>
T Opts::get( char _shortname )
{
  options_container_type::const_iterator i;
  T res;
  for ( i = storage.begin(); i != storage.end(); ++i ) {
    if ( i->shortname == _shortname ) {
      if ( !i->has_arg ) {
        throw bad_usage("using Opts::get for option without arguments");
      }
    
      std::stringstream ss;
      ss << (i->args.empty() ? i->default_v : i->args[0]);
      ss >> res;

      if (ss.fail()) {
         throw invalid_arg(std::string("-") + std::string(1,_shortname), i->args[0]);
      }
  
      break;
    }
  }

  if ( i == storage.end() ) {
    throw unknown_option( std::string("-") + _shortname );
  }

  return res;
}

template <class T>
T Opts::get_default( char _shortname )
{
  int i;
  T res;
  for (i = 0;i < storage.size();i++)
    if (storage[i].shortname == _shortname)
    {
      if (!storage[i].has_arg)
        throw bad_usage("using Opts::get for option without arguments");
    
      std::stringstream ss;
      ss << storage[i].default_v;

      ss >> res;

      if (ss.fail())
         throw invalid_arg(std::string("-") + std::string(1,_shortname),storage[i].default_v);
  
      break;
    }

  if (i == storage.size())
    throw unknown_option(std::string("-") + std::string(1,_shortname));
  return res;
}

template <class T>
T Opts::get( const std::string& _longname )
{
  int i;
  T res;
  for (i = 0;i < storage.size();i++)
    if (storage[i].longname == _longname)
    {
      if (!storage[i].has_arg)
        throw bad_usage("using Opts::get for option without arguments");
    
      std::stringstream ss;
      if (!storage[i].args.empty())
        ss << storage[i].args[0];
      else
        ss << storage[i].default_v;

      ss >> res;
  
      if (ss.fail()) // need to recover stream?
         throw invalid_arg(std::string("--") + _longname,storage[i].args[0]);
  
      break;
    }

  if (i == storage.size())
    throw unknown_option(std::string("--") + _longname);
  return res;
}

template <class T>
T Opts::get_default( const std::string& _longname )
{
  int i;
  T res;
  for (i = 0;i < storage.size();i++)
    if (storage[i].longname == _longname)
    {
      if (!storage[i].has_arg)
        throw bad_usage("using Opts::get for option without arguments");
    
      std::stringstream ss(storage[i].default_v);

      ss >> res;
  
      if (ss.fail()) // need to recover stream?
         throw invalid_arg(std::string("--") + _longname,storage[i].default_v);
  
      break;
    }

  if (i == storage.size())
    throw unknown_option(std::string("--") + _longname);
  return res;
}

template <class BackInsertIterator>
void Opts::getemall( char _shortname , BackInsertIterator bi)
{
  int i;
  for (i = 0;i < storage.size();i++)
    if (storage[i].shortname == _shortname)
    {
      if (!storage[i].has_arg)
        throw bad_usage("using Opts::getemall for option without arguments");
    
      if (!storage[i].default_v.empty())
      {
        std::stringstream ss(storage[i].default_v);
        ss >> *bi++;
      }

      if (!storage[i].args.empty())
        for (int j = 0;j < storage[i].args.size();j++)
        {

          std::stringstream ss(storage[i].args[j]);
          try
          {
            ss >> *bi++;
          }
          catch(...)
          {
            throw invalid_arg(std::string("-") + std::string(1,_shortname),storage[i].args[j]);
          }
          
          if (ss.fail())
            throw invalid_arg(std::string("-") + std::string(1,_shortname),storage[i].args[j]);
        }
     
      break;
    }

  if (i == storage.size())
    throw unknown_option(std::string("-") + std::string(1,_shortname));
}

template <class BackInsertIterator>
void Opts::getemall( const std::string& _longname , BackInsertIterator bi)
{
  int i;
  for (i = 0;i < storage.size();i++)
    if (storage[i].longname == _longname)
    {
      if (!storage[i].has_arg)
        throw bad_usage("using Opts::getemall for option without arguments");
    
      if (!storage[i].default_v.empty())
      {
        std::stringstream ss(storage[i].default_v);
        ss >> *bi++;
      }

      if (!storage[i].args.empty())
        for (int j = 0;j < storage[i].args.size();j++)
        {

          std::stringstream ss(storage[i].args[j]);
          try
          {
            ss >> *bi++;
          }
          catch(...)
          {
            throw invalid_arg(std::string("--") + _longname,storage[i].args[j]);
          }
          
          if (ss.fail())
            throw invalid_arg(std::string("-") + _longname,storage[i].args[j]);
        }
     
      break;
    }

  if (i == storage.size())
    throw unknown_option(std::string("-") + _longname);
}

#endif
