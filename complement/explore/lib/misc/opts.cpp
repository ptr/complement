#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>
#include <cassert>

#include <misc/opts.h>

using namespace std;

string Opts::get_pname() const { return pname; }

bool Opts::isterm(const string& s)
{
  return (s == "--");
}

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

void Opts::help(ostream& out)
{
  if (!brief.empty())
    out << brief << endl;
  if (!author.empty())
    out << author << endl;
  if (!copyright.empty())
    out << copyright << endl;  
  
  out << "usage: " << endl;
  out << pname  << " [option ...] [optiongoup ...] [end operands ...]" << endl;  
  out << "available options:" << endl << "shortname:" << '\t' << "longname:" << '\t' << "default value" << '\t' << "description" << endl;
  options_container_type::const_iterator i;
  for (i = storage.begin();i != storage.end();++i)
  {
    out << i->shortname << "\t[--" << i->longname << "] [" << i->default_v << "]\t-\t" << i->desc << endl;
  }
}

int Opts::addflag(char _shortname,const string& _longname,const string& _desc)
{
  Opt opt;
  opt.shortname = _shortname;
  opt.longname = _longname;
  opt.desc = _desc;
  opt.has_arg = false;
  opt.is_set = false;
  opt.token = ++free_token;
  storage.push_back(opt);
  return opt.token;
}

int Opts::addflag(const string& _longname,const string& _desc)
{
  Opt opt;
  opt.longname = _longname;
  opt.desc = _desc;
  opt.has_arg = false;
  opt.is_set = false;
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
  while (i < ac && !isterm(av[i]))
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
        p->is_set = true;
        p->cnt++;
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
          p->is_set = true;  
          p->cnt++;
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
  
  i += (i < ac && isterm(av[i]));

  while (i < ac)
    av[j++] = av[i++]; 
  ac = j;
}
