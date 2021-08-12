#ifndef THATDISKCREATOR__GPT_H
#define THATDISKCREATOR__GPT_H

#include "basic_types.h"
#include "crc_32.h"
#include "guid.h"
#include <stdint.h>
#include <stdio.h>

#define GPT_HEADER_SIZE_B 92
#define GPT_PARTITION_ARRAY_LENGTH 128
#define GPT_RESERVED_MB 1
#define GPT_RESERVED_B (GPT_RESERVED_MB << 20)
#define GPT_BLOCK_SIZE_MIN_B 512
#define GPT_BLOCK_SIZE_MAX_B 4096
// TODO: can be longer, min partition entries is 4
// TODO: replace GPT_LBA_COUNT with a variable
#define GPT_LBA_COUNT (1 + 32)

typedef uint64_t lba_t;
typedef int64_t signed_lba_t;
typedef uint64_t partition_size_b_t;
typedef int16_t partition_index_t;
typedef uint16_t partition_name_t[36];

// 128 bytes long
typedef struct {
  guid_t type_guid;
  guid_t partition_guid;
  lba_t first_lba;
  lba_t last_lba;
  uint8_t attributes[8];
  partition_name_t partition_name; // UTF-16LE
} __attribute__((packed)) __attribute__((aligned(128))) gpt_partition_t;

typedef gpt_partition_t gpt_partition_array_t[GPT_PARTITION_ARRAY_LENGTH];

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
lba_t get_block_last_lba(lba_t offset, uint32_t size, block_size_b_t logical_block_size);

// returns disk offset for a given Logical Block Address
off_t translate_lba_to_offset(signed_lba_t lba, disk_size_b_t disk_size, block_size_b_t logical_block_size);

#endif // THATDISKCREATOR__GPT_H
