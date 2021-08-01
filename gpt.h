#ifndef THATDISKCREATOR__GPT_H
#define THATDISKCREATOR__GPT_H


#include "decl.h"
#include "guid.h"
#include <stdint.h>
#include <string.h>


// INFO about EFI GPT PDF 2.9 Specs page 120 - 127

// 128 bytes long
typedef struct {
  guid_t type_guid;
  guid_t partition_guid;
  uint64_t first_lba;
  uint64_t last_lba;
  uint8_t attributes[8];
  uint16_t partition_name[36]; // 36 UTF-16LE bytes
} __attribute__((packed)) __attribute__((aligned(128))) gpt_entry_t;


// Logical Block bytes long
typedef struct {
  uint8_t signature[8];
  uint8_t revision[4];
  uint8_t header_size[4];
  uint32_t header_crc_32;
  uint8_t reserved[4]; // set to 0 across the board
  uint64_t header_lba;
  uint64_t backup_lba;
  uint64_t first_usable_lba;
  uint64_t last_usable_lba;
  guid_t disk_guid;
  uint64_t partition_entries_lba;
  uint8_t partitions_count[4];
  uint32_t partition_entry_size;
  uint32_t partition_entries_crc_32;
  char slack[LOGICAL_BLOCK_SIZE_B - GPT_HEADER_SIZE_B]; // set to 0 across the board
} __attribute__((packed)) __attribute__((aligned(LOGICAL_BLOCK_SIZE_B))) gpt_header_t;


uint64_t get_lba(int64_t index) {
  // index -1 is the very last LBA, index -2 is the one before it and so on.
  uint64_t abs_index = index >= 0 ? index : (LOGICAL_BLOCK_MAX + index + 1);

  // if overflow is inevitable set all bits to 1
  uint64_t lba = abs_index < LOGICAL_BLOCK_THEORETICAL_MAX ? abs_index * LOGICAL_BLOCK_SIZE_B : -1ULL;

  return lba;
}

// returns last LBA index for the specified size and offset
uint64_t allocate_lba(u_int64_t offset, u_int32_t size) {
  return size / LOGICAL_BLOCK_SIZE_B + offset;
}


#endif // THATDISKCREATOR__GPT_H
