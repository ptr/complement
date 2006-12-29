/*
end() return rvalue (Standard, 3.10 par. 5)

If this is a POD, increment/decrement operators invalid in this context;
If this is user-defined object, temporary object created from rvalue
(Standard, 12.2; 6.6.3) and increment/decrement operators may be valid;

Usage POD type for string iterator is effective and don't contradict to
Standard.

So the code

char c = *(--s.end());

is implementation-specific and that's why not good.

[this is explanation why this code work if begin() return v, but not work
when end() return char *; inspired by string::iterator]
*/
#include <string>

using namespace std;

class v
{
  public:
    v& operator ++() { return *this; }
};

class vconst
{
  public:
    vconst& operator ++() { return *this; }
};


class q
{
  public:
    v begin()
      { return v(); }

    vconst begin() const
      { return vconst(); }

   char *end()
      { return 0; }

  private:
};

int main()
{
  q x;

  ++x.begin();
  ++x.end();

  // string s( "123456" );

  // --s.end();

  // char c = *(--s.end());

  return 0;
}

