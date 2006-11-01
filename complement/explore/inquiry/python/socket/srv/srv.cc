
#include "ConnectionProcessor.h"
#include <iostream>
#include <sockios/sockmgr.h>

using namespace std;

int main( int, char * const * )
{
  sockmgr_stream_MP<ConnectionProcessor> srv( 2000 );
  srv.wait();

  return 0;
}
