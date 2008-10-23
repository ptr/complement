
struct A
{
  private:
    struct B
    {
        template <typename T>
        static void f( T& ) {}

        template <bool V>
        struct C
        {
#if 0
            template <typename T>
            static void f( T& ) {}
#endif
        };
        template <> struct C<true>;
#if 0
        template <>
        struct C<true>
        {
# if 0
            template <typename T>
            static void f( T& ) {}
# endif
        };
#endif
    };
};

#if 0
template <> template <typename T>
void A::B::C<true>::f( T& ) {}
#endif

#if 1
template <>
struct A::B::C<true>
{
};
#endif

int main()
{
  return 0;
}
