/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (c) 2016       Petr Ovtchenkov
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __SOCKIOS_BT_H
#define __SOCKIOS_BT_H

#include <stdint.h>
#include <sys/socket.h>
#include <bits/types.h>

#ifndef __BLUETOOTH_H /* avoid conflict with BlueZ lib/bluetooth.h */

/* follow netinet/in.h tradition */

/* Bluetooth protocols */
enum {
  BTPROTO_L2CAP = 0,    /* Bluetooth L2CAP */
#define BTPROTO_L2CAP   BTPROTO_L2CAP
  BTPROTO_HCI = 1,      /* Bluetooth HCI */
#define BTPROTO_HCI     BTPROTO_HCI
  BTPROTO_SCO = 2,      /* Bluetooth SCO */
#define BTPROTO_SCO     BTPROTO_SCO
  BTPROTO_RFCOMM = 3,   /* */
#define BTPROTO_RFCOMM  BTPROTO_RFCOMM
  BTPROTO_BNEP = 4,     /* */
#define BTPROTO_BNEP    BTPROTO_BNEP
  BTPROTO_CMTP = 5,     /* */
#define BTPROTO_CMTP    BTPROTO_CMTP
  BTPROTO_HIDP = 6,     /* */
#define BTPROTO_HIDP    BTPROTO_HIDP
  BTPROTO_AVDTP = 7,    /* */
#define BTPROTO_AVDTP   BTPROTO_AVDTP
};

/* Bluetooth socket options  */

#define SOL_HCI         0
#define SOL_L2CAP       6
#define SOL_SCO         17
#define SOL_RFCOMM      18

#ifndef SOL_BLUETOOTH
#define SOL_BLUETOOTH   274
#endif

#endif /* __BLUETOOTH_H */

/* Bluetooth device address */
typedef uint8_t bd_addr_t[6];
struct bd_addr
{
    bd_addr_t s_addr;
};

#ifndef __BLUETOOTH_H /* avoid conflict with BlueZ lib/bluetooth.h */

/* BD Address type */
#define BDADDR_BREDR           0x00
#define BDADDR_LE_PUBLIC       0x01
#define BDADDR_LE_RANDOM       0x02

#define BDADDR_ANY   {{0, 0, 0, 0, 0, 0}}
#define BDADDR_ALL   {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}}
#define BDADDR_LOCAL {{0, 0, 0, 0xff, 0xff, 0xff}}

#endif /* __BLUETOOTH_H */

/* Structure describing an Bluetooth HCI socket address. */
struct sockaddr_bt_hci
{
    __SOCKADDR_COMMON (sbt_hci_);
    uint16_t dev;
    uint16_t channel;

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sbt_hci_zero[sizeof (struct sockaddr) -
                               __SOCKADDR_COMMON_SIZE -
                               sizeof (uint16_t) -
                               sizeof (uint16_t)];
};

#endif /* __SOCKIOS_BT_H */
