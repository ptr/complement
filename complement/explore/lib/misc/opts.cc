// -*- C++ -*- Time-stamp: <08/06/28 10:25:43 ptr>

/*
 * Copyright (c) 2008
 * Dmitry Osmakov
 *
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>
#include <cassert>

#include <misc/opts.h>

using namespace std;

string Opts::get_pname() const { return pname; }

bool Opts::is_opt_name(const string& s)
{
  return (s.size() > 1) && (s[0] == '-') && !is_flag_group(s);
}

bool Opts::is_substr(const string& small,const string& big)
{
  if (small.size() > big.size())
    return false;
  for (int i = 0;i < small.size();i++)
    if (small[i] != big[i])
      return false;

  return true;
}

bool Opts::is_flag_group(const string& s)
{
  if (s.size() > 2 && s[0] == '-')
  {
    for (int i = 1;i < s.size();i++)
      if (!isalnum(s[i]))
        return false;
    return true;
  }
  else
    return false;
}

// this function assumes that is_opt_name(s) = true;
Opts::options_container_type::iterator Opts::get_opt_index(string s)
{
  assert(is_opt_name(s));
  if (s.size() == 2 && isalnum(s[1]) ) // is short name
  {  
    options_container_type::iterator i;
    for (i = storage.begin();i != storage.end();++i)
      if (i->shortname == s[1])
        break;
    return i;
  }
  
  if (s.size() > 2 && s[1] == '-')
  {
    options_container_type::iterator i;
    s = s.substr(2);

    // exact match
    for (i = storage.begin();i != storage.end();++i)
      if (i->longname == s)
        return i;

    vector< options_container_type::iterator > matches;
    for (i = storage.begin();i != storage.end();++i)
      if (is_substr(s,i->longname))
        matches.push_back(i);

    if (matches.size() == 1)
      return matches[0];
    else
      return storage.end();
  }
    
  return storage.end();
}

ostream& operator <<( ostream& out, const option& opt )
{
  if ( opt.shortname == 0 ) {
    out << "--" << opt.longname << '\t' << (opt.has_arg ? (string("[") + opt.default_v + "]\t") : "" ) << opt.desc;
  } else {
    out << '-' << opt.shortname << '\t' << (!opt.longname.empty() ? (string("[--") + opt.longname +"]\t") : "")  << (opt.has_arg ? string("[") + opt.default_v + ("]\t") : "")  << opt.desc; 
  }
  
  return out;
}

void Opts::help(ostream& out)
{
  if (!_brief.empty())
    out << _brief << endl;
  if (!_author.empty())
    out << _author << endl;
  if (!_copyright.empty())
    out << _copyright << endl;  
  
  out << "Valid options:" << endl;
  options_container_type::const_iterator i;
  for (i = storage.begin();i != storage.end();++i)
  {
    out << *i << endl;
  }
}

int Opts::addflag( char _shortname, const string& _longname, const string& _desc )
{
  option opt( _desc.c_str(), _shortname, _longname.c_str() );

  opt.has_arg = false;
  opt.token = ++free_token;
  storage.push_back(opt);
  return opt.token;
}

int Opts::addflag( const string& _longname, const string& _desc )
{
  option opt( _desc.c_str(), _longname.c_str() );

  opt.has_arg = false;
  opt.token = ++free_token;
  storage.push_back(opt);
  return opt.token;
}

void Opts::parse(int& ac,const char** av)
{
  pname = av[0];

  int i = 1;
  int j = 1;
  int q = 0;
  while ( (i < ac) && (string(av[i]) != "--") )
  {
    if (is_opt_name(av[i]))
    {
      string opt = av[i];
      string arg;
      
      int k = opt.find("=");

      if (k != string::npos)
      {
        arg = opt.substr(k + 1);
        opt = opt.substr(0,k);
      }

      options_container_type::iterator  p = get_opt_index(opt);
      
      if (p == storage.end())
        throw unknown_option(opt);
      else
      {
        p->pos.push_back(++q);
        if (p->has_arg)
        {
          if (!arg.empty())
            p->args.push_back(arg);
          else
            if (i + 1 < ac)
              p->args.push_back(av[++i]);
            else
              throw missing_arg(opt);
        }
        else
          if (!arg.empty()) //unexpected arg (not exactly mismatch)
            throw arg_typemismatch(opt,arg); 
      }  
    }
    else
    if (is_flag_group(av[i]))
    {
      string optgroup = av[i];
      for (int j = 1;j < optgroup.size();j++)
      {
        options_container_type::iterator p = get_opt_index(string("-") + optgroup[j]);
        if (p == storage.end())
        {
          throw unknown_option( "-" + string(1,optgroup[j]) );
        }
        else
        {
          p->pos.push_back(++q);
          if (p->has_arg)
            throw missing_arg( "-" + string(1,optgroup[j]) );
        }
      }
    }
    else
      av[j++] = av[i];
    i++;
  }
  
  i += ( i < ac );

  while (i < ac)
    av[j++] = av[i++]; 
  ac = j;
}
