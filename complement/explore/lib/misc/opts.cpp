#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>

#include <misc/opts.h>

using namespace std;

string Opts::get_pname() const { return pname; }
string Opts::get_brief() const { return brief; }
string Opts::get_author() const { return author; }
string Opts::get_copyright() const { return copyright; }

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
int Opts::get_opt_index(string s)
{
  if (s.size() == 2 && isalnum(s[1]) ) // is short name
  {  
    int i;
    for (i = 0;i < storage.size();i++)
      if (storage[i].shortname == s[1])
        break;
    return i;
  }
  
  if (s.size() > 2 && s[1] == '-')
  {
    int i;
    s = s.substr(2);

    // exact match
    for (i = 0;i < storage.size();i++)
      if (storage[i].longname == s)
        return i;

    vector<int> matches;
    for (i = 0;i < storage.size();i++)
      if (is_substr(s,storage[i].longname))
        matches.push_back(i);

    if (matches.size() == 1)
      return matches[0];
    else
      return storage.size();
  }
    
  return storage.size();
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
  out << "available options:" << endl;
  for (int i = 0;i < storage.size();i++)
    out << "-" << storage[i].shortname << "\t[--" << storage[i].longname << "] [" << storage[i].default_v << "]\t-\t" << storage[i].desc << endl;
}

void Opts::addflag(char _shortname,const string& _longname,const string& _desc)
{
  Opt opt;
  opt.shortname = _shortname;
  opt.longname = _longname;
  opt.desc = _desc;
  opt.has_arg = false;
  opt.is_set = false;
  storage.push_back(opt);
}


bool Opts::is_set(char _shortname)
{
  for (int i = 0;i < storage.size();i++)
    if (storage[i].shortname == _shortname)
      return storage[i].is_set;
  return false;
}

bool Opts::is_set(const string& _longname)
{
  for (int i = 0;i < storage.size();i++)
    if (storage[i].longname == _longname)
      return storage[i].is_set;
  return false;
}

int Opts::get_cnt(char _shortname) const
{
  for (int i = 0;i < storage.size();i++)
    if (storage[i].shortname == _shortname)
      return storage[i].cnt;
  return 0;
}

int Opts::get_cnt(const string& _longname) const
{
  for (int i = 0;i < storage.size();i++)
    if (storage[i].longname == _longname)
      return storage[i].cnt;
  return 0;
}

void Opts::parse(int& ac,const char** av)
{
  pname = av[0];

  int i = 1;
  int j = 1;
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

      int p = get_opt_index(opt);

      if (p == storage.size())
        throw unknown_option(opt);
      else
      {
        storage[p].is_set = true;
        storage[p].cnt++;
        if (storage[p].has_arg)
        {
          if (!arg.empty())
            storage[p].args.push_back(arg);
          else
            if (i + 1 < ac)
              storage[p].args.push_back(av[++i]);
            else
              throw missing_arg(opt);
        }
        else
          if (!arg.empty()) //unexpected arg
            throw invalid_arg(opt,arg);
      }  
    }
    else
    if (is_flag_group(av[i]))
    {
      string optgroup = av[i];
      for (int j = 1;j < optgroup.size();j++)
      {
        int p = get_opt_index(string("-") + optgroup[j]);
        if (p == storage.size())
          throw unknown_option( "-" + string(1,optgroup[j]) );
        else
        {
          storage[p].is_set = true;  
          storage[p].cnt++;
          if (storage[p].has_arg)
            throw missing_arg( "-" + string(1,optgroup[j]) );
        }
      }
    }
    else
    {
      av[j++] = av[i];
      //args.push_back(av[i]);
    }
    i++;
  }
  
  i += (i < ac && isterm(av[i]));

  while (i < ac)
    av[j++] = av[i++]; //args.push_back(av[i++]);
  ac = j;
}
