#include "disk.h"

static inline disk_size_b_t mb(uint32_t size) {
  return size << 20;
}

int main() {
  disk_options_t disk = {
      .logical_block_size_b = 512,
      .disk_size_b = mb(100),
      .efi_system_partition_index = -1,
      .boot_partition_index = 0,
      .partition_sizes_b = {
          mb(40),
          mb(58),
      },
  };

  return create_disk_image("disk.hdd", &disk);
}
