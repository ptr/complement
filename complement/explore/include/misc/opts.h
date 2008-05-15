// -*- C++ -*- Time-stamp: <08/05/01 12:02:26 ptr>

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
    std::string v;

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
    void add( char _shortname,T default_value,const std::string& _longname = "", const std::string& _desc = "" );
    void addflag( char _shortname,const std::string& _longname = "",const std::string& _desc = "" );

    // getting option
    template <class T>
    T get( char _shortname );

    template <class T>
    T get( const std::string& _longname );
  
    template <class BackInsertIterator>
    void getemall(char _shortname,BackInsertIterator bi);

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
    struct invalid_opt : public std::logic_error
    {
        invalid_opt(const std::string& _optname) :
            std::logic_error(std::string("invalid opt: ").append(_optname))
          { }
    };
  
    struct missing_arg : public std::logic_error
    {
        missing_arg( const std::string& _optname) :
            std::logic_error(std::string("missing argument for option ").append(_optname))
          { }
    };

    struct invalid_arg : public std::logic_error
    {
        invalid_arg( const std::string& _optname, const std::string& _argname) :
            std::logic_error(std::string("invalid argument [").append(_argname).append("] for option ").append(_optname))
          { }
    };
    
    struct bad_usage : public std::runtime_error
    {
        bad_usage( const std::string& what) :
          std::runtime_error(what)
         { }
    };

  
    //std::vector< std::string > args;
  private:
    // data  
    std::vector< Opt > storage;  
  
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
  storage[storage.size() - 1].v = ss.str();
  storage[storage.size() - 1].has_arg = true;
}

template <class T>
T Opts::get( char _shortname )
{
  int i;
  T res;
  for (i = 0;i < storage.size();i++)
    if (storage[i].shortname == _shortname)
    {
      if (!storage[i].has_arg)
        throw bad_usage("using Opts::get for option without arguments");
    
      if (!storage[i].args.empty())
        storage[i].v = storage[i].args[0];

      try
      {
         std::stringstream ss(storage[i].v);
         ss >> res;
      }
      catch(...)
      {
         throw invalid_arg(std::string("-") + std::string(1,_shortname),storage[i].v);
      }
  
      break;
    }

  if (i == storage.size())
    throw invalid_opt(std::string("-") + std::string(1,_shortname));
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
    
      if (!storage[i].args.empty())
        storage[i].v = storage[i].args[0];

      try
      {
         std::stringstream ss(storage[i].v);
         ss >> res;
      }
      catch(...)
      {
         throw invalid_arg(std::string("--") + _longname,storage[i].v);
      }
  
      break;
    }

  if (i == storage.size())
    throw invalid_opt(std::string("--") + _longname);
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
    
      if (!storage[i].v.empty())
      {
        std::stringstream ss(storage[i].v);
        ss >> *bi++;
      }

      if (!storage[i].args.empty())
        for (int j = 0;j < storage[i].args.size();j++)
        {
          try
          {
            std::stringstream ss(storage[i].args[j]);
            ss >> *bi++;
          }
          catch(...)
          {
            throw invalid_arg(std::string("-") + std::string(1,_shortname),storage[i].v);
          }
        }
     
      break;
    }

  if (i == storage.size())
    throw invalid_opt(std::string("-") + std::string(1,_shortname));
}


#endif
