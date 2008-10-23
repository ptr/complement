#include <stdio.h>
# include <pthread.h>

static void *f( void *par )
{
  fprintf( stderr, "%d\n", (int)par );
  sleep( 10 );
  return 0;
}

int main()
{
  int i;
  const int nth = 4000;
  pthread_t t[nth];
  for ( i = 0; i < nth; ++i ) {
    pthread_create( &t[i], 0, f, (void *)i );
  }

  for ( i = 0; i < nth; ++i ) {
    pthread_join( t[i], 0 );
  }
  return 0;
}
