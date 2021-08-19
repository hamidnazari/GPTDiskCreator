#ifndef THATDISKCREATOR__GPT_H
#define THATDISKCREATOR__GPT_H

#include "basic_types.h"
#include "crc_32.h"
#include "guid.h"
#include "lba.h"
#include <stdbool.h>
#include <stdint.h>

#define GPT_HEADER_SIZE_B 92
#define GPT_PARTITION_ARRAY_LENGTH 128
#define GPT_RESERVED_MB 1
#define GPT_RESERVED_B mb(GPT_RESERVED_MB)
#define GPT_BLOCK_SIZE_MIN_B 512
#define GPT_BLOCK_SIZE_MAX_B 4096
// TODO: can be longer, min partition entries is 4
// TODO: replace GPT_LBA_COUNT with a variable
#define GPT_LBA_COUNT (1 + 32)

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

void populate_gpt_header(gpt_header_t *header_out, lba_t header_lba, lba_t backup_lba, lba_t first_lba, lba_t last_lba, crc_32_t partition_entries_crc_32);

void populate_gpt_backup_header(gpt_header_t *header_out, const gpt_header_t *header);

void populate_gpt_partition(gpt_partition_t *partition_out, lba_t first_lba, lba_t last_lba, uint8_t boot_flag, bool is_esp, partition_index_t name_index);

#endif // THATDISKCREATOR__GPT_H
