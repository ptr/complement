/*
Overload of address operator (&), that return double, not v *; no way to take
address of object of type v; may be constructor v(double) and convert to double can help...
*/
#include <vector>

using namespace std;

class v
{
  private:
    double p[3];

  public:
    v() {}
    v( double ) {}
    
    double *operator &() { return p; }
    const double *operator &() const { return p; }
    double() const { return p[0]; }
};

int main()
{
  vector<v> vec;

  vec.push_back( v() );

  return 0;
}

