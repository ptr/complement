#ifndef __OPTS_H__
#define __OPTS_H__

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <typeinfo>
#include <cctype>

using namespace std;

class Opt
{
public:
  char shortname;
  string longname;
  string desc;
  vector< string > args;

  bool has_arg;
  bool is_set;
};

class Opts
{
public:
// construct
  Opts(const string& _brief = "",const string& _author = "",const string& _copyright = "") : brief(_brief) , author(_author) , copyright(_copyright) {};

// adding option 
  void add(char _shortname,const string& _longname = "",const string& _desc = "",bool has_arg = false);

// getting option
  template <class T>
  T get(char _shortname,T& dest);
  template <class T>
  T get(const string& _longname,T& dest);
  
  bool is_set(char _shortname);
  bool is_set(const string& _longname);


// parse
  void parse(int ac,char** av);

// stuff
  void help(ostream& out);
  string get_pname() const;
  string get_brief() const;
  string get_author() const;
  string get_copyright() const;

// error handling
  struct invalid_opt { string optname; invalid_opt(const string& _optname) : optname(_optname) {}; };
  struct missing_arg { string optname; missing_arg(const string& _optname) : optname(_optname) {}; };
  struct invalid_arg { string optname,argname; invalid_arg(const string& _optname,const string& _argname) : optname(_optname) , argname(_argname) {}; };

  
  vector< string > args;
private:
  // data  
  vector< Opt > storage;  
  
  string pname;
  string brief;
  string author;
  string copyright;

  bool isterm(const string& s);
  bool is_opt_name(const string& s);
  bool is_flag_group(const string& s);
  bool is_substr(const string& small,const string& big);
  int get_opt_index(string s);
};

template <class T>
T Opts::get(char _shortname,T& res)
{
  int i;
  for (i = 0;i < storage.size();i++)
    if (storage[i].shortname == _shortname)
    {
      if (storage[i].is_set && storage[i].has_arg)
        if (!storage[i].args.empty())
        {
          try
          {
            stringstream ss(storage[i].args[0]);
            ss >> res;
          }
          catch(...)
          {
            throw invalid_arg(string("-") + string(1,_shortname),storage[i].args[0]);
          }
        }
      break;
    }  
  if (i == storage.size())
    throw invalid_opt(string("-") + string(1,_shortname));
  return res;
}

template <class T>
T Opts::get(const string& _longname,T& res)
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
            stringstream ss(storage[i].args[0]);
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
