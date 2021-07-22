#ifndef THATDISKCREATOR__WRITE_H
#define THATDISKCREATOR__WRITE_H

#include "crc_32.h"
#include "gpt.h"
#include "guid.h"
#include "mbr.h"
#include <printf.h>
#include <stdlib.h>
#include <string.h>

size_t write(FILE *file_ptr, const void *ptr, size_t size) {
  const size_t nitems = 1;
  const size_t count = fwrite(ptr, size, nitems, file_ptr);

  if (count < nitems) {
    printf("Write to disk was unsuccessful!\n");
  }

  return count;
}

void write_mbr(FILE *file_ptr) {
  mbr_entry_t pmbr_partition = {
      .boot_indicator = 0, // any value other than 0 is non-compliant
      .partition_type = 0xEE, // protective EFI GPT
      .first_sector = {0x00, 0x02, 0x00}, // from right after the first 512 bytes
      .last_sector = {0xFF, 0xFF, 0xFF}, // all the way to the end of the disk
      .first_lba = 1, // LBA 1
      .sectors_count = LOGICAL_BLOCK_MAX - 1,
  };

  mbr_t mbr = {
      .partition = {pmbr_partition, 0x00, 0x00, 0x00},
      .boot_signature = {0x55, 0xAA},
  };

  write(file_ptr, &mbr, LOGICAL_BLOCK_SIZE);
}

void write_gpt(FILE *file_ptr) {
  gpt_header_t header = {
      .signature = "EFI PART",
      .revision = {0x00, 0x00, 0x01, 0x00},
      .header_size = GPT_HEADER_SIZE,
      .header_lba = get_lba(1),
      .backup_lba = get_lba(-GPT_LBA_COUNT),
      .first_usable_lba = get_lba(GPT_LBA_COUNT + 1),
      .last_usable_lba = get_lba(-GPT_LBA_COUNT - 1),
      .partition_entries_lba = get_lba(2),
      .disk_guid = get_random_guid(),
      .partitions_count = GPT_LBA_COUNT - 1,
      .partition_entry_size = {0x80, 0x00, 0x00, 0x00}, // TODO: there must be more to this
  };

  header.header_crc_32 = calculate_crc_32((uint8_t *) &header, sizeof(gpt_header_t));

  gpt_entry_t efi_partition = {
      .type_guid = parse_guid(GUID_EFI_SYSTEM_PARTITION),
      .partition_guid = get_random_guid(),
      .first_lba = get_lba(GPT_LBA_COUNT),
      .last_lba = get_lba(GPT_LBA_COUNT * 2),
      .attributes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
      .partition_name = u"ESP",
  };

  gpt_entry_t main_partition = {
      .type_guid = parse_guid(GUID_MICROSOFT_BASIC_DATA_PARTITION),
      .partition_guid = get_random_guid(),
      .first_lba = get_lba(GPT_LBA_COUNT),
      .last_lba = get_lba(GPT_LBA_COUNT * 2),
      .attributes = 0,
      .partition_name = u"Main Partition",
  };

  const int partition_entries_count = GPT_PARTITION_ARRAY_SIZE / sizeof(gpt_entry_t);
  gpt_entry_t partitions[partition_entries_count] = {
      efi_partition,
      main_partition,
      0
  };

  header.partition_entries_crc_32 = calculate_crc_32((uint8_t *) &partitions, sizeof(partitions));

  // GPT Header
  write(file_ptr, &header, LOGICAL_BLOCK_SIZE);

  // GPT entries
  for (int i = 0; i < partition_entries_count; ++i) {
    write(file_ptr, &partitions[i], sizeof(gpt_entry_t));
  }

  // FIXME: fseeko's offset is signed 64bit whereas LBA is unsigned 64bit. Trouble!
  fseeko(file_ptr, get_lba(-GPT_LBA_COUNT - 1), SEEK_SET);

  // Backup GPT entries
  for (int i = 0; i < partition_entries_count; ++i) {
    write(file_ptr, &partitions[i], sizeof(gpt_entry_t));
  }

  // Backup GPT header
  write(file_ptr, &header, LOGICAL_BLOCK_SIZE);
}

#endif //THATDISKCREATOR__WRITE_H
