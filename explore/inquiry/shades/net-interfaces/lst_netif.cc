// #define __USE_STREAM

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>

#ifdef __USE_STREAM
# include <stropts.h>
#endif

#include <string>
#include <list>

#include <iostream>

using namespace std;

struct net_iface
{
    string name;
    struct in_addr addr;
    struct in_addr mask;
    bool up;
};

typedef list<net_iface> list_iface;

void get_interfaces( list_iface& lst )
{
  struct ifconf ifc;
  char buff[8192];

  int fd = socket(AF_INET, SOCK_DGRAM, 0);

  if ( fd == -1 ) {
    return;
  }

#ifndef __USE_STREAM
  ifc.ifc_len = sizeof(buff);
  ifc.ifc_buf = buff;

  if ( ioctl(fd, SIOCGIFCONF, &ifc) != 0 ) {
    close(fd);
    return;
  }

  struct ifreq *ifr = ifc.ifc_req;
  int n = ifc.ifc_len / sizeof(struct ifreq);

  for ( int i = 0; i < n; ++i ) {
    if ( ioctl(fd, SIOCGIFADDR, &ifr[i]) == 0 ) {
      lst.push_back( net_iface() );

      lst.back().name = ifr[i].ifr_name;
      lst.back().addr = ((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr;
      if ( ioctl(fd, SIOCGIFNETMASK, &ifr[i]) == 0 ) {
        lst.back().mask = ((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr;
      } else {
        lst.back().mask.s_addr = 0;
      }
      if ( ioctl(fd, SIOCGIFFLAGS, &ifr[i]) == 0 ) {
        lst.back().up = ( (ifr[i].ifr_flags & IFF_UP) == 0 ) ? false : true;
      } else {
        lst.back().up = false;
      }
    }
  }
#else // __USE_STREAM
  struct strioctl strioctl;

  strioctl.ic_cmd = SIOCGIFCONF;
  strioctl.ic_dp  = buff;
  strioctl.ic_len = sizeof(buff);
  
  if ( ioctl(fd, I_STR, &strioctl) != 0 ) {
    close(fd);
    return;
  }
  /* we can ignore the possible sizeof(int) here as the resulting
     number of interface structures won't change */
  int n = strioctl.ic_len / sizeof(struct ifreq);
  /* we will assume that the kernel returns the length as an int
     at the start of the buffer if the offered size is a
     multiple of the structure size plus an int */
  struct ifreq *ifr = 
    (n * sizeof(struct ifreq) + sizeof(int) == strioctl.ic_len) ?
    (struct ifreq *)(buff + sizeof(int)) : (struct ifreq *)buff;
  for ( int i = 0; i < n; ++i ) {
    struct ifreq ifreq = ifr[i];

    strioctl.ic_cmd = SIOCGIFADDR;
    strioctl.ic_dp  = (char *)&ifreq;
    strioctl.ic_len = sizeof(struct ifreq);
    if ( ioctl(fd, I_STR, &strioctl) == 0 ) {
      lst.push_back( net_iface() );

      lst.back().name = ifreq.ifr_name;
      lst.back().addr = ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;

      strioctl.ic_cmd = SIOCGIFNETMASK;
      strioctl.ic_dp  = (char *)&ifreq;
      strioctl.ic_len = sizeof(struct ifreq);
      if ( ioctl(fd, I_STR, &strioctl) == 0 ) {
        lst.back().mask = ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
      } else {
        lst.back().mask.s_addr = 0;
      }

      strioctl.ic_cmd = SIOCGIFFLAGS;
      strioctl.ic_dp  = (char *)&ifreq;
      strioctl.ic_len = sizeof(struct ifreq);
      if ( ioctl(fd, I_STR, &strioctl) == 0 ) {
        lst.back().up = ( (ifreq.ifr_flags & IFF_UP) == 0 ) ? false : true;
      } else {
        lst.back().up = false;
      }

    }
  }
#endif // __USE_STREAM

  close(fd);
}

int main ()
{
  list_iface l;

  get_interfaces( l );

  for ( list_iface::const_iterator i = l.begin(); i != l.end(); ++i ) {
    cerr << i->name
         << "\t\t"
         << (i->addr.s_addr&0xff) << "."
         << ((i->addr.s_addr >> 8)&0xff) << "."
         << ((i->addr.s_addr >> 16)&0xff) << "."
         << ((i->addr.s_addr >> 24)&0xff)
         << "\t"
         << (i->mask.s_addr&0xff) << "."
         << ((i->mask.s_addr >> 8)&0xff) << "."
         << ((i->mask.s_addr >> 16)&0xff) << "."
         << ((i->mask.s_addr >> 24)&0xff)
         << endl;
  }

  return 0;
}
