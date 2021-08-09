#include "gpt.h"

uint64_t get_lba(int64_t index) {
  // index -1 is the very last LBA, index -2 is the one before it and so on.
  uint64_t abs_index = index >= 0 ? index : (LOGICAL_BLOCK_MAX + index + 1);

  // if overflow is inevitable set all bits to 1
  uint64_t lba = abs_index < LOGICAL_BLOCK_THEORETICAL_MAX ? abs_index * LOGICAL_BLOCK_SIZE_B : -1ULL;

  return lba;
}

uint64_t allocate_lba(u_int64_t offset, u_int32_t size) {
  return size / LOGICAL_BLOCK_SIZE_B + offset;
}
