#include "gpt.h"

inline uint64_t get_disk_last_lbi(disk_size_b_t disk_size, block_size_b_t logical_block_size) {
  return disk_size / logical_block_size - 1;
}

inline uint64_t get_block_lbi(uint64_t offset, uint32_t size, block_size_b_t logical_block_size) {
  return offset + (size / logical_block_size);
}

uint64_t translate_lbi_to_lba(int64_t index, disk_size_b_t disk_size, block_size_b_t logical_block_size) {
  // index -1 is the very last LBA, index -2 is the one before it and so on.
  uint64_t abs_index = index >= 0 ? index : (get_disk_last_lbi(disk_size, logical_block_size) + index + 1);

  // total number of blocks we can possibly fit in a GPT disk of this block size
  uint64_t theoretical_max = -1ULL / logical_block_size;

  // if overflow is inevitable set all bits to 1
  uint64_t lba = abs_index < theoretical_max ? abs_index * logical_block_size : -1ULL;

  return lba;
}
