// -*- C++ -*- Time-stamp: <08/05/01 12:02:26 ptr>

#ifndef __OPTS_H__
#define __OPTS_H__

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <typeinfo>
#include <cctype>

class Opt
{
  public:
    Opt() { cnt = 0; }
    char shortname;
    std::string longname;
    std::string desc;
    std::vector< std::string > args;

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

    // adding option 
    void add( char _shortname, const std::string& _longname = "", const std::string& _desc = "", bool has_arg = false );

    // getting option
    template <class T>
    T get( char _shortname, T& dest );

    template <class T>
    T get( const std::string& _longname, T& dest );
  
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
    struct invalid_opt
    {
        std::string optname;

        invalid_opt(const std::string& _optname) :
            optname(_optname)
          { }
    };

    struct missing_arg
    {
        std::string optname;

        missing_arg( const std::string& _optname) :
            optname(_optname)
          { }
    };

    struct invalid_arg
    {
        std::string optname;
        std::string argname;

        invalid_arg( const std::string& _optname, const std::string& _argname) :
            optname(_optname),
            argname(_argname)
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
T Opts::get( char _shortname, T& res )
{
  int i;
  for (i = 0;i < storage.size();i++) {
    if (storage[i].shortname == _shortname)
    {
      if (storage[i].is_set && storage[i].has_arg)
        if (!storage[i].args.empty())
        {
          try
          {
            std::stringstream ss(storage[i].args[0]);
            ss >> res;
          }
          catch(...)
          {
            throw invalid_arg(std::string("-") + std::string(1,_shortname),storage[i].args[0]);
          }
        }
      break;
    }
  }
  if (i == storage.size())
    throw invalid_opt(std::string("-") + std::string(1,_shortname));
  return res;
}

template <class T>
T Opts::get(const std::string& _longname,T& res)
{
  int i;
  for (i = 0;i < storage.size();i++)
    if (storage[i].longname == _longname)
    {
      if (storage[i].is_set && storage[i].has_arg)
        if  (!storage[i].args.empty())
        {
          try
          {
            std::stringstream ss(storage[i].args[0]);
            ss >> res;
          }
          catch(...)
          {
            throw invalid_arg(_longname,storage[i].args[0]);
          }
        }
      break;
    }  
  if (i == storage.size())
    throw invalid_opt(_longname);
  return res;  
}

#endif
