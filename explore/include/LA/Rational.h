// -*- C++ -*- Time-stamp: <96/04/09 13:37:27 ptr>
#ifndef __LA_Rational_h
#define __LA_Rational_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifndef __GNUC__
#ifndef BOOL_H
#include <stl/bool.h>
#endif
#endif

template <class I>
class Rational
{
  protected:
    I num;
    I den;

  public:
    Rational()
      { }
    Rational( const I& n ) :
	num( n ),
	den( 1L )
      { }
    Rational( const I& n, const I& d ) :
	num( n ),
	den( d )
      { normalize(); }
    Rational( long n, long d = 1L ) :
	num( n ),
	den( d )
      { normalize(); }

    Rational& operator =( const Rational& x )
      {
	num = x.num;
	den = x.den;
	return *this;
      }
    Rational& operator =( const I& x )
      {
	num = x;
	den = 1L;
	return *this;
      }

    Rational operator +( const Rational& x ) const;
    Rational operator -( const Rational& x ) const;
    Rational operator *( const Rational& x ) const;
    Rational operator /( const Rational& x ) const;
    Rational operator -() const;
    Rational operator +( const I& x ) const;
    Rational operator -( const I& x ) const;
    Rational operator *( const I& x ) const;
    Rational operator /( const I& x ) const;

    Rational& operator +=( const Rational& x );
    Rational& operator -=( const Rational& x );
    Rational& operator *=( const Rational& x );
    Rational& operator /=( const Rational& x );

    bool operator <( const Rational& x ) const;
    bool operator ==( const Rational& x ) const;

    I trunc() const
      { return num / den; }
    I floor() const;
    I ceil() const;
    I round() const;

  protected:
    void normalize();

    friend ostream& operator <<( ostream&, const Rational<I>& );
    friend istream& operator >>( istream&, Rational<I>& );
};

template <class I>
void Rational<I>::normalize()
{
  if ( den == 1L ) {
    return;
  }
  I g( num.gcd( den ) );
  if ( g != 1L ) {
    num /= g;
    den /= g; 
  }
}

template <class I>
Rational<I> Rational<I>::operator +( const Rational& x ) const
{
  Rational<I> r;
  r.num = x.num * den;
  r.num += x.den * num;
  r.den = x.den * den;
  r.normalize();

  return r;
}

template <class I>
Rational<I> Rational<I>::operator -( const Rational& x ) const
{
  Rational<I> r;
  r.num = x.den * num;
  r.num -= x.num * den;
  r.den = x.den * den;
  r.normalize();

  return r;
}

template <class I>
Rational<I> Rational<I>::operator *( const Rational& x ) const
{
  return Rational<I>( num * x.num, den * x.den );
}

template <class I>
Rational<I> Rational<I>::operator /( const Rational& x ) const
{
  if ( x.num < 0L ) {
    return Rational<I>( -num * x.den, -den * x.num );
  }
  return Rational<I>( num * x.den, den * x.num );
}

template <class I>
Rational<I> Rational<I>::operator -() const
{
  return Rational<I>( -num, den );
}

template <class I>
Rational<I> Rational<I>::operator +( const I& x ) const
{
  Rational<I> r;
  r.num = x * den;
  r.num += num;
  r.den = den;
  r.normalize();

  return r;
}

template <class I>
Rational<I> Rational<I>::operator -( const I& x ) const
{
  Rational<I> r;
  r.num = num;
  r.num -= x * den;
  r.den = den;
  r.normalize();

  return r;
}

template <class I>
Rational<I> Rational<I>::operator *( const I& x ) const
{
  return Rational<I>( num * x, den );
}

template <class I>
Rational<I> Rational<I>::operator /( const I& x ) const
{
  if ( x < 0L ) {
    return Rational<I>( -num, -den * x );
  }
  return Rational<I>( num, den );
}


template <class I>
Rational<I>& Rational<I>::operator +=( const Rational& x )
{
  num *= x.den;
  num += x.num * den;
  den *= x.den;
  normalize();

  return *this;
}

template <class I>
Rational<I>& Rational<I>::operator -=( const Rational& x )
{
  num *= x.den;
  num -= x.num * den;
  den *= x.den;
  normalize();

  return *this;
}

template <class I>
Rational<I>& Rational<I>::operator *=( const Rational& x )
{
  num *= x.num;
  den *= x.den;
  normalize();
  return *this;
}

template <class I>
Rational<I>& Rational<I>::operator /=( const Rational& x )
{
  num *= x.den;
  den *= x.num;
  if ( x.num < 0L ) {
    den = -den;
    num = -num;
  }
  normalize();
  return *this;
}

template <class I>
bool Rational<I>::operator <( const Rational& x ) const
{
  return num * x.den < den * x.num;
}

template <class I>
bool Rational<I>::operator ==( const Rational& x ) const
{
  return num == x.num && den == x.den;
}

template <class I>
I Rational<I>::floor() const
{
  if ( num >= 0L ) {
    return num / den;
  }
  I r;
  I i( num.div( den, r ) );

  return r != 0L ? --i : i;
}

template <class I>
I Rational<I>::ceil() const
{
  if ( num >= 0L ) {
    I r;
    I i( num.div( den, r ) );
    
    return r != 0L ? ++i : i;
  }
  return num / den;
}

template <class I>
I Rational<I>::round() const
{
  I r;
  I i( num.div( den, r ) );
  r <<= 2;
  return den < r ? (num >= 0L ? ++i : --i) : i;
}

#endif // __LA_Rational_h
