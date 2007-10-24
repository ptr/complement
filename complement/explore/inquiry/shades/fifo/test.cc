#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

int main()
{
  mkfifo( "./file", 0666 );
  {
    ofstream f( "./file" );

    f << "Hello" << endl;
  }

  return 0;
}
