#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#include "socket.h"

namespace sct
{

int sock::connectSock( const char *service, const char *transport, int qlen, char *host)
{
  int s;
  memset (&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  
  if ( pse = getservbyname( service, transport ) )
    sin.sin_port = pse->s_port;
  else if ( ( sin.sin_port = htons( (unsigned short)atoi( service ) ) ) == 0 ) 
    std::cerr << "error: can't get service entry";

  ppe = getprotobyname(transport);
  if (ppe == 0) 
    std::cerr << "error: can't get protocol entry";
  
  if ( strcmp( transport, "udp" ) == 0 )
    type = SOCK_DGRAM;
  else {
    type = SOCK_STREAM;
  }

  s = socket( PF_INET, type, ppe->p_proto );
  
  if (s < 0)
    std::cerr << "error: can't create socket";
  
  if ( host == "" ) {         //server socket is created
    sin.sin_addr.s_addr = INADDR_ANY;
    int t = bind( s, (struct sockaddr *)&sin, sizeof( sin ) );
//    std::cerr << t << std::endl;
    if ( type == SOCK_STREAM ) {
      listen(s,qlen);
    }
  } else {                    //client socket is created
    if ( phe = gethostbyname( host ) )
      memcpy( &sin.sin_addr, phe->h_addr, phe->h_length );
    else sin.sin_addr.s_addr = inet_addr( host );
    int t = connect( s, (struct sockaddr *)&sin, sizeof( sin ) );
//    std::cerr << t << std::endl;
  }
  
  return s;
}

sock::sock( const char *service, const char *transport, int qlen) {   //constructor for server socket
  s = connectSock( service, transport, qlen, "");
}

sock::sock( const char *service, const char *transport, char *host) { //constructor for client socket
  s = connectSock( service, transport, 0, host);
}

int sock::getDescr() {
  return s;
}

} //namespace sct
