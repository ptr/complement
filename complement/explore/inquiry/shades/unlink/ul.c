#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
  int i;
  for ( i = 0; i < 1000; ++i ) {
    int fd = open( "yy", O_CREAT | O_WRONLY, 0666 );
    char c[60*1024];
    if ( fd < 0 ) {
      printf( "shit happens\n" );
    }
    write( fd, c, 60*1024 ); 
    close(fd);
    unlink( "yy" );
    // printf( "%d\n", i );
  }
  return 0;
}
