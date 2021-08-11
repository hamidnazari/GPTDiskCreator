#ifndef THATDISKCREATOR__CRC_32_H
#define THATDISKCREATOR__CRC_32_H

#include <stdint.h>

typedef uint32_t crc_32_t;

crc_32_t calculate_crc_32(uint8_t *stream_ptr, uint32_t n);

#endif //THATDISKCREATOR__CRC_32_H
