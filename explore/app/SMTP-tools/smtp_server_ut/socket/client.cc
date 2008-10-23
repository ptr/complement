#include <unistd.h>
#include <stdio.h>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "socket.h"

int main(){
  const int linelen = 1000;
  char *host = "localhost";
  char *service = "3000";
//  char *transport = "udp";
  char *transport = "tcp";
  char buf[linelen+1];
  int nchars;
  
    
  sct::sock s( service, transport, host );
  
  while ( fgets( buf, sizeof( buf ), stdin) ) {
    buf[linelen] = '\0';
    
    nchars = strlen( buf );
    write( s.getDescr(), buf, nchars );
    buf[0] = '\0';
    int n = read( s.getDescr(), buf, nchars );
    fputs( buf, stdout );
  }

  return 0;
}
