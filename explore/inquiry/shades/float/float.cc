#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

int main()
{
  cout << setprecision(19) << log(FLT_MAX) << ", " << float(M_LN2*FLT_MAX_EXP) << endl;
  return 0;
}
