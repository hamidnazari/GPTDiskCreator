#include "mbr.h"
#include "gpt.h"
#include <string.h>

void populate_mbr(mbr_t *mbr_out, disk_size_b_t disk_size_b, block_size_b_t logical_block_size_b) {
  mbr_entry_t pmbr_partition = {
      .boot_indicator = 0, // any value other than 0 is non-compliant
      .partition_type = 0xEE, // protective EFI GPT
      .first_sector = {0x00, 0x02, 0x00}, // from right after the first 512 bytes
      .last_sector = {0xFF, 0xFF, 0xFF}, // all the way to the end of the disk
      .first_lba = 1,
      .sectors_count = get_disk_last_lba(disk_size_b, logical_block_size_b),
  };

  mbr_t mbr = {
      .partition = {pmbr_partition, {0x00}, {0x00}, {0x00}},
      .boot_signature = {0x55, 0xAA},
  };

  memcpy(mbr_out, &mbr, sizeof(mbr_t));
}
