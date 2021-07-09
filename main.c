#include "mbr.h"

#include <printf.h>
#include <stdlib.h>
#include <string.h>

void write_mbr(FILE *file_ptr) {
  mbr_entry_t pmbr_partition = {
      .boot_indicator = 0,    // any value other than 0 is non-compliant
      .partition_type = 0xEE, // protective EFI GPT
      .first_sector = {0x00, 0x02, 0x00},
      .last_sector = {0xFF, 0xFF, 0xFF},
      .first_lba = 1, // LBA 1
      .sectors_count = LOGICAL_BLOCK_MAX - 1,
  };

  mbr_t mbr = {
      .partition = {pmbr_partition, 0x00, 0x00, 0x00},
      .boot_signature = {0x55, 0xAA},
  };

  fwrite(&mbr, LOGICAL_BLOCK_SIZE, 1, file_ptr);
}

void write_primary_gpt_header(FILE *file_ptr) {
  char buffer[LOGICAL_BLOCK_SIZE];
  memset(buffer, '\0', LOGICAL_BLOCK_SIZE);

  for (int i = 0; i < GPT_LBA_COUNT; ++i) {
    fwrite(buffer, LOGICAL_BLOCK_SIZE, 1, file_ptr);
  }
}

void write_secondary_gpt_header(FILE *file_ptr) {
  char buffer[LOGICAL_BLOCK_SIZE];
  memset(buffer, '\0', LOGICAL_BLOCK_SIZE);

  for (int i = 0; i < GPT_LBA_COUNT; ++i) {
    fwrite(buffer, LOGICAL_BLOCK_SIZE, 1, file_ptr);
  }
}

void write_blank(FILE *file_ptr) {
  char buffer[LOGICAL_BLOCK_SIZE];
  memset(buffer, '\0', LOGICAL_BLOCK_SIZE);

  // empty blocks count equals total blocks - (primary and secondary gpt blocks) - one mbr block
  int count = LOGICAL_BLOCK_MAX - (GPT_LBA_COUNT * 2) - 1;

  for (int i = 0; i < count; ++i) {
    fwrite(buffer, LOGICAL_BLOCK_SIZE, 1, file_ptr);
  }
}

int main() {
  FILE *file_ptr = fopen(DISK_FILE_NAME, "we");

  if (file_ptr == NULL) {
    fprintf(stderr, "Could not create %s!\n", DISK_FILE_NAME);
    exit(1);
  }

  write_mbr(file_ptr);
  write_primary_gpt_header(file_ptr);
  write_blank(file_ptr);
  write_secondary_gpt_header(file_ptr);

  fclose(file_ptr);

  return 0;
}
