#include "gpt.h"
#include "mbr.h"

#include <printf.h>
#include <stdlib.h>
#include <string.h>

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

  fwrite(&mbr, LOGICAL_BLOCK_SIZE, 1, file_ptr);
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
      //.disk_guid = , // TODO: generate guid
      .partitions_count = GPT_LBA_COUNT - 1,
      .partition_entry_size = {0x80, 0x00, 0x00, 0x00}, // TODO: there must be more to this
      //.partition_entries_crc_32 = // TODO: generate this
  };

  // TODO: calculate CRC-32 of header
  //header.header_crc_32 =

  // TODO: calculate CRC-32 of partition entries array
  //header.partition_entries_crc_32 = ;

  fwrite(&header, LOGICAL_BLOCK_SIZE, 1, file_ptr);

  // GPT entries
  char buffer[LOGICAL_BLOCK_SIZE];
  for (int i = 0; i < GPT_LBA_COUNT - 1; ++i) {
    memset(buffer, '\0', LOGICAL_BLOCK_SIZE);
    fwrite(buffer, LOGICAL_BLOCK_SIZE, 1, file_ptr);
  }

  // FIXME: fseeko's offset is signed 64bit whereas LBA is unsigned 64bit. Trouble!
  fseeko(file_ptr, get_lba(-GPT_LBA_COUNT - 1), SEEK_SET);

  // Backup GPT entries
  for (int i = 0; i < GPT_LBA_COUNT - 1; ++i) {
    memset(buffer, '\0', LOGICAL_BLOCK_SIZE);
    fwrite(buffer, LOGICAL_BLOCK_SIZE, 1, file_ptr);
  }

  // Backup GPT header
  fwrite(&header, LOGICAL_BLOCK_SIZE, 1, file_ptr);
}

int main() {
  if (LOGICAL_BLOCK_MAX > LOGICAL_BLOCK_PRACTICAL_MAX) {
    fprintf(stderr, "Disk size and block size too large for this program.\n");
    exit(1);
  }

  FILE *file_ptr = fopen(DISK_FILE_NAME, "we");

  if (file_ptr == NULL) {
    fprintf(stderr, "Could not create %s!\n", DISK_FILE_NAME);
    exit(1);
  }

  write_mbr(file_ptr);
  write_gpt(file_ptr);

  fclose(file_ptr);

  return 0;
}
