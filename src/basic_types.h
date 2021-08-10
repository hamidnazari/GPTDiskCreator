#ifndef THATDISKCREATOR__BASIC_TYPES_H
#define THATDISKCREATOR__BASIC_TYPES_H

#include <stdint.h>

typedef uint64_t disk_size_b_t;
typedef uint64_t partition_size_b_t;
typedef uint16_t block_size_b_t;
typedef int8_t partition_index_t;

typedef enum {
  DISK_OPTIONS_VALID = 0,
  DISK_OPTIONS_INVALID_DISK_SIZE = -1,
  DISK_OPTIONS_INVALID_BLOCK_SIZE = -2,
  DISK_OPTIONS_INVALID_PARTITION_SIZES = -3,
  DISK_OPTIONS_INVALID_ESP_INDEX = -4,
} disk_options_state_e;

#endif //THATDISKCREATOR__BASIC_TYPES_H
