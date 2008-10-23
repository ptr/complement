#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>

namespace sct
{

class sock
{
  private:
    struct hostent *phe;
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s, type;

    int connectSock( const char *service, const char *transport, int qlen, char *host);
    
  public:
    sock( const char *service, const char *transport, int qlen);    //constructor for server socket
    sock( const char *service, const char *transport, char *host);  //constructor for client socket
    ~sock() 
      {};
    int getDescr();
};

} //namespace sct
