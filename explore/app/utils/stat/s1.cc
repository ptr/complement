#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

using namespace std;

struct rec {
    string name;
    long size;
    long time;
    long count;
    long size_acc;

    rec& operator =( const rec& r )
      {
        name = r.name; size = r.size; time = r.time; count = r.count;
        size_acc = r.size_acc;
        return *this;
      }

    rec& operator =( long c )
      {
        count = c;
        return *this;
      }
};

bool less_tm( const rec& r1,  const rec& r2 )
{ return r1.time < r2.time; }

bool equal_tm( const rec& r1,  const rec& r2  )
{ return r1.time == r2.time; }

void out( const rec& r1 )
{
  cout << r1.name << ' ' << r1.size << ' ' << r1.time
       << ' ' << r1.count << ' ' << r1.size_acc << '\n';
}

rec sumsize( const rec& r1,  const rec& r2  )
{
  rec r( r2 );
  r.size_acc = r2.size + r1.size_acc;
  return r;
}

class incr
{
  public:
    incr() :
        _count(0)
      { }

    long operator ()()
      { return ++_count; }

  private:
    long _count;
};

int main()
{
  ifstream f( "stat" );
  rec r;
  vector<rec> v;

  do {
    f >> r.name >> r.size >> r.time;
    if ( !f.fail() ) {
      v.push_back( r );
    }
  } while ( f.good() );

  // sort by time
  sort( v.begin(), v.end(), less_tm );

  // count samples
  // incr g;
  generate( v.begin(), v.end(), incr() );

  //vector<rec> v3;
  //back_insert_iterator<vector<rec> > ib0( v3 );

  v[0].size_acc = v[0].size;
  partial_sum( v.begin(), v.end(), v.begin(), sumsize );

//  vector<rec> v2;
//  back_insert_iterator<vector<rec> > ib( v2 );

//  unique_copy( v.rbegin(), v.rend(), ib, equal_tm );
//  reverse( v2.begin(), v2.end() );

  for_each( v.begin(), v.end(), out );
  // for_each( v2.begin(), v2.end(), out );

  return 0;
}
