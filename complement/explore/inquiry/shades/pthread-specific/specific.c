#include <pthread.h>
#include <stdio.h>

int main( int argc, char **argv )
{
  pthread_key_t mt_key;
  char *value = 0;
  int ret = 0;
  ret = pthread_key_create( &mt_key, 0 );
  fprintf( stderr, "0x%x %s:%d\n", ret, __FILE__, __LINE__ );
  value = (char *)malloc( 20 );
  strcpy( value, "Hello, world!" );
  ret = pthread_setspecific( mt_key, value );

  fprintf( stderr, "0x%x %s:%d\n", value, __FILE__, __LINE__ );
  fprintf( stderr, "0x%x %s:%d\n", ret, __FILE__, __LINE__ );

  value = pthread_getspecific( mt_key );
  fprintf( stderr, "0x%x %s:%d\n", value, __FILE__, __LINE__ );

  return 0;
}
