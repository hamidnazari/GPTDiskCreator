#ifndef THATDISKCREATOR__GPT_H
#define THATDISKCREATOR__GPT_H

#include "basic_types.h"
#include "crc_32.h"
#include "decl.h"
#include "guid.h"
#include <stdint.h>
#include <stdio.h>

#define GPT_HEADER_SIZE_B 92

typedef uint64_t lba_t;
typedef int64_t signed_lba_t;
typedef uint64_t partition_size_b_t;
typedef int8_t partition_index_t;

// 128 bytes long
typedef struct {
  guid_t type_guid;
  guid_t partition_guid;
  lba_t first_lba;
  lba_t last_lba;
  uint8_t attributes[8];
  uint16_t partition_name[36]; // 36 UTF-16LE bytes
} __attribute__((packed)) __attribute__((aligned(128))) gpt_entry_t;

// 512 bytes long
typedef struct {
  uint8_t signature[8];
  uint8_t revision[4];
  uint8_t header_size[4];
  crc_32_t header_crc_32;
  uint8_t reserved[4]; // set to 0 across the board
  lba_t header_lba;
  lba_t backup_lba;
  lba_t first_usable_lba;
  lba_t last_usable_lba;
  guid_t disk_guid;
  lba_t partition_entries_lba;
  uint8_t partitions_count[4];
  uint32_t partition_entry_size;
  crc_32_t partition_entries_crc_32;
} __attribute__((packed)) __attribute__((aligned(512))) gpt_header_t;

// returns the last Logical Block Address on a given disk
lba_t get_disk_last_lba(disk_size_b_t disk_size, block_size_b_t logical_block_size);

// returns the last Logical Block Address index for a given size and offset
lba_t get_block_lba(lba_t offset, uint32_t size, block_size_b_t logical_block_size);

// returns disk offset for a given Logical Block Address
off_t translate_lba_to_offset(signed_lba_t lba, disk_size_b_t disk_size, block_size_b_t logical_block_size);

#endif // THATDISKCREATOR__GPT_H
