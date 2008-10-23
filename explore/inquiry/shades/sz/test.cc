#include <iostream>
#include <typeinfo>
// #include <string>

char *arr[][2] = {
  { "1", "1.2" },
  { "2", "2.2" },
  { "2", "2.2" }
};

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
    static const bool __value = sizeof(__test<_Tp>(0)) == 1;
#endif

};

#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
template <class _Tp>
const bool __instance<_Tp>::__value = sizeof(__instance<_Tp>::__test<_Tp>(0)) == 1;
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
    static const bool __value = sizeof(__test<T>(0)) == 1;
#endif
};

#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
template <class T>
const bool __uoc_aux<T>::__value = sizeof(__uoc_aux<T>::__test<T>(0)) == 1;
#endif

template <class T>
struct __xuoc_aux : // union or class
    public __select_types
{
  private:
    template <class _Up>
    static __t1 __test( int (_Up::*)() );

    template <class>
    static __t2 __test(...);

  public:
#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
    static const bool __value;
#else
    static const bool __value = sizeof(__test<T>(0)) == 1;
#endif
};

#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
template <class T>
const bool __xuoc_aux<T>::__value = sizeof(__xuoc_aux<T>::__test<T>(0)) == 1;
#endif

template <class T, bool B>
class __inheritance_aux
{};

template <class T>
class __inheritance_aux<T,false> :
    public T
{
  public:
    virtual ~__inheritance_aux()
      { }
};


template <class T, bool B>
struct __virtual_aux
{
  public:
#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
    static const bool __value;
#else
    static const bool __value = B ? false : (sizeof(__inheritance_aux<T,B>) == sizeof(T));
#endif
};

#ifdef __FIT_NO_INLINE_TEMPLATE_STATIC_INITIALISATION
template <class T, bool B>
const bool __virtual_aux<T,B>::__value = B ? false : sizeof(__inheritance_aux<T,B>) == sizeof(T);
#endif


struct N
{
};

struct M :
  public N
{
};

struct Q :
  public N
{
  virtual ~Q()
    { }
};

struct P
{
  virtual void x()
    { }
};

struct R :
  public P
{
  virtual ~R()
    { }
};

union U
{
  int i;
  double j;

    // int q( void (U::*)(void) );
};

union UU
{
  int i;
  double j;

    // int q( void (U::*)(void) );
};

// struct DU :
//   public U
// {
// };

template <class T>
struct DU
{
    // DU() :
    //     __value( typeid(T) == typeid(T) )
    //   { }

    // static bool __value = typeid(T) == typeid(T);
};

using namespace std;

int main()
{
  cerr << sizeof(arr) << " " << sizeof(arr[0]) << " " << sizeof(arr)/sizeof(arr[0]) << endl;

  cerr << sizeof( N ) << " " << sizeof( M ) << " " << sizeof( Q ) << " " << sizeof(P) << " "
       << sizeof( R ) << endl;

  cerr << __uoc_aux<N>::__value << " " << __xuoc_aux<N>::__value << endl;

  cerr << __virtual_aux<N,false>::__value << " " << __virtual_aux<int,true>::__value << " "
       << __virtual_aux<Q,false>::__value << " " << __virtual_aux<R,false>::__value << endl;

  cerr << __xuoc_aux<U>::__value << endl;

  cerr << (typeid(U) == typeid(UU)) << endl;

#if 0
  string s( "01234567Tabc" );

  string::size_type p = s.find( 'T', 6 );
  if ( p != string::npos ) {
    cerr << s.substr( 0, p ) << endl;
  }

  s.erase( p );
  cerr << s << endl;
#endif
  return 0;
}

