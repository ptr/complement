// -*- C++ -*- Time-stamp: <96/04/04 18:03:23 ptr>
#ifndef __LA_Integer_h
#define __LA_Integer_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifndef _LIMITS_H
#include <limits.h>
#endif

#ifndef BOOL_H
#include <stl/bool.h>
#endif

#ifndef _IOSTREAM_H
#include <iostream.h>
#endif

#include <CLASS/checks.h>
#include <stl/algobase.h>

const unsigned long sign_mask = ~((unsigned long)LONG_MAX);
const unsigned long no_sign_mask = (unsigned long)LONG_MAX;
const unsigned long long_bits = sizeof( unsigned long ) * 8;
const unsigned long hi_bit_mask = (1UL << (long_bits - 2));

union __longlong {
    long long A;
    struct {
	unsigned long hi;
	unsigned long lo;
    } a;
};

class Integer_overflow
{
};

template <int P>
class Integer
{
  public:
    Integer()
      { }
    Integer( long );
    Integer( unsigned long );
    Integer( const Integer& );

    Integer& operator =( const Integer& );
    Integer& operator =( long );
    Integer& operator =( unsigned long );

    Integer& operator +=( const Integer& );
    Integer& operator -=( const Integer& );
    Integer& operator *=( const Integer& );
    Integer& operator /=( const Integer& );
    Integer& div1( const Integer&, Integer & );

    Integer operator +( const Integer& ) const;
    Integer operator -( const Integer& ) const;
    Integer operator *( const Integer& ) const;
    Integer operator /( const Integer& ) const;
    Integer operator %( const Integer& ) const;
    Integer div( const Integer&, Integer & ) const;

    bool operator <( const Integer& ) const;
    bool operator ==( const Integer& ) const;
    
  protected:
    bool __overflow( unsigned long x ) const
      { return long(x) < 0; }
    void radix_complement();
    bool isNegative() const
      { return long(*d) < 0; }
    void multiply_aux( const Integer &, unsigned long * ) const;
    unsigned long *divide_aux( unsigned long *, unsigned long,
			       unsigned long * ) const;
    void shl_aux( unsigned long *, unsigned long *, int ) const;
    unsigned long *divide_aux2( unsigned long *, unsigned long *,
				unsigned long *, int, int ) const;
    void divide_aux3( const Integer&, unsigned long *theResult,
		      unsigned long *theRest = 0 ) const;

    unsigned long d[P];

    friend ostream& operator <<( ostream&, const Integer<P>& );
};

template <int P>
Integer<P>::Integer( unsigned long x )
{
  unsigned long *last   = &d[P];
  unsigned long *first  = &d[0];

  *--last = x;
  while ( first < last ) {
    *first++ = 0UL;
  }
  if ( __overflow( x ) && P > 1) {
    *last-- &= no_sign_mask;
    *last = 1UL;
  }
}

template <int P>
Integer<P>::Integer( long x )
{
  unsigned long *last   = &d[P];
  unsigned long *first  = &d[0];

  *--last = x;
  if ( x >= 0L ) {
    while ( first < last ) {
      *first++ = 0UL;
    }
  } else if ( P > 1 ) {
    *last &= no_sign_mask;
    *first++ = ULONG_MAX;
    while ( first > last ) {
      *first++ = LONG_MAX;
    }
  }
}

template <int P>
Integer<P>::Integer( const Integer& x )
{
  copy( x.d, x.d + P, d );
}

template <int P>
Integer<P>& Integer<P>::operator =( const Integer& x )
{
  copy( x.d, x.d + P, d );
  return *this;
}

template <int P>
Integer<P>& Integer<P>::operator =( unsigned long x )
{
  unsigned long *last   = &d[P];
  unsigned long *first  = &d[0];

  *--last = x;
  while ( first < last ) {
    *first++ = 0UL;
  }
  if ( __overflow( x ) && P > 1) {
    *last-- &= no_sign_mask;
    *last = 1UL;
  }
}

template <int P>
Integer<P>& Integer<P>::operator =( long x )
{
  unsigned long *last   = &d[P];
  unsigned long *first  = &d[0];

  *--last = x;
  if ( x >= 0L ) {
    while ( first > last ) {
      *first++ = 0UL;
    }
  } else if ( P > 1 ) {
    *last &= no_sign_mask;
    *first++ = ULONG_MAX;
    while ( first < last ) {
      *first++ = LONG_MAX;
    }
  }

  return *this;
}

template <int P>
bool Integer<P>::operator <( const Integer& x ) const
{
  const unsigned long *last   = &d[P];
  const unsigned long *first  = &d[0];
  const unsigned long *second = &x.d[0];

  while ( first < last ) {
    if ( *first != *second ) {
      return *first < *second;
    }
    ++first;
    ++second;
  }
  return false;
}

template <int P>
bool Integer<P>::operator ==( const Integer& x ) const
{
  const unsigned long *last   = d+P;
  const unsigned long *first  = d;
  const unsigned long *second = x.d;

  while ( first < last ) {
    if ( *first++ != *second++ ) {
      return false;
    }
  }
  return true;
}

template <int P>
Integer<P>& Integer<P>::operator +=( const Integer<P>& x )
{
  unsigned long k = 0UL;
  unsigned long *last   = d+P;
  unsigned long *first  = d;
  const unsigned long *second = x.d+P;

  while ( first < --last ) {
    *last += *--second + k;
    k = __overflow( *last ) ? 1UL : 0UL;
    *last &= no_sign_mask; // clear sign bit
  }
  *last += *--second + k;
  return *this;
}

template <int P>
Integer<P>& Integer<P>::operator -=( const Integer<P>& x )
{
  unsigned long k = 1UL;
  unsigned long *last  = d+P;
  unsigned long *first = d;
  const unsigned long *second = x.d+P;

  while ( first < --last ) {
    *last += ULONG_MAX - *--second + k;
    k = __overflow( *last ) ? 0UL : 1UL;
    *last &= no_sign_mask; // clear sign bit
  }
  *last +=  ULONG_MAX - *--second + k;

  return *this;
}


template <int P>
Integer<P> Integer<P>::operator +( const Integer<P>& x ) const
{
  Integer r;
  unsigned long k = 0UL;
  unsigned long *current      = r.d+P;
  const unsigned long *last   = d+P;
  const unsigned long *first  = d;
  const unsigned long *second = x.d+P;

  while ( first < --last  ) {
    *--current = *last + *--second + k;
    k = __overflow( *current ) ? 1UL : 0UL;
    *current &= no_sign_mask; // clear sign bit
  }
  *--current = *last + *--second + k;

  return r;
}

template <int P>
Integer<P> Integer<P>::operator -( const Integer<P>& x ) const
{
  Integer r;
  unsigned long k = 1UL;
  unsigned long *current      = r.d+P;
  const unsigned long *last   = d+P;
  const unsigned long *first  = d;
  const unsigned long *second = x.d+P;

  while ( first < --last ) {
    *--current = ULONG_MAX - *--second + *last + k;
    k = __overflow( *current ) ? 0UL : 1UL;
    *current &= no_sign_mask; // clear sign bit
  }
  *--current = ULONG_MAX - *--second + *last + k;

  return r;
}

template <int P>
Integer<P> Integer<P>::operator *( const Integer<P>& x ) const
{
  if ( !( isNegative() || x.isNegative() ) ) {
    Integer<P> r;
    multiply_aux( x, r.d );
    return r;
  } else if ( isNegative() ) {
    Integer tmp1( *this );
    tmp1.radix_complement();
    if ( x.isNegative() ) {
      Integer tmp2( x );
      tmp2.radix_complement();
      tmp2.multiply_aux( tmp1, tmp2.d );
      return tmp2;
    }
    tmp1.multiply_aux( x, tmp1.d );
    tmp1.radix_complement();
    return tmp1;
  }
  Integer tmp2( x );
  tmp2.radix_complement();
  tmp2.multiply_aux( *this, tmp2.d );
  tmp2.radix_complement();
  return tmp2;
}

template <int P>
Integer<P>& Integer<P>::operator *=( const Integer<P>& x )
{

  if ( !( isNegative() || x.isNegative() ) ) {
    multiply_aux( x, d );
    return *this;
  } else if ( isNegative() ) {
    radix_complement();
    if ( x.isNegative() ) {
      Integer tmp2( x );
      tmp2.radix_complement();
      multiply_aux( tmp2, d );

      return *this;
    }
    multiply_aux( x, d );
    radix_complement();

    return *this;
  }
  Integer tmp2( x );
  tmp2.radix_complement();
  multiply_aux( tmp2, d );
  radix_complement();

  return *this;
}

template <int P>
void Integer<P>::multiply_aux( const Integer<P>& x, unsigned long *r ) const
{
  unsigned long w[P+P];
  unsigned long *wp = w + P;
  unsigned long *end = wp + P;
  fill( wp, end, 0UL ); // clear low digits
  unsigned long *_w = wp;
  const unsigned long *v = x.d + P; // v[m]
  while ( _w-- > w ) {
    if ( *--v == 0UL ) {
      *_w = 0UL;
      continue;
    }
    unsigned long k = 0UL;
    __longlong t;
    unsigned long *wl = _w + P; // w[n+j]
    const unsigned long *u = d + P; // u[n]
    while ( u-- > d ) {
      t.A = *u != 0UL ? ((long long)*u * *v + *wl + k) : ((long long)*wl + k);
      // next two lines are calculations t.A mod LONG_MAX
      k = (t.a.hi << 1) | (__overflow( t.a.lo ) ? 1UL : 0UL);
      *wl-- = t.a.lo & no_sign_mask; // with clear sign bit
    }
    *wl = k;
  }
  copy( wp, end, r ); // copy low half of digits
}

template <int P>
void Integer<P>::radix_complement()
{
  unsigned long *first = d;
  unsigned long *last = d + P;
  unsigned long k = 0UL;
  unsigned long second = 1UL;

  while ( first < --last ) {
    *last = ~*last;
    *last &= no_sign_mask;
    *last += second + k;
    second = 0UL;
    k = __overflow( *last ) ? 1UL : 0UL;
    *last &= no_sign_mask; // clear sign bit
  }
  *last = ~*last;
  *last += second + k;
}

template <int P>
Integer<P> Integer<P>::operator /( const Integer<P>& x ) const
{
  if ( !( isNegative() || x.isNegative() ) ) {
    Integer<P> r;
    divide_aux3( x, r.d );
    return r;
  } else if ( isNegative() ) {
    Integer tmp1( *this );
    tmp1.radix_complement();
    if ( x.isNegative() ) {
      Integer tmp2( x );
      tmp2.radix_complement();
      tmp2.divide_aux3( tmp1, tmp2.d );
      return tmp2;
    }
    tmp1.divide_aux3( x, tmp1.d );
    tmp1.radix_complement();
    return tmp1;
  }
  Integer tmp2( x );
  tmp2.radix_complement();
  tmp2.divide_aux3( *this, tmp2.d );
  tmp2.radix_complement();
  return tmp2;
}

template <int P>
Integer<P>& Integer<P>::operator /=( const Integer<P>& x )
{
  if ( !( isNegative() || x.isNegative() ) ) {
    divide_aux3( x, d );
    return *this;
  } else if ( isNegative() ) {
    radix_complement();
    if ( x.isNegative() ) {
      Integer tmp2( x );
      tmp2.radix_complement();
      divide_aux3( tmp2, d );

      return *this;
    }
    divide_aux3( x, d );
    radix_complement();

    return *this;
  }
  Inger tmp2( x );
  tmp2.radix_complement();
  divide_aux3( tmp2, d );
  radix_complement();

  return *this;
}

template <int P>
Integer<P> Integer<P>::operator %( const Integer<P>& x ) const
{
  if ( !( isNegative() || x.isNegative() ) ) {
    Integer r;
    divide_aux3( x, 0, r.d );
    return r;
  } else if ( isNegative() ) {
    Integer tmp1( *this );
    tmp1.radix_complement();
    if ( x.isNegative() ) {
      Integer tmp2( x );
      tmp2.radix_complement();
      tmp2.divide_aux3( tmp1, 0, tmp2.d );
      return tmp2;
    }
    tmp1.divide_aux3( x, 0, tmp1.d );
    tmp1.radix_complement();
    return tmp1;
  }
  Integer tmp2( x );
  tmp2.radix_complement();
  tmp2.divide_aux3( *this, 0, tmp2.d );
  tmp2.radix_complement();
  return tmp2;
}

template <int P>
Integer<P> Integer<P>::div( const Integer<P>& x, Integer<P>& r ) const
{
  if ( !( isNegative() || x.isNegative() ) ) {
    Integer res;
    divide_aux3( x, res.d, r.d );
    return res;
  } else if ( isNegative() ) {
    Integer tmp1( *this );
    tmp1.radix_complement();
    if ( x.isNegative() ) {
      Integer tmp2( x );
      tmp2.radix_complement();
      tmp2.divide_aux3( tmp1, tmp2.d, r.d );
      return tmp2;
    }
    tmp1.divide_aux3( x, tmp1.d, r.d );
    tmp1.radix_complement();
    return tmp1;
  }
  Integer tmp2( x );
  tmp2.radix_complement();
  tmp2.divide_aux3( *this, tmp2.d, r.d );
  tmp2.radix_complement();
  return tmp2;
}

template <int P>
Integer<P>& Integer<P>::div1( const Integer<P>& x, Integer<P>& r )
{
  if ( !( isNegative() || x.isNegative() ) ) {
    divide_aux3( x, d, r.d );
    return *this;
  } else if ( isNegative() ) {
    radix_complement();
    if ( x.isNegative() ) {
      Integer tmp2( x );
      tmp2.radix_complement();
      divide_aux3( tmp2, d, r.d );

      return *this;
    }
    divide_aux3( x, d, r.d );
    radix_complement();

    return *this;
  }
  Integer tmp2( x );
  tmp2.radix_complement();
  divide_aux3( tmp2, d, r.d );
  radix_complement();

  return *this;
}

/*
  This do division of number u_0 u_1 ... u_{P-1} on number v_0 (with single
  digit).
*/
template <int P>
unsigned long *Integer<P>::divide_aux( unsigned long *u, unsigned long v,
				       unsigned long *q ) const
{
  __longlong t;
  
  unsigned long *qi = q;
  unsigned long *uj = u;
  unsigned long *up = u + P;
  int k = 0;
  while ( uj < up ) {
    if ( *uj < v ) {
      if ( uj == up - 1 ) {
	// *uj is a rest (uj = u+P-1)
	break;
      }
      t.a.hi = *uj >> 1;
      t.a.lo = *(uj+1) | (*uj << (long_bits - 1));
      *qi++ = (unsigned long)(t.A / v);
      *(++uj) = (unsigned long)(t.A % v);
    } else {
      *qi++ = *uj / v;
      *uj %= v;
    }
  }

  return qi;
}

template <int P>
unsigned long *Integer<P>::divide_aux2( unsigned long *u, unsigned long *v0,
					unsigned long *q, int m, int n ) const
{
  unsigned long *v = v0 + 1;
  // D1
  // find hiest non-zero bit.
  int sh = 0;
  unsigned long tmp = hi_bit_mask;
  while ( (*v & tmp) == 0UL ) {
    tmp >>= 1;
    ++sh;
  }
  // multiply on d (sh)
  shl_aux( v, v+n, sh );
  shl_aux( u, u+n+m+1, sh );
  // D2
  unsigned long qt;
  unsigned long rt;
  __longlong t;
  unsigned long *uj = u;
  unsigned long *um = u+m+1;
  unsigned long *qj = q;
  while ( uj < um ) {
    // D3
    t.a.hi = *uj >> 1;   // t.A = *(u+j) * (LONG_MAX + 1) + *(u+j+1)
    t.a.lo = *(uj+1) | (*uj << (long_bits - 1));
    qt = *uj == *v ? LONG_MAX : (unsigned long) (t.A / *v);
    if ( qt == 0UL ) {
      *qj++ = qt; // D5
      ++uj; // D7
      continue;
    }
    rt = (unsigned long)(t.A % *v);
    t.a.hi = rt >> 1; // t.A = (*(u+j) * (LONG_MAX + 1) + + *(u+j+1) - qt * *v)
    t.a.lo = *(uj+2) | (rt << (long_bits - 1)); // * (LONG_MAX + 1) + *(u+j+2)
    if ( (long long)(*(v+1)) * (long long)qt > t.A ) {
      t.a.hi = *uj >> 1;
      t.a.lo = *(uj+1) | (*uj << (long_bits - 1));
      r= (unsigned long)(t.A % *v);
      t.a.hi = rt >> 1;
      t.a.lo = *(uj+2) | (rt << (long_bits - 1));
      if ( (long long)(*(v+1)) * (long long)(--qt) > t.A ) {
	qt--;
      }
    }
    // D4: u_j...u_j+n - v_1...v_n * qt
    unsigned long k = 0UL;
    unsigned long *uji = uj+n; // u[j+n]
    const unsigned long *vi = v+n; // v[n]
    while ( vi-- > v ) {
      t.A = (long long)*vi * (long long)qt;
      t.a.hi <<= 1;
      t.a.hi |= (__overflow( t.a.lo ) ? 1UL : 0UL);
      t.a.lo &= no_sign_mask;
      tmp = *uji + k;
      k = (__overflow( tmp ) ? ULONG_MAX : 0UL ) - t.a.hi;
      tmp &= no_sign_mask;
      tmp -= t.a.lo;
      k += (__overflow( tmp ) ? ULONG_MAX : 0UL);
      *uji-- = tmp & no_sign_mask;
    }
    *uji += k;

    *qj = qt; // D5
    if ( __overflow( *uji ) ) { // D6
      // begin radix complement in case t.A < 0
      unsigned long one = 1UL;

      k = 0UL;
      uji = u+n;
      while ( uj < uji ) {
	*uji = ~*uji;
	*uji &= no_sign_mask;
	*uji += one + k;
	one = 0UL;
	k = __overflow( *uji ) ? 1UL : 0UL;
	*uji-- &= no_sign_mask; // clear sign bit
      }
      *uji = ~*uji;
      *uji += one + k;
      // end radix complement
      --*qj;
      // add
      k = 0UL;
      uji = uj+n;
      vi = v+n; // ????

      while ( uj < uji ) {
	*uji += *--vi + k;
	k = __overflow( *uji ) ? 1UL : 0UL;
	*uji-- &= no_sign_mask; // clear sign bit
      }
      *uji += k;
      *uji &= no_sign_mask;
    }
    ++uj; // D7
    ++qj;
  }
  *v0 = 1UL << sh; // in v0 I return d: the rest is u_{m+1} ... u_{m+n} / d
  return qj;
}

template <int P>
void Integer<P>::divide_aux3( const Integer<P>& x, unsigned long *r,
			      unsigned long *modulo ) const
{
  unsigned long u[P+1];
  unsigned long v0[P+1];
  unsigned long *v = v0 + 1;
  *v0 = 0L;
  unsigned long q[P];
  unsigned long *qj;
  int n = P;
  int j = 0;
  while ( j < P && x.d[j] == 0 ) {
    n--;
    j++;
  }
  if ( n == 1 ) {
    copy( d, d + P, u );
    qj = divide_aux( u, x.d[P-1], q );
    if ( r ) {
      fill( r, copy_backward( q, qj, r+P ), 0UL );
    }
    if ( modulo ) {
      *(modulo+P-1) = *(u+P-1);
      fill( modulo, modulo+P-1, 0UL );
    }
    return;
  }
  copy( x.d + j, x.d + P, v ); // v of size n
  j = 0;
  int m = P;
  while ( j < P && d[j] == 0 ) {
    m--;
    j++;
  }
  m -= n;
  copy( d + j, d + P, u + 1 ); // u of size m + n + 1
  *u = 0UL;
  qj = divide_aux2( u, v0, q, m, n );
  if ( r ) {
    fill( r, copy_backward( q, qj, r+P ), 0UL );
  }
  if ( modulo ) {
    fill( v, v + P, 0UL );
    copy_backward( u+m+1, u+m+n+1, v+P );
    qj = divide_aux( v, *v0, q );
    fill( modulo, copy_backward( q, qj, modulo+P ), 0UL );
  }
}

/*
  This is shift left: << sh, sign bit to be taken into account!
  I expected that first < last!
*/
template <int P>
void Integer<P>::shl_aux( unsigned long *first, unsigned long *last,
			  int sh ) const
{
  if ( sh <= 0 ) {
    return;
  }
  int s_bit_sh = long_bits - 1;
  int s_bit_shr = 0;
  int shr = long_bits - sh;
  unsigned long tmp;
  unsigned long tmp2 = 0UL;
  unsigned long tmp3 = 0UL;

  while ( first < --last ) {
    // compress sign bit
    *last >>= s_bit_shr; 
    *last |= *(last-1) << s_bit_sh;
    // shift as array of unsigned
    tmp = *last >> shr;
    *last <<= sh;
    *last |= tmp2;
    tmp2 = tmp;
    // decompress sign bit
    tmp = *last >> s_bit_sh;
    *last <<= s_bit_shr;
    *last |= tmp3;
    tmp3 = tmp;
    *last &= no_sign_mask;

    ++s_bit_shr;
    --s_bit_sh;
  }
  // compress sign bit
  *last >>= s_bit_shr;
  // shift as array of unsigned
  *last <<= sh;
  *last |= tmp2;
  // decompress sign bit
  *last <<= s_bit_shr; 
  *last |= tmp3;
}

#endif
