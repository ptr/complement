// -*- C++ -*- Time-stamp: <2011-04-29 19:45:34 ptr>

/*
 * Copyright (c) 2007-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 * Should be close to JTC1/SC22/WG21 C++ 0x working draft
 * [http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/2008/n2723.pdf]
 */

#ifndef __misc_type_traits_h
#define __misc_type_traits_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#if !defined(STLPORT) || (_STLPORT_VERSION < 0x520)

// libstdc++ v3, timestamp 20050519 (3.4.4) has __type_traits,
// libstdc++ v3, timestamp 20060306 (3.4.6) has __type_traits,
// while libstdc++ v3, 20050921 (4.0.2) not; use libstdc++ instead
# if /* defined(STLPORT) || */ !defined(__GNUC__) || (defined(__GNUC__) && (__GNUC__ < 4) ) /* !defined(__GLIBCXX__) || (defined(__GNUC__) && (__GNUC__ < 4)) */

namespace std {

namespace detail {

struct __select_types
{
    typedef char __t1;
    struct __t2
    {
        char __two[2];
    };
};

template <class _Tp>
struct __instance :
    public __select_types
{
  private:
    template <class _Up>
    static __t1 __test(_Up(*)[1]);

    template <class>
    static __t2 __test(...);
    
  public:
#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
    static const bool __value;
#else
    static const bool __value = sizeof(__test<_Tp>(0)) == sizeof(__select_types::__t1);
#endif

};

#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
template <class _Tp>
const bool __instance<_Tp>::__value = sizeof(__instance<_Tp>::__test<_Tp>(0)) == sizeof(__select_types::__t1);
#endif

template <class T>
struct __uoc_aux : // union or class
    public __select_types
{
  private:
    template <class _Up>
    static __t1 __test( int _Up::* );

    template <class>
    static __t2 __test(...);
    
  public:
#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
    static const bool __value;
#else
    static const bool __value = sizeof(__test<T>(0)) == sizeof(__select_types::__t1);
#endif
};

#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
template <class T>
const bool __uoc_aux<T>::__value = sizeof(__uoc_aux<T>::__test<T>(0)) == sizeof(__select_types::__t1);
#endif

template <class T>
class __empty
{ };

template <class T, bool B>
class __inheritance_aux
{};

template <class T>
class __inheritance_aux<T,true> :
    public T
{
  public:
    virtual ~__inheritance_aux()
      { }
};

#if 0
template <class T, bool B>
struct __virtual_aux
{
  public:
#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
    static const bool __value;
#else
    static const bool __value = B ? (sizeof(__inheritance_aux<T,B>) == sizeof(T)) : false;
#endif
};

#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
template <class T, bool B>
const bool __virtual_aux<T,B>::__value = B ? (sizeof(__inheritance_aux<T,B>) == sizeof(T)) : false;
#endif
#endif

} // namespace detail

template <class _Tp, _Tp __v>
struct integral_constant
{
    static const _Tp                    value = __v;
    // enum { value = __v }; ?

    typedef _Tp                         value_type;
    typedef integral_constant<_Tp, __v> type;
};

typedef integral_constant<bool, true>   true_type;
typedef integral_constant<bool, false>  false_type;

namespace detail {

template <typename _Tp>
struct __is_union_or_class :
    public integral_constant<bool, __uoc_aux<_Tp>::__value>
{ };

#if 0
template<typename _Tp>
struct __is_vtbl : // has virtual table?
    public integral_constant<bool, __virtual_aux<_Tp,__is_union_or_class<_Tp>::value >::__value>
{ };
#endif

template <typename _Tp>
struct __is_vtbl : // has virtual table?
    public integral_constant<bool, __is_union_or_class<_Tp>::value ? (sizeof(__inheritance_aux<_Tp,__is_union_or_class<_Tp>::value>) == sizeof(_Tp)) : false >
{ };

} // namespace detail

#define  __SPEC_(C,T,B)               \
template <>                           \
struct C<T> :                         \
    public integral_constant<bool, B> \
{ }

#define __CV_SPEC(C,T,B) \
__SPEC_(C,T,B);            \
__SPEC_(C,const T,B);      \
__SPEC_(C,volatile T,B);   \
__SPEC_(C,const volatile T,B)

#define  __SPEC_1(C,T,B)              \
template <class _Tp>                  \
struct C<T> :                         \
    public integral_constant<bool, B> \
{ }

#define __CV_SPEC_1(C,T,B) \
__SPEC_1(C,T,B);            \
__SPEC_1(C,T const,B);      \
__SPEC_1(C,T volatile,B);   \
__SPEC_1(C,T const volatile,B)

#define  __SPEC_2(C,T,B)              \
template <class _Tp1, class _Tp2>     \
struct C<T> :                         \
    public integral_constant<bool, B> \
{ }

#define __CV_SPEC_2(C,T,B) \
__SPEC_2(C,T,B);            \
__SPEC_2(C,T const,B);      \
__SPEC_2(C,T volatile,B);   \
__SPEC_2(C,T const volatile,B)

// [20.5.4.1] primary type categories:

template <class _Tp>
struct is_void :
    public false_type
{ };

template <>
struct is_void<void> :
    public true_type
{ };

template <class _Tp>
struct is_integral :
    public false_type
{ };

__CV_SPEC(is_integral,bool,true);
__CV_SPEC(is_integral,char,true);
__CV_SPEC(is_integral,signed char,true);
__CV_SPEC(is_integral,unsigned char,true);
__CV_SPEC(is_integral,wchar_t,true);
__CV_SPEC(is_integral,short,true);
__CV_SPEC(is_integral,unsigned short,true);
__CV_SPEC(is_integral,int,true);
__CV_SPEC(is_integral,unsigned int,true);
__CV_SPEC(is_integral,long,true);
__CV_SPEC(is_integral,unsigned long,true);
__CV_SPEC(is_integral,long long,true);
__CV_SPEC(is_integral,unsigned long long,true);

template <class _Tp>
struct is_floating_point :
    public false_type
{ };

__CV_SPEC(is_floating_point,float,true);
__CV_SPEC(is_floating_point,double,true);
__CV_SPEC(is_floating_point,long double,true);

template <class _Tp>
struct is_array :
    public false_type
{ };

template <class _Tp, std::size_t _Sz>
struct is_array<_Tp[_Sz]> :
    public true_type
{ };

template <class _Tp>
struct is_array<_Tp[]> :
    public true_type
{ };

template <class _Tp>
struct is_pointer :
    public false_type
{ };

__CV_SPEC_1(is_pointer,_Tp *,true);

template <class _Tp>
struct is_lvalue_reference :
    public false_type
{ };

template <class _Tp>
struct is_lvalue_reference<_Tp&> :
    public true_type
{ };

template <class _Tp>
struct is_rvalue_reference :
    public false_type
{ };

// template <class _Tp>
// struct is_rvalue_reference<_Tp&&> :
//     public true_type
// { };

template <class _Tp>
struct is_reference :
    public integral_constant<bool, is_lvalue_reference<_Tp>::value || is_rvalue_reference<_Tp>::value>
{ };

template <class _Tp>
struct is_function :
    public integral_constant<bool, !(detail::__instance<_Tp>::__value
                                     || detail::__is_union_or_class<_Tp>::value
                                     || is_lvalue_reference<_Tp>::value
                                     || is_void<_Tp>::value)>
{ };

template <class _Tp>
struct is_member_object_pointer :
    public false_type
{ };

// _SPEC_FULL2(is_member_object_pointer, _Tp1 _Tp2::*,!is_function<_Tp1>::value);

template <class _Tp1, class _Tp2>
struct is_member_object_pointer<_Tp1 _Tp2::*> :
    public integral_constant<bool, !is_function<_Tp1>::value>
{ };

template <class _Tp1, class _Tp2>
struct is_member_object_pointer<_Tp1 _Tp2::* const> :
    public integral_constant<bool, !is_function<_Tp1>::value>
{ };

template <class _Tp1, class _Tp2>
struct is_member_object_pointer<_Tp1 _Tp2::* volatile> :
    public integral_constant<bool, !is_function<_Tp1>::value>
{ };

template <class _Tp1, class _Tp2>
struct is_member_object_pointer<_Tp1 _Tp2::* const volatile> :
    public integral_constant<bool, !is_function<_Tp1>::value>
{ };

template <class _Tp>
struct is_member_function_pointer :
    public false_type
{ };

// _SPEC_FULL2(is_member_function_pointer,_Tp1 _Tp2::*,is_function<_Tp1>::value);

template <class _Tp1, class _Tp2>
struct is_member_function_pointer<_Tp1 _Tp2::*> :                         
    public integral_constant<bool, is_function<_Tp1>::value> 
{ };

template <class _Tp1, class _Tp2>
struct is_member_function_pointer<_Tp1 _Tp2::* const> :
    public integral_constant<bool, is_function<_Tp1>::value>
{ };

template <class _Tp1, class _Tp2>
struct is_member_function_pointer<_Tp1 _Tp2::* volatile> :
    public integral_constant<bool, is_function<_Tp1>::value>
{ };

template <class _Tp1, class _Tp2>
struct is_member_function_pointer<_Tp1 _Tp2::* const volatile> :
    public integral_constant<bool, is_function<_Tp1>::value>
{ };

template <class _Tp>
struct is_member_pointer :
    public integral_constant<bool, (is_member_object_pointer<_Tp>::value || is_member_function_pointer<_Tp>::value)>
{ };

// [20.5.4.2] composite type categories

// is_reference see above

template <class _Tp>
struct is_arithmetic :
    public integral_constant<bool, (is_integral<_Tp>::value || is_floating_point<_Tp>::value)>
{ };

template <class _Tp>
struct is_fundamental :
    public integral_constant<bool, (is_arithmetic<_Tp>::value || is_void<_Tp>::value)>
{ };

// [20.5.4.1] primary type categories (continued):

template <class _Tp>
struct is_enum :
    public integral_constant<bool, !(is_fundamental<_Tp>::value
                                     || is_array<_Tp>::value
                                     || is_pointer<_Tp>::value
                                     || is_lvalue_reference<_Tp>::value
                                     || is_member_pointer<_Tp>::value
                                     || is_function<_Tp>::value
                                     || detail::__is_union_or_class<_Tp>::value) >
{ };

template <class T>
struct is_union
{ };

template <class T>
struct is_class
{ };

// is_function (above)

// [20.5.4.2] composite type categories (continued)

// is_arithmetic (above)
// is_fundamental (above)

template <class _Tp>
struct is_object :
    public integral_constant<bool, (is_arithmetic<_Tp>::value ||
                                    is_array<_Tp>::value ||
                                    is_pointer<_Tp>::value ||
                                    is_member_pointer<_Tp>::value ||
                                    detail::__is_union_or_class<_Tp>::value)>
{ };

template <class _Tp>
struct is_scalar :
    public integral_constant<bool, (is_arithmetic<_Tp>::value
                                    || is_enum<_Tp>::value
                                    || is_pointer<_Tp>::value
                                    || is_member_pointer<_Tp>::value)>
{ };

template <class _Tp>
struct is_compound :
    public integral_constant<bool, !is_fundamental<_Tp>::value>
{ };

// is_member_pointer

// [20.5.4.3] type properties:

template <class _Tp>
struct is_const :
    public false_type
{ };

template <class _Tp>
struct is_const<_Tp const> :
    public true_type
{ };

template <class _Tp>
struct is_volatile :
    public false_type
{ };

template <class _Tp>
struct is_volatile<_Tp volatile> :
    public true_type
{ };


// [20.5.6.4] array modifications:

template <class _Tp>
struct remove_extent
{
    typedef _Tp type;
};

template <class _Tp, std::size_t _Sz>
struct remove_extent<_Tp[_Sz]>
{
    typedef _Tp type;
};

template <class _Tp>
struct remove_extent<_Tp[]>
{
    typedef _Tp type;
};

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

// [20.5.4.3] type properties (continued):

template <class _Tp>
struct is_trivial :
    public integral_constant<bool, (is_void<_Tp>::value
                                    || is_scalar<typename remove_all_extents<_Tp>::type>::value)>
{ };

template <class _Tp>
struct is_standard_layout :
    public integral_constant<bool, (is_void<_Tp>::value
                                    || is_scalar<typename remove_all_extents<_Tp>::type>::value)>
{ };

template <class _Tp>
struct is_pod :
    public integral_constant<bool, (is_void<_Tp>::value
                                    || is_scalar<typename remove_all_extents<_Tp>::type>::value)>
{ };

template<typename _Tp>
struct is_empty
    : public integral_constant<bool, (detail::__is_union_or_class<_Tp>::value
                                      && (sizeof(detail::__empty<_Tp>) == sizeof(_Tp)))>
{ };

// is_polimorphic
// is_abstract

template <class _Tp>
struct has_trivial_default_constructor :
    public integral_constant<bool, is_pod<_Tp>::value>
{ };

template <class _Tp>
struct has_trivial_copy_constructor :
    public integral_constant<bool, is_pod<_Tp>::value>
{ };

template <class _Tp>
struct has_trivial_assign :
    public integral_constant<bool, is_pod<_Tp>::value>
{ };

template <class _Tp>
struct has_trivial_destructor :
    public integral_constant<bool, is_pod<_Tp>::value>
{ };

template <class _Tp>
struct has_nothrow_default_constructor :
    public integral_constant<bool, is_pod<_Tp>::value>
{ };

template <class _Tp>
struct has_nothrow_copy_constructor :
    public integral_constant<bool, is_pod<_Tp>::value>
{ };

template <class _Tp>
struct has_nothrow_assign :
    public integral_constant<bool, is_pod<_Tp>::value>
{ };

template <class _Tp>
struct has_virtual_destructor :
    public false_type
{ };

template <class _Tp>
struct is_signed :
    public false_type
{ };

__CV_SPEC(is_signed,signed char,true);
__CV_SPEC(is_signed,short,true);
__CV_SPEC(is_signed,int,true);
__CV_SPEC(is_signed,long,true);
__CV_SPEC(is_signed,long long,true);

template <class _Tp>
struct is_unsigned :
    public false_type
{ };

__CV_SPEC(is_unsigned,unsigned char,true);
__CV_SPEC(is_unsigned,unsigned short,true);
__CV_SPEC(is_unsigned,unsigned int,true);
__CV_SPEC(is_unsigned,unsigned long,true);
__CV_SPEC(is_unsigned,unsigned long long,true);

// alignment_of
// rank
// extent

// [20.5.5] type relations:

template <class _Tp1, class _Tp2>
struct is_same :
    public false_type
{ };

template <class _Tp>
struct is_same<_Tp, _Tp> :
    public true_type
{ };

// is_base_of
// is_convertible

// [20.5.6.1] const-volatile modifications

template <class _Tp>
struct remove_const
{
    typedef _Tp type;
};

template <class _Tp>
struct remove_const<_Tp const>
{
    typedef _Tp type;
};
  
template <class _Tp>
struct remove_volatile
{
    typedef _Tp type;
};

template <class _Tp>
struct remove_volatile<_Tp volatile>
{
    typedef _Tp type;
};

template <class _Tp>
struct remove_cv
{
    typedef typename remove_const<typename remove_volatile<_Tp>::type>::type type;
};
  
template <class _Tp>
struct add_const
{
    typedef _Tp const type;
};

template <class _Tp>
struct add_const<_Tp const>
{
    typedef _Tp const type;
};
  
template <class _Tp>
struct add_volatile
{
    typedef _Tp volatile type;
};
  
template <class _Tp>
struct add_volatile<_Tp volatile>
{
    typedef _Tp volatile type;
};

template <class _Tp>
struct add_cv
{
    typedef typename add_const<typename add_volatile<_Tp>::type>::type type;
};

// [20.5.6.2] reference modifications:

template <class _Tp>
struct remove_reference
{
    typedef _Tp type;
};

template <class _Tp>
struct remove_reference<_Tp&>
{
    typedef _Tp type;
};
  
// template <class _Tp>
// struct remove_reference<_Tp&&>
// {
//     typedef _Tp type;
// };

template <class _Tp>
struct add_lvalue_reference
{
    typedef _Tp& type;
};

template <class _Tp>
struct add_lvalue_reference<_Tp&>
{
    typedef _Tp& type;
};
 
// template <class _Tp>
// struct add_rvalue_reference
// {
//     typedef _Tp&& type;
// };

// template <class _Tp>
// struct add_rvalue_reference<_Tp&&>
// {
//     typedef _Tp&& type;
// };

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

// [20.5.6.4] array modifications (see above)

// [20.5.6.5] pointer modifications:

template <class _Tp>
struct remove_pointer
{
    typedef _Tp type;
};

template <class _Tp>
struct remove_pointer<_Tp *>
{
    typedef _Tp type;
};

template <class _Tp>
struct remove_pointer<_Tp * const>
{
    typedef _Tp type;
};

template <class _Tp>
struct remove_pointer<_Tp * volatile>
{
    typedef _Tp type;
};

template <class _Tp>
struct remove_pointer<_Tp * const volatile>
{
    typedef _Tp type;
};

template <class _Tp>
struct add_pointer
{
    typedef typename remove_reference<_Tp>::type * type;
};

// [20.5.7] other transformations:

// template <std::size_t Len, std::size_t Align> struct aligned_storage;
// template <std::size_t Len, class... Types> struct aligned_union;

namespace detail {

template <bool,class _U1>
struct _decay_aux2
{
    typedef typename remove_cv<_U1>::type type;
};

template <class _U1>
struct _decay_aux2<true,_U1>
{
    typedef typename add_pointer<_U1>::type type;
};

template <bool, class _U1>
struct _decay_aux1
{
    typedef typename _decay_aux2<is_function<_U1>::value,_U1>::type type;
};

template <class _U1>
struct _decay_aux1<true,_U1>
{
    typedef typename remove_extent<_U1>::type* type;
};

} // namespace detail

template <class _Tp>
class decay
{
  private:
    typedef typename remove_reference<_Tp>::type _U1;

  public:
    typedef typename detail::_decay_aux1<is_array<_U1>::value,_U1>::type type;
};

template <bool, class _Tp = void>
struct enable_if
{
};

template <class _Tp>
struct enable_if<true,_Tp>
{
    typedef _Tp type;
};

template <bool, class _Tp1, class _Tp2>
struct conditional
{
    typedef _Tp2 type;
};

template <class _Tp1, class _Tp2>
struct conditional<true,_Tp1,_Tp2>
{
    typedef _Tp1 type;
};

// template <class... _Tp> struct common_type;
#ifdef __FIT_CPP_0X

template <class... _Tp>
struct common_type;

template <class _Tp>
struct common_type<_Tp>
{
    typedef _Tp type;
};

template <class _T1, class _T2>
struct common_type<_T1,_T2>
{
    typedef decltype( true ? declval<_T1>() : declval<_T2>() ) type;
};

template <class _T1, class _T2, class... _T3>
struct common_type<_T1,_T2,_T3...>
{
    typedef typename common_type<typename common_type<_T1,_T2>::type,_T3...>::type type;
};

#endif // __FIT_CPP_0X

#undef __CV_SPEC
#undef __SPEC_
#undef __CV_SPEC_1
#undef __SPEC_1
#undef __CV_SPEC_2
#undef __SPEC_2

} // namespace std

# else // __GLIBCXX__ && (__GNUC__ >= 4) && !STLPORT
#  if defined(__GNUC__) && ((__GNUC__ == 4) && !defined(__FIT_CPP_0X) /* && (__GNUC_MINOR__ < 3) */ )
#    include <tr1/type_traits>
namespace std {

namespace tr1 {

template <class _Tp>
struct is_trivial :
    public integral_constant<bool, (is_void<_Tp>::value
                                    || is_scalar<typename remove_all_extents<_Tp>::type>::value)>
{ };

template <class _Tp>
struct is_standard_layout :
    public integral_constant<bool, (is_void<_Tp>::value
                                    || is_scalar<typename remove_all_extents<_Tp>::type>::value)>
{ };

template <class _Tp>
struct is_lvalue_reference :
    public false_type
{ };

template <class _Tp>
struct is_lvalue_reference<_Tp&> :
    public true_type
{ };

template <class _Tp>
struct is_rvalue_reference :
    public false_type
{ };

template <class _Tp>
struct add_lvalue_reference
{
    typedef _Tp& type;
};

template <class _Tp>
struct add_lvalue_reference<_Tp&>
{
    typedef _Tp& type;
};

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

namespace detail {

template <bool,class _U1>
struct _decay_aux2
{
    typedef typename remove_cv<_U1>::type type;
};

template <class _U1>
struct _decay_aux2<true,_U1>
{
    typedef typename add_pointer<_U1>::type type;
};

template <bool, class _U1>
struct _decay_aux1
{
    typedef typename _decay_aux2<is_function<_U1>::value,_U1>::type type;
};

template <class _U1>
struct _decay_aux1<true,_U1>
{
    typedef typename remove_extent<_U1>::type* type;
};

} // namespace detail

template <class _Tp>
class decay
{
  private:
    typedef typename remove_reference<_Tp>::type _U1;

  public:
    typedef typename detail::_decay_aux1<is_array<_U1>::value,_U1>::type type;
};

template <bool, class _Tp = void>
struct enable_if
{
};

template <class _Tp>
struct enable_if<true,_Tp>
{
    typedef _Tp type;
};

template <bool, class _Tp1, class _Tp2>
struct conditional
{
    typedef _Tp2 type;
};

template <class _Tp1, class _Tp2>
struct conditional<true,_Tp1,_Tp2>
{
    typedef _Tp1 type;
};

// #ifdef __FIT_CPP_0X  // <--- fix

//template <class _Tp>
//struct common_type<_Tp>
//{
//    typedef _Tp type;
//};

template <class _T1, class _T2>
struct common_type
{
    // typedef _Tp type;
};

template <class _T1>
struct common_type<_T1,_T1>
{
    typedef _T1 type;
};

// template <class _T1, class _T2>
//struct common_type<_T1,_T2>
//{
//    typedef decltype( true ? declval<_T1>() : declval<_T2>() ) type;
//};

//template <class _T1, class _T2, class... _T3>
//struct common_type<_T1,_T2,_T3...>
//{
//    typedef typename common_type<typename common_type<_T1,_T2>::type,_T3...>::type type;
//};

// #endif // __FIT_CPP_0X

} // namespace tr1

using namespace tr1;

} // namespace std
#  else
#   include <type_traits>
#  endif
# endif

#else // STLPORT
# include <type_traits>
#endif

#endif // __misc_type_traits_h
