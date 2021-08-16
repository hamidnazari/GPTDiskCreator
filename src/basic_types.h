#ifndef THATDISKCREATOR__BASIC_TYPES_H
#define THATDISKCREATOR__BASIC_TYPES_H

#include <stdint.h>

typedef uint64_t disk_size_b_t;
typedef uint16_t block_size_b_t;
typedef uint64_t partition_size_b_t;

static inline disk_size_b_t kb(uint32_t size) {
  return (disk_size_b_t) size << 10;
}

static inline disk_size_b_t mb(uint32_t size) {
  return (disk_size_b_t) size << 20;
}

static inline disk_size_b_t gb(uint32_t size) {
  return (disk_size_b_t) size << 30;
}

#endif //THATDISKCREATOR__BASIC_TYPES_H
