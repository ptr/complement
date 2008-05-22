#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include "socket.h"

const int bufsize = 4096;

int main()
{
  struct sockaddr_in fsin;
  
  char *service = "3000";
//  char *transport = "udp";
  char *transport = "tcp";
  int qlen = 0;
  char buf[bufsize];

  sct::sock s ( service, transport, qlen );
  unsigned int alen;
  if (transport == "tcp")
  {
    alen = sizeof( fsin );
    int sc = accept( s.getDescr(), (struct sockaddr *)&fsin, &alen);
      
    while (1){
      int cc = read( sc, buf, sizeof( buf ) );
      write( sc, buf, cc );
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
