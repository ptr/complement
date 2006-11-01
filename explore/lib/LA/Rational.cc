// -*- C++ -*- Time-stamp: <96/04/09 15:15:40 ptr>

#include <LA/Rational.h>
#include <LA/Integer.h>

#ifdef __GNUC__
typedef la_int<4, long long, unsigned long, long, la_int_traits> Integer4;
#endif

ostream& operator <<( ostream& s, const Rational<Integer4 >& i )
{
  return s << i.num << "/" << i.den;
}

istream& operator >>( istream& s, Rational<Integer4 >& i )
{
  s >> i.num >> i.den;
  i.normalize();
  return s;
}
