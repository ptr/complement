// -*- C++ -*- Time-stamp: <07/08/03 08:59:36 ptr>

/*
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __misc_type_traits_h
#define __misc_type_traits_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#if !defined(STLPORT) /* || (_STLPORT_VERSION < 50200) */

// libstdc++ v3, timestamp 20050519 (3.4.4) has __type_traits,
// libstdc++ v3, timestamp 20060306 (3.4.6) has __type_traits,
// while libstdc++ v3, 20050921 (4.0.2) not; use libstdc++ instead
# if !defined(__GLIBCXX__) || (defined(__GNUC__) && (__GNUC__ < 4))

namespace std {

namespace tr1 {

template <class _Tp, _Tp __v>
struct integral_constant
{
    static const _Tp                    value = __v;
    // enum { value = __v };

    typedef _Tp                         value_type;
    typedef integral_constant<_Tp, __v> type;
};

typedef integral_constant<bool, true>   true_type;
typedef integral_constant<bool, false>  false_type;

#define  __SPEC_(C,T,B)               \
template <>                           \
struct C<T> :                         \
    public integral_constant<bool, B> \
{ }

#define __SPEC_FULL(C,T,B) \
__SPEC_(C,T,B);            \
__SPEC_(C,const T,B);      \
__SPEC_(C,volatile T,B);   \
__SPEC_(C,const volatile T,B)

#define  __SPEC_1(C,T,B)              \
template <class _Tp>                  \
struct C<T> :                         \
    public integral_constant<bool, B> \
{ }

#define __SPEC_FULL1(C,T,B) \
__SPEC_1(C,T,B);            \
__SPEC_1(C,const T,B);      \
__SPEC_1(C,volatile T,B);   \
__SPEC_1(C,const volatile T,B)

template <class _Tp>
struct is_void :
        public false_type
{ };

__SPEC_FULL(is_void,bool,true);

template <class _Tp>
struct is_integral :
    public false_type
{ };

__SPEC_FULL(is_integral,bool,true);
__SPEC_FULL(is_integral,char,true);
__SPEC_FULL(is_integral,signed char,true);
__SPEC_FULL(is_integral,unsigned char,true);
__SPEC_FULL(is_integral,wchar_t,true);
__SPEC_FULL(is_integral,short,true);
__SPEC_FULL(is_integral,unsigned short,true);
__SPEC_FULL(is_integral,int,true);
__SPEC_FULL(is_integral,unsigned int,true);
__SPEC_FULL(is_integral,long,true);
__SPEC_FULL(is_integral,unsigned long,true);
__SPEC_FULL(is_integral,long long,true);
__SPEC_FULL(is_integral,unsigned long long,true);

template <class _Tp>
struct is_floating_point :
    public false_type
{ };

__SPEC_FULL(is_floating_point,float,true);
__SPEC_FULL(is_floating_point,double,true);
__SPEC_FULL(is_floating_point,long double,true);

template <class _Tp>
struct is_arithmetic :
    public integral_constant<bool, (is_integral<_Tp>::value
                                    || is_floating_point<_Tp>::value)>
{ };

template <class _Tp>
struct is_pointer :
    public false_type
{ };

__SPEC_FULL1(is_pointer,_Tp *,true);

template <class _Tp>
struct is_scalar :
    public integral_constant<bool, (is_arithmetic<_Tp>::value
                                    /* || is_enum<_Tp>::value */
                                    || is_pointer<_Tp>::value
                                    /* || is_member_pointer<_Tp>::value */ )>
{ };

template <class _Tp>
struct remove_all_extents
{
    typedef _Tp type;
};

template <class _Tp, std::size_t _Size>
struct remove_all_extents<_Tp[_Size]>
{
    typedef typename remove_all_extents<_Tp>::type type;
};

template<typename _Tp>
struct remove_all_extents<_Tp[]>
{
    typedef typename remove_all_extents<_Tp>::type type;
};

template <class _Tp>
struct is_pod :
    public integral_constant<bool, (is_void<_Tp>::value
                                    || is_scalar<typename remove_all_extents<_Tp>::type>::value)>
{ };

#undef __SPEC_FULL
#undef __SPEC_
#undef __SPEC_FULL1
#undef __SPEC_1

} // namespace tr1

} // namespace std

# else // __GLIBCXX__ && (__GNUC__ >= 4)
#  include <tr1/type_traits>
# endif

#else // STLPORT
# include <type_traits>
#endif

#endif // __misc_type_traits_h

