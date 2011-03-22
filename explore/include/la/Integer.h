// -*- C++ -*- Time-stamp: <97/01/16 14:42:23 ptr>
#ifndef __LA_Integer_h
#define __LA_Integer_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <stdint.h>
#include <tr1/type_traits>
#include <string>
#include <iostream>
#include <algorithm>

namespace detail {
  
template <class _Tp>
struct make_extended
{
};

template <>
struct make_extended<signed char>
{
    typedef short type;
};

template <>
struct make_extended<signed short>
{ 
    typedef long type;
};

template <>
struct make_extended<signed long>
{
    typedef long long type;
};

// copy-paste from STLport

// [20.5.6.3] sign modifications

template <class _Tp>
struct make_signed
{
};

template <>
struct make_signed<char>
{
    typedef signed char type;
};

template <>
struct make_signed<signed char>
{
    typedef signed char type;
};

template <>
struct make_signed<unsigned char>
{
    typedef signed char type;
};

template <>
struct make_signed<short>
{
    typedef short type;
};

template <>
struct make_signed<unsigned short>
{
    typedef short type;
};

template <>
struct make_signed<int>
{
    typedef int type;
};

template <>
struct make_signed<unsigned int>
{
    typedef int type;
};

template <>
struct make_signed<long>
{
    typedef long type;
};

template <>
struct make_signed<unsigned long>
{
    typedef long type;
};

template <>
struct make_signed<long long>
{
    typedef long long type;
};

template <>
struct make_signed<unsigned long long>
{
    typedef long long type;
};

template <class _Tp>
struct make_unsigned
{
};

template <>
struct make_unsigned<char>
{
    typedef unsigned char type;
};

template <>
struct make_unsigned<signed char>
{
    typedef unsigned char type;
};

template <>
struct make_unsigned<unsigned char>
{
    typedef unsigned char type;
};

template <>
struct make_unsigned<short>
{
    typedef unsigned short type;
};

template <>
struct make_unsigned<unsigned short>
{
    typedef unsigned short type;
};

template <>
struct make_unsigned<int>
{
    typedef unsigned int type;
};

template <>
struct make_unsigned<unsigned int>
{
    typedef unsigned int type;
};

template <>
struct make_unsigned<long>
{
    typedef unsigned long type;
};

template <>
struct make_unsigned<unsigned long>
{
    typedef unsigned long type;
};

template <>
struct make_unsigned<long long>
{
    typedef unsigned long long type;
};

template <>
struct make_unsigned<unsigned long long>
{
    typedef unsigned long long type;
};

}

template <int P, class T > class la_int;
typedef la_int<4, long> Integer4;

template <int P, class T>
class la_int
{
  public:
    typedef typename detail::make_signed<T>::type base_type;
    typedef typename detail::make_unsigned<base_type>::type ubase_type;
    typedef typename detail::make_extended<base_type>::type long_base_type;
        
    la_int()
      { std::fill( d, d + P, 0 ); }
    
    la_int( base_type );
    la_int( const la_int& );

    la_int& operator =( const la_int& );
    la_int& operator =( base_type );
    
    la_int& operator +=( const la_int& );
    la_int& operator -=( const la_int& );
    la_int& operator *=( const la_int& );
    la_int& operator /=( const la_int& );
    la_int& div1( const la_int& divider, la_int& rest );
    
    la_int& operator ++()
      { return *this += 1; }
      
    la_int operator ++( int )
      {
        la_int tmp( *this );
        *this += 1;
        return tmp;
      }
      
    la_int& operator --()
      { return *this -= 1L; }
      
    la_int operator --( int )
      {
        la_int tmp( *this );
        *this -= 1;
        return tmp;
      }
      
    la_int& operator <<=( base_type s )
      {
        shl_aux( d, d+P, s );
        return *this;
      }

    la_int operator +( const la_int& x ) const
      { la_int r(*this); r += x; return r; }
    la_int operator -( const la_int& x ) const
      { la_int r(*this); r -= x; return r; }
    la_int operator *( const la_int& x ) const
      { la_int r(*this); r *= x; return r; }
    la_int operator /( const la_int& x ) const
      { la_int r(*this); r /= x; return r; }
    la_int operator %( const la_int& ) const;
  
    la_int operator -() const;
    la_int div( const la_int& divider, la_int& rest ) const;
    la_int gcd( const la_int& ) const;
    la_int operator <<( base_type s )
      {
        la_int tmp( *this );
        tmp.shl_aux( tmp.d, tmp.d+P, s );
        return tmp;
      }

    bool operator <( const la_int& ) const;
    bool operator ==( const la_int& ) const;
    
    bool operator !=( const la_int& x ) const
      { return !(*this == x); }
      
    bool operator <=( const la_int& x ) const
      { return ( (*this < x) || (*this == x) ); }
    
    bool operator >(const la_int& x ) const
      { return !(*this <= x); }

    bool operator >=(const la_int& x ) const
      { return !(*this < x); }
      
    la_int& assign( const std::string& ); // input from string
    operator std::string(); // output to string
    operator std::string() const;
    
  protected:

    static const ubase_type long_bits;
    static const ubase_type shift_first_to_hi;
    static const ubase_type sign_mask;
    static const ubase_type no_sign_mask;
    static const ubase_type hi_bit_mask;
    static const ubase_type UMAX;
    static const ubase_type MAX;    

    bool __overflow( ubase_type x ) const
      { return base_type(x) < 0; }
    void radix_complement();
    bool isNegative() const
      { return base_type(*d) < 0; }
    void multiply_aux( const la_int &, ubase_type * ) const;
    ubase_type *divide_aux( ubase_type *, ubase_type, ubase_type * ) const;
    void shl_aux( ubase_type *, ubase_type *, int ) const;
    void shr_aux( ubase_type *, ubase_type *, int ) const;
    void shr_aux2( ubase_type *, ubase_type * ) const;
    ubase_type *divide_aux2( ubase_type *, ubase_type *, ubase_type *,
			     int, int ) const;
    void divide_aux3( const la_int&, ubase_type *theResult,
		      ubase_type *theRest = 0 ) const;

    ubase_type d[P];

    union __longlong {
      long_base_type A;
      struct {
#ifdef _BIG_ENDIAN
        ubase_type hi;
        ubase_type lo;
#endif
#ifdef _LITTLE_ENDIAN
        ubase_type lo;
        ubase_type hi;
#endif
      } a;
    };

    void __load( __longlong& t, ubase_type hi, ubase_type lo ) const
      {
        t.a.hi = hi >> 1;
        t.a.lo = lo | (hi << shift_first_to_hi);
      }
};

template <int P, class T>
const typename la_int< P, T >::ubase_type la_int< P, T >::long_bits = 8 * sizeof(T);
template <int P, class T>
const typename la_int< P, T >::ubase_type la_int< P, T >::shift_first_to_hi = 8 * sizeof(T) - 1;
template <int P, class T>
const typename la_int< P, T >::ubase_type la_int< P, T >::sign_mask = (typename la_int< P, T >::ubase_type(1) << la_int< P, T >::shift_first_to_hi);
template <int P, class T>
const typename la_int< P, T >::ubase_type la_int< P, T >::no_sign_mask = (typename la_int< P, T >::ubase_type)(~la_int< P, T >::sign_mask);
template <int P, class T>
const typename la_int< P, T >::ubase_type la_int< P, T >::hi_bit_mask = la_int< P, T >::sign_mask >> 1;
template <int P, class T>
const typename la_int< P, T >::ubase_type la_int< P, T >::UMAX = ~typename la_int< P, T >::ubase_type(0);
template <int P, class T>
const typename la_int< P, T >::ubase_type la_int< P, T >::MAX = la_int< P, T >::no_sign_mask;

template <int P, class T>
la_int<P,T>::la_int( base_type x )
{
  ubase_type *last   = d + P;
  ubase_type *first  = d;

  *--last = x;
  if ( x >= 0 ) {
    while ( first < last ) {
      *first++ = 0;
    }
  } else if ( P > 1 ) {
    *last &= la_int<P,T>::no_sign_mask;
    *first++ = UMAX;
    while ( first < last ) {
      *first++ = MAX;
    }
  }
}

template <int P, class T>
la_int<P,T>& la_int<P,T>::operator =( base_type x )
{
  ubase_type *last   = d + P;
  ubase_type *first  = d;

  *--last = x;
  if ( x >= 0 ) {
    while ( first < last ) {
      *first++ = 0;
    }
  } else if ( P > 1 ) {
    *last &= no_sign_mask;
    *first++ = UMAX;
    while ( first < last ) {
      *first++ = MAX;
    }
  }

  return *this;
}

template <int P, class T>
la_int<P,T>::la_int( const la_int& x )
{
  std::copy( x.d, x.d + P, d );
}

template <int P, class T>
la_int<P,T>& la_int<P,T>::operator =( const la_int& x )
{
  std::copy( x.d, x.d + P, d );
  return *this;
}

template <int P, class T>
bool la_int<P,T>::operator <( const la_int& x ) const
{
  const ubase_type *last   = d + P;
  const ubase_type *first  = d;
  const ubase_type *second = x.d;

  while ( first < last ) {
    if ( *first != *second ) {
      return base_type(*first) < base_type(*second);
    }
    ++first;
    ++second;
  }
  return false;
}

template <int P, class T>
bool la_int<P,T>::operator ==( const la_int& x ) const
{
  const ubase_type *last   = d+P;
  const ubase_type *first  = d;
  const ubase_type *second = x.d;

  while ( first < last ) {
    if ( *first++ != *second++ ) {
      return false;
    }
  }
  return true;
}

template <int P, class T>
la_int<P,T>& la_int<P,T>::operator +=( const la_int& x )
{
  ubase_type k = 0UL;
  ubase_type *last   = d+P;
  ubase_type *first  = d;
  const ubase_type *second = x.d+P;

  while ( first < --last ) {
    *last += *--second + k;
    k = __overflow( *last ) ? 1 : 0;
    *last &= no_sign_mask; // clear sign bit
  }
  *last += *--second + k;
  return *this;
}

template <int P, class T>
la_int<P,T>& la_int<P,T>::operator -=( const la_int& x )
{
  ubase_type k = 1;
  ubase_type *last  = d + P;
  ubase_type *first = d;
  const ubase_type *second = x.d + P;

  while ( first < --last ) {
    *last += UMAX - *--second + k;
    k = __overflow( *last ) ? 0 : 1;
    *last &= no_sign_mask; // clear sign bit
  }
  *last +=  UMAX - *--second + k;

  return *this;
}

template <int P, class T>
la_int<P,T>& la_int<P,T>::operator *=( const la_int& x )
{
  if ( !( isNegative() || x.isNegative() ) ) {
    multiply_aux( x, d );
    return *this;
  } else if ( isNegative() ) {
    radix_complement();
    if ( x.isNegative() ) {
      la_int tmp2( x );
      tmp2.radix_complement();
      multiply_aux( tmp2, d );

      return *this;
    }
    multiply_aux( x, d );
    radix_complement();

    return *this;
  }
  la_int tmp2( x );
  tmp2.radix_complement();
  multiply_aux( tmp2, d );
  radix_complement();

  return *this;
}

template <int P, class T>
void la_int<P,T>::multiply_aux( const la_int& x, ubase_type *r ) const
{
  ubase_type w[P + P];
  ubase_type *wp = w + P;
  ubase_type *end = wp + P;
  std::fill( wp, end, 0UL ); // clear low digits
  ubase_type *_w = wp;
  const ubase_type *v = x.d + P; // v[m]
  while ( _w-- > w ) {
    if ( *--v == 0 ) {
      *_w = 0;
      continue;
    }
    ubase_type k = 0;
    __longlong t;
    ubase_type *wl = _w + P; // w[n+j]
    const ubase_type *u = d + P; // u[n]
    while ( u-- > d ) {
      t.A = *u != 0 ? ((long_base_type)*u * *v + *wl + k) : ((long_base_type)*wl + k);
      // next two lines are calculations t.A mod LONG_MAX
      k = (t.a.hi << 1) | (__overflow( t.a.lo ) ? 1 : 0);
      *wl-- = t.a.lo & no_sign_mask; // with clear sign bit
    }
    *wl = k;
  }
  std::copy( wp, end, r ); // copy low half of digits
}

template <int P, class T>
void la_int<P,T>::radix_complement()
{
  ubase_type *first = d;
  ubase_type *last = d + P;
  ubase_type k = 1;

  while ( first < --last ) {
    *last = (~*last & no_sign_mask) + k;
    k = __overflow( *last ) ? 1 : 0;
    *last &= no_sign_mask; // clear sign bit
  }
  *last = ~*last + k;
}

template <int P, class T>
la_int<P,T> la_int<P,T>::operator -() const
{
  la_int r( *this );

  r.radix_complement();

  return r;
}

template <int P, class T>
la_int<P,T>& la_int<P,T>::operator /=( const la_int& x )
{
  if ( !( isNegative() || x.isNegative() ) ) {
    divide_aux3( x, d );
    return *this;
  } else if ( isNegative() ) {
    radix_complement();
    if ( x.isNegative() ) {
      la_int tmp2( x );
      tmp2.radix_complement();
      divide_aux3( tmp2, d );

      return *this;
    }
    divide_aux3( x, d );
    radix_complement();

    return *this;
  }
  la_int tmp2( x );
  tmp2.radix_complement();
  divide_aux3( tmp2, d );
  radix_complement();

  return *this;
}

template <int P, class T>
la_int<P,T> la_int<P,T>::operator %( const la_int& x ) const
{
  if ( !( isNegative() || x.isNegative() ) ) {
    la_int r;
    divide_aux3( x, 0, r.d );
    return r;
  } else if ( isNegative() ) {
    la_int tmp1( *this );
    tmp1.radix_complement();
    if ( x.isNegative() ) {
      la_int tmp2( x );
      tmp2.radix_complement();
      tmp1.divide_aux3( tmp2, 0, tmp1.d );
      return tmp1;
    }
    tmp1.divide_aux3( x, 0, tmp1.d );
    tmp1.radix_complement();
    return tmp1;
  }
  la_int tmp2( x );
  tmp2.radix_complement();
  tmp2.divide_aux3( *this, 0, tmp2.d );
  tmp2.radix_complement();
  return tmp2;
}

template <int P, class T>
la_int<P,T> la_int<P,T>::div( const la_int& x, la_int& r ) const
{
  if ( !( isNegative() || x.isNegative() ) ) {
    la_int res;
    divide_aux3( x, res.d, r.d );
    return res;
  } else if ( isNegative() ) {
    la_int tmp1( *this );
    tmp1.radix_complement();
    if ( x.isNegative() ) {
      la_int tmp2( x );
      tmp2.radix_complement();
      tmp1.divide_aux3( tmp2, tmp1.d, r.d );
      return tmp1;
    }
    tmp1.divide_aux3( x, tmp1.d, r.d );
    tmp1.radix_complement();
    return tmp1;
  }
  la_int tmp2( x );
  tmp2.radix_complement();
  tmp2.divide_aux3( *this, tmp2.d, r.d );
  tmp2.radix_complement();
  return tmp2;
}

template <int P, class T>
la_int<P,T>& la_int<P,T>::div1( const la_int& x, la_int& r )
{
  if ( !( isNegative() || x.isNegative() ) ) {
    divide_aux3( x, d, r.d );
    return *this;
  } else if ( isNegative() ) {
    radix_complement();
    if ( x.isNegative() ) {
      la_int tmp2( x );
      tmp2.radix_complement();
      divide_aux3( tmp2, d, r.d );

      return *this;
    }
    divide_aux3( x, d, r.d );
    radix_complement();

    return *this;
  }
  la_int tmp2( x );
  tmp2.radix_complement();
  divide_aux3( tmp2, d, r.d );
  radix_complement();

  return *this;
}

/*
  This do division of number u_0 u_1 ... u_{P-1} on number v_0 (with single
  digit).
*/
template <int P, class T>
typename la_int<P,T>::ubase_type *
la_int<P,T>::divide_aux( typename la_int<P,T>::ubase_type *u,
                                 typename la_int<P,T>::ubase_type v,
                                 typename la_int<P,T>::ubase_type *q ) const
{
  __longlong t;
  
  ubase_type *qi = q;
  ubase_type *uj = u;
  ubase_type *up = u + P;
  int k = 0;
  while ( uj < up ) {
    if ( *uj < v ) {
      if ( uj == up - 1 ) {
        // *uj is a rest (uj = u+P-1)
        break;
      }
      __load( t, *uj, *(uj+1) );
      *qi++ = (ubase_type)(t.A / v);
      *(++uj) = (ubase_type)(t.A % v);
    } else {
      *qi++ = *uj / v;
      *uj %= v;
    }
  }

  return qi;
}

template <int P, class T>
typename la_int<P,T>::ubase_type *
la_int<P,T>::divide_aux2( typename la_int<P,T>::ubase_type *u,
                                  typename la_int<P,T>::ubase_type *v,
                                  typename la_int<P,T>::ubase_type *q,
                                  int m, int n ) const
{
  // D2
  ubase_type qt;
  ubase_type rt;
  __longlong t;
  ubase_type *uj = u;
  ubase_type *um = u+m+1;
  ubase_type *qj = q;
  ubase_type tmp;
  while ( uj < um ) {
    // D3: calculate qt
    __load( t, *uj, *(uj+1) ); // t.A = *(u+j) * (LONG_MAX + 1) + *(u+j+1)
    qt = (*uj >= *v) ? MAX : (ubase_type) (t.A / (long_base_type)*v);
    if ( qt == 0 ) { // other loop part don't need if qt == 0
      *qj++ = qt; // D5
      ++uj;       // D7
      continue;
    }

    t.A %= (long_base_type)*v;
    t.A <<= shift_first_to_hi; // t.A = (u_j * b + u_j+1 - qt* v_1) * b
    t.A += (long_base_type)*(uj+2) - (long_base_type)(*(v+1)) * (long_base_type)qt;
    if ( t.A < 0LL ) { // v_2*qt > (u_j*b + u_{j+1}) mod v_1?
      t.A += ((long_base_type)*v << shift_first_to_hi) + (long_base_type)(*(v+1));
      qt--;
      if ( t.A < 0 ) { // v_2*qt > (u_j*b + u_{j+1}) mod v_1 + v_1*b?
        qt--;
      }
    }
    
    // D4: u_j...u_{j+n} - v_1...v_n * qt
    ubase_type k = 0;
    ubase_type *uji = uj+n; // u[j+n]
    const ubase_type *vi = v+n; // v[n]
    while ( vi-- > v ) {
      t.A = (long_base_type)*vi * (long_base_type)qt;
      t.a.hi <<= 1;
      t.a.hi |= (__overflow( t.a.lo ) ? 1 : 0);
      tmp = *uji + k;
      k = (__overflow( tmp ) ? UMAX : 0 ) - t.a.hi;
      tmp &= no_sign_mask;
      tmp -= t.a.lo & no_sign_mask;
      k += (__overflow( tmp ) ? UMAX : 0);
      *uji-- = tmp & no_sign_mask;
    }
    *uji += k;
    
    if ( __overflow( *uji ) ) {
      // radix complement (part of step D4)
      k = 1;
      uji = u+n;
      while ( uj < uji ) {
        *uji = (~*uji & no_sign_mask) + k;
        k = __overflow( *uji ) ? 1 : 0;
        *uji-- &= no_sign_mask; // clear sign bit
      }
      *uji = ~*uji + k;
      // radix complement done, decrease *qj
      --qt; // D6: compensate addition
      k = 0;
      uji = uj+n;
      vi = v+n;

      while ( uj < uji ) {
        *uji += *--vi + k;
        k = __overflow( *uji ) ? 1 : 0;
        *uji-- &= no_sign_mask; // clear sign bit
      }
      *uji += k;
      *uji &= no_sign_mask;
    }
    *qj++ = qt; // D5
    ++uj;       // D7
  }
  // The rest is u_{m+1} ... u_{m+n}
  return qj;
}

template <int P, class T>
void la_int<P,T>::divide_aux3( const la_int& x, ubase_type *r,
			     ubase_type *modulo ) const
{
  ubase_type u[P+1];
  ubase_type v[P+1];
  ubase_type q[P];
  ubase_type *qj;
  int n = P;
  int j = 0;
  while ( j < P && x.d[j] == 0 ) {
    n--;
    j++;
  }
  
  if ( n == 1 ) {
    std::copy( d, d + P, u );
    qj = divide_aux( u, x.d[P-1], q );
    if ( r ) {
      std::fill( r, std::copy_backward( q, qj, r+P ), 0 );
    }
    if ( modulo ) {
      *(modulo+P-1) = *(u+P-1);
      std::fill( modulo, modulo+P-1, 0 );
    }
    return;
  }
  
  std::copy( x.d + j, x.d + P, v ); // v of size n
  j = 0;
  int m = P;
  while ( j < P && d[j] == 0 ) {
    m--;
    j++;
  }

  m -= n;
  std::copy( d + j, d + P, u + 1 ); // u of size m + n + 1
  *u = 0UL;
  // D1: normalisation (v_1 >= floor( b/2 ), b here LONG_MAX + 1)
  // find hiest non-zero bit.
  int sh = 0;
  ubase_type tmp = hi_bit_mask;
  while ( (*v & tmp) == 0 ) {
    tmp >>= 1;
    ++sh;
  }
  if ( sh ) { // multiply on d (sh)
    shl_aux( v, v+n, sh );
    shl_aux( u, u+n+m+1, sh );
  }
  
  qj = divide_aux2( u, v, q, m, n );
  if ( r ) {
    std::fill( r, std::copy_backward( q, qj, r+P ), 0 );
  }
  if ( modulo ) {
    std::fill( v, v + P, 0 );
    std::copy_backward( u+m+1, u+m+n+1, v+P );
    qj = divide_aux( v, 1UL << sh, q );
    std::fill( modulo, std::copy_backward( q, qj, modulo+P ), 0 );
  }
}

/*
  This is shift left: << sh, sign bit to be taken into account!
  I expected that first < last and non-zero sh!
*/
template <int P, class T>
void la_int<P,T>::shl_aux( ubase_type *first, ubase_type *last, int sh ) const
{
  int s_bit_sh = long_bits - 1;
  int s_bit_shr = 0;
  int shr = long_bits - sh;
  ubase_type tmp;
  ubase_type tmp2 = 0;
  ubase_type tmp3 = 0;

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

template <int P, class T>
void la_int<P,T>::shr_aux( ubase_type *first, ubase_type *last,
			  int sh ) const
{
  int shr = long_bits - sh - 1;

  while ( first < --last ) {
    *last >>= sh;
    *last |= *(last-1) << shr;
    *last &= no_sign_mask;
  }
  *last >>= sh;
}

template <int P, class T>
void la_int<P,T>::shr_aux2( ubase_type *first, ubase_type *last ) const
{
  int shr = long_bits - 2;

  while ( first < --last ) {
    *last >>= 1;
    *last |= *(last-1) << shr;
    *last &= no_sign_mask;
  }
  ubase_type s = *last & sign_mask;
  *last >>= 1;
  *last |= s;
}

template <int P, class T>
la_int<P,T> la_int<P,T>::gcd( const la_int& x ) const
{
  la_int u( *this );
  la_int v( x );
  la_int t;
  if ( u < 0 ) {
    u.radix_complement();
  }
  if ( v < 0 ) {
    v.radix_complement();
  }

  int k = 0;
  while ( !(u.d[P-1] & 1) && !(v.d[P-1] & 1) ) {
    shr_aux2( u.d, u.d + P );
    shr_aux2( v.d, v.d + P );
    k++;
  }
  if ( u.d[P-1] & 1 ) {
    t = v;
    t.radix_complement();
  } else {
    t = u;
    shr_aux2( t.d, t.d + P );
  }
  while ( t != 0 ) {
    while ( (t.d[P-1] & 1) == 0 ) {
      shr_aux2( t.d, t.d + P );
    }
    if ( !(*t.d & sign_mask) ) {
      u = t;
    } else {
      v = t;
      v.radix_complement();
    }
    t = u - v;
  }
  if ( k != 0 ) {
    shl_aux( u.d, u.d+P, k );
  }
  return u;
}

template <int P, class T>
la_int<P,T>& la_int<P,T>::assign( const std::string& s )
{
  std::string::size_type f = s.find_first_not_of( " \t\n" );

  *this = 0L;
  la_int ten( 10 );
  bool flag = true;
  if ( s[f] == '+' ) {
    ++f;
  } else if ( s[f] == '-' ) {
    ++f;
    flag = false;
  }
  
  std::string::size_type p = s.find_first_not_of( "0123456789", f );
  std::string::const_iterator end = p != std::string::npos ? s.begin() + p : s.end();
  std::string::const_iterator ch  = s.begin() + f;

  while ( ch != end ) {
    *this *= ten;
    *this += long( *ch++ - '0' );
  }
  if ( !flag ) {
    radix_complement();
  }

  return *this;
}

template <int P, class T>
la_int<P,T>::operator std::string()
{
  std::string s;
  la_int ten( 10 );
  la_int n;
  la_int tmp( *this );
  bool flag = false;
  if ( *tmp.d & sign_mask ) {
    flag = true;
    tmp.radix_complement();
  }

  do {
    tmp.div1( ten, n );
    s.append( 1, char( '0' + n.d[P-1] ) ); // n is a tmp % 10
  } while ( tmp != 0L );
  if ( flag ) {
    s.append( "-" );
  }
  
  std::reverse( s.begin(), s.end() );
  return s;
}

template <int P, class T>
la_int<P,T>::operator std::string() const
{
  std::string s;
  la_int ten( 10 );
  la_int n;
  la_int tmp( *this );
  bool flag = false;
  if ( *tmp.d & sign_mask ) {
    flag = true;
    tmp.radix_complement();
  }

  do {
    tmp.div1( ten, n );
    s.append( 1, char( '0' + n.d[P-1] ) ); // n is a tmp % 10
  } while ( tmp != 0L );
  if ( flag ) {
    s.append( '-' );
  }
  
  std::reverse( s.begin(), s.end() );
  return s;
}

template <int P, class T>
la_int<P,T> abs( const la_int<P,T>& x)
{
  return (x >= 0 ? x : - x);
}

#endif // __LA_Integer_h
