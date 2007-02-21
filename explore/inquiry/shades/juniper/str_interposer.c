/* Example from: http://developers.sun.com/solaris/articles/lib_interposers_code.html
 *
 */

/* library interposer: interpose on
 * str...().
 * export LD_PRELOAD=$cwd/libtest.so.0.0
 * run the app
 * unset LD_PRELOAD
 */

#include <stdio.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>


#if 0
void *malloc(size_t size)
{
  static void * (*func)();

  if(!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "malloc");
  printf("malloc(%d) is called\n", size);     
  return(func(size));
}
#endif

char *strcpy( char *s1, const char *s2 )
{
  static void * (*func)();

  if (!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "strcpy");

  printf( "strcpy: '%s'\n", s2 );

  return func( s1, s2 );
}

char *strdup(const char *s1)
{
  static void * (*func)();

  if (!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "strdup");

  printf( "strdup: '%s'\n", s1 );

  return func( s1 );
}

char *strncat(char *s1, const char *s2, size_t n)
{
  static void * (*func)();
  char *r;

  if (!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "strncat");

  r = func( s1, s2, n );
  printf( "strncat: '%s'\n", r );

  return r;
}

char *strtok(char *s1, const char *s2)
{
  static void * (*func)();

  if (!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "strtok");

  printf( "strtok: '%s'\n", s1 );

  return func( s1, s2 );
}

ssize_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
  static ssize_t (*func)();
  int i;

  if (!func)
    func = (ssize_t (*)()) dlsym(RTLD_NEXT, "sendto");

  printf( "sendto: (%d) '", length );
  for ( i = 0; i < length; ++i ) {
    printf( "%c", (const char *)message + i );
  }
  printf( "'\n" );
  return func( socket, message, length, flags, dest_addr, dest_len );
}

ssize_t recvfrom(int socket, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len)
{
  static ssize_t (*func)();
  int i;
  ssize_t s;

  if (!func)
    func = (ssize_t (*)()) dlsym(RTLD_NEXT, "recvfrom");

  s = func( socket, buffer, length, flags, address, address_len );
  printf( "recvfrom: (%d) '", length );
  for ( i = 0; i < length; ++i ) {
    printf( "%c", (const char *)buffer + i );
  }
  printf( "'\n" );
  return s;
}

int connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
  static int (*func)();

  if (!func)
    func = (int (*)()) dlsym(RTLD_NEXT, "connect");

  printf( "connect: " );
  printf( "%x", *(((unsigned *)&((struct sockaddr_in *)address)->sin_addr)) );
  printf( ":%d\n", ntohs(((struct sockaddr_in *)address)->sin_port) );

  return func( socket, address, address_len );
}
