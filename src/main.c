#include "disk.h"

int main() {
  const disk_size_b_t disk_size = mb(256);
  const partition_size_b_t efi_size = mb(40);

  disk_options_t disk = {
      .logical_block_size_b = 512,
      .disk_size_b = disk_size,
      .efi_system_partition_index = -1,
      .boot_partition_index = 0,
      .partition_sizes_b = {
          efi_size,
          disk_size - efi_size - GPT_RESERVED_B,
      },
  };

  errors_e exit_code = create_disk_image("disk.hdd", &disk);

  switch (exit_code) {
  case DISK_OPERATION_SUCCESS:
    printf("Virtual disk file creation successful.");
    break;
  case DISK_OPTIONS_INVALID_DISK_SIZE:
    fprintf(stderr, "Error: invalid disk size requested.");
    break;
  case DISK_OPTIONS_INVALID_LOGICAL_BLOCK_SIZE:
    fprintf(stderr, "Error: invalid logical block size requested.");
    break;
  case DISK_OPTIONS_INVALID_PARTITION_SIZES:
    fprintf(stderr, "Error: invalid partition size requested.");
    break;
  case DISK_OPTIONS_INVALID_ESP_INDEX:
    fprintf(stderr, "Error: requested ESP index is invalid.");
    break;
  case DISK_OPTIONS_INVALID_BOOT_PARTITION_INDEX:
    fprintf(stderr, "Error: requested boot partition index is invalid.");
    break;
  case DISK_FILE_ERROR:
    fprintf(stderr, "Error: could not not create virtual disk file.");
    break;
  case DISK_WRITE_ERROR:
    fprintf(stderr, "Error: could not not write to virtual disk file.");
    break;
  }

  return exit_code;
}
