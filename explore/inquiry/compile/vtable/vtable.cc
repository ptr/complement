
class A
{
  public:
    virtual void x();
};

void A::x()
{
}

class B :
    public A
{
  public:
    virtual void x();
};

// void B::x()
// {
// }

/* 
 * You will see diagnostic:
 * obj/gcc/shared-g/vtable.o: In function `B::B[in-charge]()':
 * vtable.cc:32: undefined reference to `vtable for B'
 *
 * You will see this diagnostic while compile with debug option,
 * but not for release build.
 *
 */

int main( int, char * const * )
{
  B b;

  return 0;
}
