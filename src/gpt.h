#ifndef THATDISKCREATOR__GPT_H
#define THATDISKCREATOR__GPT_H

#include "basic_types.h"
#include "decl.h"
#include "guid.h"
#include <stdint.h>

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

// returns the last Logical Block index on a given disk
uint64_t get_disk_last_lbi(disk_size_b_t disk_size, block_size_b_t logical_block_size);

// returns the last Logical Block index for a given size and offset
uint64_t get_block_lbi(uint64_t offset, uint32_t size, block_size_b_t logical_block_size);

// returns Logical Block Address of a Logical Block index
uint64_t translate_lbi_to_lba(int64_t index, disk_size_b_t disk_size, block_size_b_t logical_block_size);

#endif // THATDISKCREATOR__GPT_H
