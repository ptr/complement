#include <algorithm>

using std::min;

enum Foo { foo = 0 };

const int bar = 0;

int main()
{
  min(foo, foo); // GNU okay. STLport warning/error: returns temporary
  min(bar, bar); // Okay

  return 0;
}
