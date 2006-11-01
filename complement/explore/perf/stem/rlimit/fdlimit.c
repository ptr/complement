#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include "sys/resource.h"
 
int main (int argc, char *argv[])
{
        if (argc != 2)
        {
                printf ("usage: fdlimit new-limit\n"
                        "$ ./fdlimit 100\n");
                exit (1);
        }
        struct rlimit rlp;
        int result = getrlimit (RLIMIT_NOFILE, &rlp);
        if (result != 0)
        {
                printf ("error %d: %s\n", errno, strerror (errno));
                exit (1);       
        }
        printf ("current fd limit is %d\n", rlp.rlim_cur);
        printf ("max fd limit is %d\n", rlp.rlim_max);
 
        /* change the current limit */
        rlp.rlim_cur = atoi (argv[1]);
        
        result = setrlimit (RLIMIT_NOFILE, &rlp);
        if (result != 0)
        {
                printf ("error %d: %s\n", errno, strerror (errno));
                /* don't exit. see that the limit is unchanged */
        }
 
        result = getrlimit (RLIMIT_NOFILE, &rlp);
        if (result != 0)
        {
                printf ("error %d: %s\n", errno, strerror (errno));
                exit (1);
        }
        printf ("current fd limit is %d\n", rlp.rlim_cur);
        printf ("max fd limit is %d\n", rlp.rlim_max);
        return 0;
}
