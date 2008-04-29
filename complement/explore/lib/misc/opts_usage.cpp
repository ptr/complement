#include <iostream>
#include <string>
#include "opts.h"

using namespace std;

struct point
{
  int x;
  int y;
  point(int _x = 0,int _y = 0) : x(_x) , y(_y) {};
};

istream& operator>>(istream& t,point& p)
{
  t >> p.x >> p.y;
  return t;  
}

ostream& operator<<(ostream& t,const point& p)
{
  t << p.x << ' ' << p.y;
  return t;
}

int main(int ac,char** av)
{
  Opts opts;

  // control variables with default values  
  point p(1,1);
  string name = "maos";
  int port = 80;
  
  opts.add('h',"help","display help message");
  opts.add('v',"verbose","verbose");
  opts.add('j',"just","just do it");

  opts.add('p',"port","port number",true);
  opts.add('s',"point","start point",true);
  opts.add('n',"name","your name",true);
  opts.add('g');
  
  try
  {
    opts.parse(ac,av);
  }
  catch(Opts::invalid_opt& t)
  {
    cout << "Invalid option: " << t.optname << endl;
    return 1;
  }
  catch(Opts::invalid_arg& t)
  {
    cout << "Invalid argument: " << t.optname << ' ' << t.argname << endl;
    return 1;
  }
  catch(Opts::missing_arg& t)
  {
    cout << "Missing argument: " << t.optname << endl;
    return 1;
  }
  
  if (opts.is_set('h'))  
    opts.help(cout);

  if (opts.is_set('v'))
    cout << "Verbose mode is set" << endl;
  else
    cout << "Verbose mode is not set" << endl;

  if (opts.is_set('g'))
    cout << "-g is set" << endl;
  else
    cout << "-g is not set" << endl;

  if (opts.is_set("just"))
    cout << "Just do it!" << endl;
  else
    cout << "Just don't do it!" << endl;

  cout << "port = " << opts.get('p',port) << endl;
  cout << "point = " << opts.get('s',p) << endl;
  cout << "name = " << opts.get('n',name) << endl; 

  cout << "operands: " << endl;

  for (int i = 0;i < opts.args.size();i++)
    cout << opts.args[i] << endl;

  return 0;
}
