#include <stdlib.h>
#include <stdio.h>


int main()
{
  char buf[1024];
  int fd;

  strcpy( buf, "/tmp/qXXXXXX" );
  printf( "%s\n", buf );

  fd = mkstemp( buf );

  printf( "%s\n", buf );
  close( fd );
  /* unlink( buf ); */

  return 0;
}
