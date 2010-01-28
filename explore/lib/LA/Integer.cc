// -*- C++ -*- Time-stamp: <96/04/09 16:13:40 ptr>

#include <limits>
#include <LA/Integer.h>

#include <algorithm>

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

unsigned long la_int_traits::long_bits = sizeof( unsigned long ) * 8;
unsigned long la_int_traits::shift_first_to_hi = la_int_traits::long_bits - 1;
unsigned long la_int_traits::sign_mask =
                                      1UL << la_int_traits::shift_first_to_hi;
unsigned long la_int_traits::no_sign_mask = ~la_int_traits::sign_mask;
unsigned long la_int_traits::hi_bit_mask = la_int_traits::sign_mask >> 1;
unsigned long la_int_traits::UMAX = ULONG_MAX;
unsigned long la_int_traits::MAX = LONG_MAX;

unsigned long la_char_traits::long_bits = sizeof( unsigned char ) * 8;
unsigned long la_char_traits::shift_first_to_hi = la_char_traits::long_bits - 1;
unsigned long la_char_traits::sign_mask =
                                      1UL << la_char_traits::shift_first_to_hi;
unsigned long la_char_traits::no_sign_mask = ~la_char_traits::sign_mask;
unsigned long la_char_traits::hi_bit_mask = la_char_traits::sign_mask >> 1;
unsigned long la_char_traits::UMAX = UCHAR_MAX;
unsigned long la_char_traits::MAX = CHAR_MAX;

unsigned long la_short_traits::long_bits = sizeof( unsigned short ) * 8;
unsigned long la_short_traits::shift_first_to_hi = la_short_traits::long_bits - 1;
unsigned long la_short_traits::sign_mask =
                                      1UL << la_short_traits::shift_first_to_hi;
unsigned long la_short_traits::no_sign_mask = ~la_short_traits::sign_mask;
unsigned long la_short_traits::hi_bit_mask = la_short_traits::sign_mask >> 1;
unsigned long la_short_traits::UMAX = USHRT_MAX;
unsigned long la_short_traits::MAX = SHRT_MAX;
