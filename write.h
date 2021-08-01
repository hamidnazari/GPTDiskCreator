#ifndef THATDISKCREATOR__WRITE_H
#define THATDISKCREATOR__WRITE_H


#include "crc_32.h"
#include "gpt.h"
#include "guid.h"
#include "mbr.h"
#include <printf.h>
#include <stdlib.h>
#include <string.h>


static const uint64_t esp_first_lba_index = 2048; // TODO: replace with GPT_LBA_COUNT+1?


static size_t write(FILE *file_ptr, const void *ptr, size_t size) {
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
      .first_lba = 1,
      .sectors_count = LOGICAL_BLOCK_MAX - 1,
  };

  mbr_t mbr = {
      .partition = {pmbr_partition, 0x00, 0x00, 0x00},
      .boot_signature = {0x55, 0xAA},
  };

  write(file_ptr, &mbr, LOGICAL_BLOCK_SIZE_B);
}


void write_gpt(FILE *file_ptr) {
  const uint32_t partition_entry_size = sizeof(gpt_entry_t);
  const uint8_t partition_entries_count = GPT_PARTITION_ARRAY_SIZE_B / partition_entry_size;
  const uint64_t last_usable_lba = LOGICAL_BLOCK_MAX - GPT_LBA_COUNT - 1;

  gpt_header_t header = {
      .signature = "EFI PART",
      .revision = {0x00, 0x00, 0x01, 0x00},
      .header_size = GPT_HEADER_SIZE_B,
      .header_lba = 1,
      .backup_lba = LOGICAL_BLOCK_MAX - 1,
      .first_usable_lba = GPT_LBA_COUNT + 1,
      .last_usable_lba = last_usable_lba,
      .partition_entries_lba = 2,
      .disk_guid = get_random_guid(),
      .partitions_count = partition_entries_count,
      .partition_entry_size = partition_entry_size,
  };

  const uint64_t esp_last_lba_index = allocate_lba(esp_first_lba_index, ESP_SIZE_B) - 1;
  const uint64_t main_partition_last_lba_index = allocate_lba(esp_last_lba_index + 1, MAIN_VOLUME_SIZE_B) - 1;

  gpt_entry_t efi_system_partition = {
      .type_guid = parse_guid(GUID_EFI_SYSTEM_PARTITION),
      .partition_guid = get_random_guid(),
      .first_lba = esp_first_lba_index,
      .last_lba = esp_last_lba_index,
      .attributes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
      .partition_name = u"ESP",
  };

  gpt_entry_t main_partition = {
      .type_guid = parse_guid(GUID_MICROSOFT_BASIC_DATA_PARTITION),
      .partition_guid = get_random_guid(),
      .first_lba = esp_last_lba_index + 1,
      .last_lba = main_partition_last_lba_index,
      .attributes = 0,
      .partition_name = u"Main Partition",
  };

  gpt_entry_t partitions[partition_entries_count] = {
      efi_system_partition,
      main_partition,
      0,
  };

  header.partition_entries_crc_32 = calculate_crc_32((uint8_t *) &partitions, sizeof(partitions));
  header.header_crc_32 = calculate_crc_32((uint8_t *) &header, GPT_HEADER_SIZE_B);

  // GPT Header
  write(file_ptr, &header, LOGICAL_BLOCK_SIZE_B);

  // GPT entries
  for (uint8_t i = 0; i < partition_entries_count; ++i) {
    write(file_ptr, &partitions[i], sizeof(gpt_entry_t));
  }

  // FIXME: fseeko's offset is signed 64bit whereas LBA is unsigned 64bit. Trouble!
  fseeko(file_ptr, get_lba(-GPT_LBA_COUNT - 1), SEEK_SET);

  // Backup GPT entries
  for (uint8_t i = 0; i < partition_entries_count; ++i) {
    write(file_ptr, &partitions[i], sizeof(gpt_entry_t));
  }

  header.header_crc_32 = 0;
  header.header_lba = LOGICAL_BLOCK_MAX - 1;
  header.backup_lba = 1;
  header.partition_entries_lba = LOGICAL_BLOCK_MAX - GPT_LBA_COUNT;
  header.header_crc_32 = calculate_crc_32((uint8_t *) &header, GPT_HEADER_SIZE_B);

  // Backup GPT header
  write(file_ptr, &header, LOGICAL_BLOCK_SIZE_B);
}


#endif //THATDISKCREATOR__WRITE_H
