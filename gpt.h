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
  uint8_t partition_name[72]; // 36 UTF-16LE bytes
} __attribute__((packed)) gpt_entry_t;

// Logical Block bytes long
typedef struct {
  uint8_t signature[8];
  uint8_t revision[4];
  uint8_t header_size[4];
  uint8_t header_crc_32[4];
  uint8_t reserved[4]; // set to 0 across the board
  uint64_t header_lba;
  uint64_t backup_lba;
  uint64_t first_usable_lba;
  uint64_t last_usable_lba;
  guid_t disk_guid;
  uint64_t partition_entries_lba;
  uint8_t partitions_count[4];
  uint8_t partition_entry_size[4];
  uint8_t partition_entries_crc_32[4];
  char slack[LOGICAL_BLOCK_SIZE - GPT_HEADER_SIZE]; // set to 0 across the board
} __attribute__((packed)) gpt_header_t;

#endif // THATDISKCREATOR__GPT_H
