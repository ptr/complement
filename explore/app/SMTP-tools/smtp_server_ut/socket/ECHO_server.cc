#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include <pthread.h>

#include "socket.h"

const int bufsize = 4096;
char buf[bufsize];

int TCPecho (int fd){
  int cc;
  while ( cc = read( fd, buf, sizeof( buf ) ) ) {
    write( fd, buf, cc );
  }
  close(fd);
  return 0;
}

int main()
{
  struct sockaddr_in fsin;

  char *service = "3000";
//  char *transport = "udp";
  char *transport = "tcp";
  int qlen = 0;
  unsigned int alen;

  pthread_t th;
  
  sct::sock s ( service, transport, qlen );
  
  if (transport == "tcp")
  {
    while (1){
      alen = sizeof( fsin );
      int sc = accept( s.getDescr(), (struct sockaddr *)&fsin, &alen);
      pthread_create( &th, 0, ( void * (*)( void *))TCPecho, (void *)sc );
    }
  } else {
    while (1){
      alen = sizeof( fsin );
      recvfrom( s.getDescr(), buf, sizeof( buf ), 0, (struct sockaddr *)&fsin, &alen );
      sendto( s.getDescr(), buf, sizeof( buf ), 0, (struct sockaddr *)&fsin, sizeof( fsin ) );
    }
  }
  return 0;
}
