#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>

using namespace std;

int main()
{
  cout << setprecision(19) << log(FLT_MAX) << ", " << float(M_LN2*FLT_MAX_EXP) << endl;

  cout << numeric_limits<double>::min_exponent << " " << DBL_MIN_EXP << " " << numeric_limits<double>::min_exponent10 << endl;
  cout << numeric_limits<double>::digits << " " << numeric_limits<double>::digits10 << endl;
  cout << numeric_limits<double>::max_exponent << " " << DBL_MAX_EXP << " " << numeric_limits<double>::max_exponent10 << endl;
  cout << numeric_limits<long double>::digits << " " << numeric_limits<long double>::digits10 << endl;
  cout << numeric_limits<long double>::max_exponent << " " << numeric_limits<long double>::max_exponent10 << endl;

  return 0;
}
