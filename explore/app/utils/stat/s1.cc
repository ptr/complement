#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

using namespace std;

struct rec
{
    string name;
    unsigned long size;
    unsigned long time;
    unsigned long count;
    unsigned long long size_acc;

    rec& operator =( const rec& r )
      {
        name = r.name;
        size = r.size;
        time = r.time;
        count = r.count;
        size_acc = r.size_acc;
        return *this;
      }

    rec& operator =( unsigned long c )
      {
        count = c;
        return *this;
      }
};

struct derive_rec
{
    unsigned long time;
    double num;
    double size;
};

bool less_tm( const rec& r1,  const rec& r2 )
{ return r1.time < r2.time; }

bool less_sz( const rec& r1,  const rec& r2 )
{ return r1.size < r2.size; }

bool equal_tm( const rec& r1,  const rec& r2  )
{ return r1.time == r2.time; }

bool equal_sz( const rec& r1,  const rec& r2  )
{ return r1.size == r2.size; }

void out( const rec& r1 )
{
  cout << r1.name << ' ' << r1.size << ' ' << r1.time
       << ' ' << r1.count << ' ' << r1.size_acc << '\n';
}

void out_der( const derive_rec& r1 )
{
  cout << r1.time << ' ' << r1.num << ' ' << r1.size << '\n';
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

void time_dist( vector<rec>& v )
{
  // sort by time
  sort( v.begin(), v.end(), less_tm );

  // count samples
  // incr g;
  generate( v.begin(), v.end(), incr() );

  //vector<rec> v3;
  //back_insert_iterator<vector<rec> > ib0( v3 );

  v[0].size_acc = v[0].size;
  partial_sum( v.begin(), v.end(), v.begin(), sumsize );

  vector<rec> v2;
  back_insert_iterator<vector<rec> > ib( v2 );

  unique_copy( v.rbegin(), v.rend(), ib, equal_tm );
  reverse( v2.begin(), v2.end() );

  // for_each( v.begin(), v.end(), out );
  // for_each( v2.begin(), v2.end(), out );

  unsigned long t = 0;
  rec last;
  vector<rec> vgrp;
  for (vector<rec>::iterator i = v2.begin(); i != v2.end(); ++i ) {
    // last = *i;
    if ( t == 0 ) {
      t = (i->time / 60 + 1) * 60;
    }
    if ( i->time > t ) {
      last.time = t;
      vgrp.push_back( last );
      if ( i->time > (t + 60) ) {
        rec empty = last;
        empty.time += 60;
        while ( empty.time < i->time ) {
          vgrp.push_back( empty );
          empty.time += 60;
        }
      }
      t = (i->time / 60 + 1) * 60;
    }
    last = *i;
  }
  
  // for_each( vgrp.begin(), vgrp.end(), out );

  vector<derive_rec> vder;

  unsigned long last_num = 0;
  unsigned long long last_size = 0;
  derive_rec r;
  for (vector<rec>::iterator i = vgrp.begin(); i != vgrp.end(); ++i ) {
    r.time = i->time;
    r.num = (i->count - last_num) / 60.0;
    r.size = (i->size_acc - last_size) / 60.0;
    vder.push_back( r );
    last_num = i->count;
    last_size = i->size_acc;
  }

  for_each( vder.begin(), vder.end(), out_der );
}

void size_dist( vector<rec>& v )
{
  // sort by size
  sort( v.begin(), v.end(), less_sz );
  generate( v.begin(), v.end(), incr() );

  vector<rec> v2;
  back_insert_iterator<vector<rec> > ib( v2 );

  unique_copy( v.rbegin(), v.rend(), ib, equal_sz );
  reverse( v2.begin(), v2.end() );

  // for_each( v2.begin(), v2.end(), out );
#if 1
  const unsigned long sz_grid = 512;
  unsigned long s = 0;
  rec last;
  vector<rec> vgrp;
  for (vector<rec>::iterator i = v2.begin(); i != v2.end(); ++i ) {
    if ( s == 0 ) {
      s = (i->size / sz_grid + 1) * sz_grid;
    }
    if ( i->size > s ) {
      last.size = s;
      vgrp.push_back( last );
      if ( i->size > (s + sz_grid) ) {
        rec empty = last;
        empty.size += sz_grid;
        while ( empty.size < i->size ) {
          vgrp.push_back( empty );
          empty.size += sz_grid;
        }
      }
      // cout << last.size << ' ' << last.count << endl;
      s = (i->size / sz_grid + 1) * sz_grid;
    }
    last = *i;
  }

  for_each( vgrp.begin(), vgrp.end(), out );

  double max_count = (*(vgrp.end() - 1)).count;

/*
  vector<derive_rec> vder;

  unsigned long last_num = 0;
  derive_rec r;
  for (vector<rec>::iterator i = vgrp.begin(); i != vgrp.end(); ++i ) {
    // r.time = i->time;
    r.num = (i->count - last_num) / max_count;
    r.size = i->size;
    vder.push_back( r );

    last_num = i->count;
  }

  for_each( vder.begin(), vder.end(), out_der );
*/
#endif
}

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

  // time_dist( v );
  size_dist( v );

  return 0;
}
