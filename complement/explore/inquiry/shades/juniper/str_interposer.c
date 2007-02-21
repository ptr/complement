/* Example from: http://developers.sun.com/solaris/articles/lib_interposers_code.html
 *
 */

/* Example of a library interposer: interpose on 
 * malloc().
 * Build and use this interposer as following:
 * cc -o malloc_interposer.so -G -Kpic malloc_interposer.c
 * setenv LD_PRELOAD $cwd/malloc_interposer.so
 * run the app
 * unsetenv LD_PRELOAD
 */

#include <stdio.h>
#include <dlfcn.h>

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

  if(!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "strcpy");

  printf( "strcpy: '%s'\n", s2 );

  return func( s1, s2 );
}

char *strdup(const char *s1)
{
  static void * (*func)();

  if(!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "strdup");

  printf( "strdup: '%s'\n", s1 );

  return func( s1 );
}

char *strncat(char *s1, const char *s2, size_t n)
{
  static void * (*func)();
  char *r;

  if(!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "strncat");

  r = func( s1, s2, n );
  printf( "strncat: '%s'\n", r );

  return r;
}

char *strtok(char *s1, const char *s2)
{
  static void * (*func)();

  if(!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "strtok");

  printf( "strtok: '%s'\n", s1 );

  return func( s1, s2 );
}

