#include "gpt.h"

inline lba_t get_disk_last_lba(disk_size_b_t disk_size, block_size_b_t logical_block_size) {
  return disk_size / logical_block_size - 1;
}

inline lba_t get_block_last_lba(lba_t offset, uint32_t size, block_size_b_t logical_block_size) {
  return offset + (size / logical_block_size) - 1;
}

off_t translate_lba_to_offset(signed_lba_t lba, disk_size_b_t disk_size, block_size_b_t logical_block_size) {
  // LBA -1 refers to the very last block, lba -2 is the one before it and so on.
  lba_t abs_lba = lba >= 0 ? (lba_t) lba : (get_disk_last_lba(disk_size, logical_block_size) + lba + 1);

  // total number of blocks we can possibly fit in a GPT disk of this block size
  lba_t max_lba = UINT64_MAX / logical_block_size;

  // if overflow is inevitable return max LBA
  off_t offset = abs_lba < max_lba ? abs_lba * logical_block_size : UINT64_MAX;

  return offset;
}
