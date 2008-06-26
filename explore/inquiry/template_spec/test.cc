#include <ostream>

using namespace std;

template <class T>
class A;

template <class T>
ostream& operator >>(ostream&, A<T>);

template
class A<int>;

template <>
ostream& operator >>(ostream&, A<int>);
