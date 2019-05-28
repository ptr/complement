/*
 * Copyright (c) 2019
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __CRC16_CCITT_H
#define __CRC16_CCITT_H

#include <stdint.h>
#include <sys/types.h>

#include "config/feature.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef uint16_t crc16_type;

union crc16_bytes_selector {
    crc16_type crc;
    struct {
#ifdef _LITTLE_ENDIAN
        uint8_t low;
        uint8_t high;
#elif defined(_BIG__ENDIAN)
        uint8_t high;
        uint8_t low;
#endif
    } byte;
};

extern crc16_type crc16_ccitt_init();
extern crc16_type crc16_ccitt_byte(crc16_type crc, uint8_t b);
extern crc16_type crc16_ccitt_update(crc16_type crc, uint8_t const *octets, size_t len);
extern crc16_type crc16_ccitt_final(crc16_type crc);

#ifdef  __cplusplus
}
#endif

#endif /* __CRC16_CCITT_H */
