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

  return create_disk_image("disk.hdd", &disk);
}
