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

void *malloc(size_t size)
{
  static void * (*func)();

  if(!func)
    func = (void *(*)()) dlsym(RTLD_NEXT, "malloc");
  printf("malloc(%d) is called\n", size);     
  return(func(size));
}
 
