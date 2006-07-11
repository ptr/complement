#include <iostream>
#include <string>
#include <list>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
// #include <ifaddrs.h>
#include <netinet/in.h>

#include <algorithm>

using namespace std;

struct net_interface
{
  net_interface( const char *nm, int f, const sockaddr& address ) :
    name( nm ),
    flags( f )
  {
    _address.any = address;
  }

  net_interface()
  {}

  string name;
  int flags;
  union {
    sockaddr_in inet;
    sockaddr    any;
  } _address;

};

int main()
{
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  struct ifconf ifc;

  int num = 0;
  ioctl( sock, SIOCGIFCOUNT, &num );
  cerr << "Count interfaces: " << num << endl;

  int lastlen = 0;
  int len = 100 * sizeof(struct ifreq);
  char *buf = 0;
  for ( ; ; ) {
    buf = (char *)malloc(len);
    ifc.ifc_len = len;
    ifc.ifc_buf = buf;
    memset (buf, 0, len);
    if ( ioctl(sock, SIOCGIFCONF, &ifc) < 0 ) {
      if (errno != EINVAL || lastlen != 0) {
        cerr << "SIOCGIFCONF ioctl error getting list of interfaces: " << errno << endl;
      }
    } else {
      if ((unsigned) ifc.ifc_len < sizeof(struct ifreq)) {
        cerr << "SIOCGIFCONF ioctl gave too small return buffer" << endl;
      }
      if (ifc.ifc_len == lastlen)
        break;                  /* success, len has not changed */
      lastlen = ifc.ifc_len;
    }
    len += 10 * sizeof(struct ifreq);       /* increment */
    free(buf);
  }
  struct ifreq* ifr = (struct ifreq *) ifc.ifc_req;
  struct ifreq* last = (struct ifreq *) ((char *) ifr + ifc.ifc_len);
  list<net_interface> lst;
  while (ifr < last) {
    /*
     * Skip entries that begin with "dummy", or that include
     * a ":" (the latter are Solaris virtuals).
     */
    cerr << "Interface " << ifr->ifr_name << endl;
    lst.push_back( net_interface( ifr->ifr_name, 0, ifr->ifr_addr) );
    cerr << hex << lst.back()._address.inet.sin_addr.s_addr << dec << endl;

    // if (strncmp(ifr->ifr_name, "dummy", 5) == 0 || strchr(ifr->ifr_name, ':') != NULL)
    //   goto next;

    /*
     * If we already have this interface name on the list,
     * don't add it, but, if we don't already have an IP
     * address for it, add that address (SIOCGIFCONF returns,
     * at least on BSD-flavored systems, one entry per
     * interface *address*; if an interface has multiple
     * addresses, we get multiple entries for it).
     */
     // user_data.name = ifr->ifr_name;
     // user_data.if_info = NULL;
     // g_list_foreach(il, search_for_if_cb, &user_data);
     // if (user_data.if_info != NULL) {
     //   if_info_add_address(user_data.if_info, &ifr->ifr_addr);
     //  goto next;
     // }

     /*
      * Get the interface flags.
      */
     // memset(&ifrflags, 0, sizeof ifrflags);
     // strncpy(ifrflags.ifr_name, ifr->ifr_name,
     // sizeof ifrflags.ifr_name);
     // if (ioctl(sock, SIOCGIFFLAGS, (char *)&ifrflags) < 0) {
     //   if (errno == ENXIO)
     //     goto next;
     //   g_snprintf(err_str, PCAP_ERRBUF_SIZE,
     //              "SIOCGIFFLAGS error getting flags for interface %s: %s",
     //              ifr->ifr_name, strerror(errno));
     //   goto fail;
     // }

     /*
      * Skip interfaces that aren't up.
      */
     // if (!(ifrflags.ifr_flags & IFF_UP))
     //   goto next;

     /*
      * Skip interfaces that we can't open with "libpcap".
      * Open with the minimum packet size - it appears that the
      * IRIX SIOCSNOOPLEN "ioctl" may fail if the capture length
      * supplied is too large, rather than just truncating it.
      */
     // pch = pcap_open_live(ifr->ifr_name, MIN_PACKET_SIZE, 0, 0,
     //             err_str);
     // if (pch == NULL)
     //   goto next;
     // pcap_close(pch);

                /*
                 * If it's a loopback interface, add it at the end of the
                 * list, otherwise add it after the last non-loopback
                 * interface, so all loopback interfaces go at the end - we
                 * don't want a loopback interface to be the default capture
                 * device unless there are no non-loopback devices.
                 */
     // if_info = if_info_new(ifr->ifr_name, NULL);
     // if_info_add_address(if_info, &ifr->ifr_addr);
     // if ((ifrflags.ifr_flags & IFF_LOOPBACK) ||
     //     strncmp(ifr->ifr_name, "lo", 2) == 0) {
     //   if_info->loopback = TRUE;
     //   il = g_list_append(il, if_info);
     // } else {
     //   if_info->loopback = FALSE;
     //   il = g_list_insert(il, if_info, nonloopback_pos);
                        /*
                         * Insert the next non-loopback interface after this
                         * one.
                         */
     //   nonloopback_pos++;
     // }

     // next:
// #ifdef HAVE_SA_LEN
//     ifr = (struct ifreq *) ((char *) ifr +
//                    (ifr->ifr_addr.sa_len > sizeof(ifr->ifr_addr) ?
//                        ifr->ifr_addr.sa_len : sizeof(ifr->ifr_addr)) +
//                    IFNAMSIZ);
// #else
    ifr = (struct ifreq *) ((char *) ifr + sizeof(struct ifreq));
// #endif
  }



  close( sock );
  return 0;
}
