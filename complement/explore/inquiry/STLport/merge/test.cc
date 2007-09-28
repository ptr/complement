#include <list>
#include <iostream>

using namespace std;

int main()
{
  // char buf1[1024];
  // StackAllocator<int> stack1(buf1, buf1 + sizeof(buf1));

  // char buf2[1024];
  // StackAllocator<int> stack2(buf2, buf2 + sizeof(buf2));

  // typedef list<int, StackAllocator<int> > ListInt;
  typedef list<int> ListInt;

  ListInt lint1(10, 0 /* , stack1 */ );
  ListInt lint2(10, 1 /* , stack2 */ );

  // ListInt lintref(stack2);
  // lintref.insert(lintref.begin(), 10, 1);
  // lintref.insert(lintref.begin(), 10, 0);

  lint1.merge(lint2);
  cerr << lint1.size() << endl;
  // CPPUNIT_ASSERT( lint1 == lintref );
  //  CPPUNIT_ASSERT( lint2.empty() );

  return 0;
}
