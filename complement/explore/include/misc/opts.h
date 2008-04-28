#ifndef __OPTS_H__
#define __OPTS_H__

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <typeinfo>
#include <cctype>

using namespace std;

class Value
{
public:
	void* ptr;
	Value() { ptr = 0; }
};

class Opt
{
public:
	char shortname;
	string longname;
	string desc;
	vector< string > args;
	Value  val; // must be vector

	bool has_arg;
	bool is_set;
};

class Opts
{
public:
// construct
	Opts(const string& _brief = "",const string& _author = "",const string& _copyright = "") : brief(_brief) , author(_author) , copyright(_copyright) {};

// adding option 
	// 4 params
	template <class T>
	void add(char _shortname,const string& _longname,const string& _desc,T v);
	
	
	// 3 params
	template <class T>
	void add(char _shortname,const string& desc,T v);
	template <class T>
	void add(const string& longname,const string& desc,T v);
	
	// 2 params
	template <class T>
	void add(char _shortname,T v);
	template <class T>
	void add(const string& _longname,T v);

	// 1 param - randomly generated names?

// adding flag 
	void addf(char _shortname,const string& _longname,const string& _desc);
	void addf(char _shortname,const string& _desc);
	void addf(const string& _longname,const string& _desc);

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

private:
	// data	
	vector< Opt > storage;	
	vector< string > args;
	
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
void Opts::add(char _shortname,const string& _longname,const string& _desc,T v)
{
	Opt opt;

	//opt.val.tinfo = typeid(T);
	opt.val.ptr = new T;
	*(reinterpret_cast<T*>(opt.val.ptr)) = v;

	opt.shortname = _shortname;
	opt.longname = _longname;
	opt.desc = _desc;

	opt.has_arg = true;
	opt.is_set = false;
	
	storage.push_back(opt);
}

template <class T>
void Opts::add(char _shortname,const string& _desc,T v)
{
	Opt opt;

	//opt.val.tinfo = typeid(T);
	opt.val.ptr = new T;
	*(reinterpret_cast<T*>(opt.val.ptr)) = v;

	opt.shortname = _shortname;
	opt.desc = _desc;

	opt.has_arg = true;
	opt.is_set = false;
	
	storage.push_back(opt);
}

template <class T>
void Opts::add(const string& _longname,const string& _desc,T v)
{
	Opt opt;

	//opt.val.tinfo = typeid(T);
	opt.val.ptr = new T;
	*(reinterpret_cast<T*>(opt.val.ptr)) = v;

	opt.longname = _longname;
	opt.desc = _desc;

	opt.has_arg = true;
	opt.is_set = false;
	
	storage.push_back(opt);
}

template <class T>
void Opts::add(char _shortname,T v)
{
	Opt opt;

	//opt.val.tinfo = typeid(T);
	opt.val.ptr = new T;
	*(reinterpret_cast<T*>(opt.val.ptr)) = v;

	opt.shortname = _shortname;

	opt.has_arg = true;
	opt.is_set = false;
	
	storage.push_back(opt);
}

template <class T>
void Opts::add(const string& _longname,T v)
{
	Opt opt;

	//opt.val.tinfo = typeid(T);
	opt.val.ptr = new T;
	*(reinterpret_cast<T*>(opt.val.ptr)) = v;

	opt.longname = _longname;

	opt.has_arg = true;
	opt.is_set = false;
	
	storage.push_back(opt);
}

template <class T>
T Opts::get(char _shortname,T& res)
{
	int i;
	for (i = 0;i < storage.size();i++)
		if (storage[i].shortname == _shortname)
		{
			if (storage[i].is_set && storage[i].has_arg)
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
			else
			{
				res = *reinterpret_cast<T*>(storage[i].val.ptr);
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
			else
			{
				res = *reinterpret_cast<T*>(storage[i].val.ptr);
			}
			break;
		}	
	if (i == storage.size())
		throw invalid_opt(_longname);
	return res;	
}

#endif
