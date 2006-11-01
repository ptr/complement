#include <string>
#include <iostream>
#include <iomanip>

using namespace std;
#ifndef _STLP_USE_SHORT_STRING_OPTIM
# error _STLP_USE_SHORT_STRING_OPTIM
#endif

#ifdef _STLP_FORCE_STRING_TERMINATION
# error _STLP_FORCE_STRING_TERMINATION
#endif

int main()
{
  string s; ( "1234456789abcde" );

  for ( int i = 0; i < 256; ++i ) {
    const char *data1 = s.data();

  // s[s.size()];
  // s.c_str();

    const char *null = s.c_str() + s.size();

    if ( *null != '\0' ) {
      cout << "Error" << endl;
    }

    const char *data2 = s.data();

    cout << i << ": " << hex << (unsigned long)data1 << ":" << (unsigned long)data2 << ":" << (unsigned long)null << dec << endl;
    s += 'x';
  }
}
