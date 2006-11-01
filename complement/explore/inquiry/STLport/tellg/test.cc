#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main()
{
  ifstream f;
  string line;


  f.open("test.txt", fstream::in);

  if ( f.is_open() ) {
    cout << "file position: " << f.tellg() << endl;
    for ( int i = 0; i < 50; i++ ) {
      getline(f, line, '\n');
      cout << "file position: " << f.tellg() << endl;
      cout << "read: " << line << endl;
    }
  }
  return 0;
}
