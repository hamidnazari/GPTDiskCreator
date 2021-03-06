#ifndef THATDISKCREATOR__DISK_H
#define THATDISKCREATOR__DISK_H

#include "basic_types.h"
#include "fat_32.h"
#include "gpt.h"
#include <stdio.h>

// TODO: determine minimum size: 33MB, e.g. 32MB (block size 512) + 1MB for MBR+GPT
#define DISK_SIZE_MIN_MB (FAT_32_VOLUME_SIZE_MIN_MB + GPT_RESERVED_MB)
#define DISK_SIZE_MIN_B mb(DISK_SIZE_MIN_MB)

#define DISK_SIZE_MAX_MB (GPT_PARTITION_ARRAY_LENGTH * FAT_32_VOLUME_SIZE_MAX_MB + GPT_RESERVED_MB)
#define DISK_SIZE_MAX_B mb(DISK_SIZE_MAX_MB)

typedef enum {
  DISK_OPERATION_SUCCESS = 0,
  DISK_OPTIONS_INVALID_DISK_SIZE = -1,
  DISK_OPTIONS_INVALID_LOGICAL_BLOCK_SIZE = -2,
  DISK_OPTIONS_INVALID_PARTITION_SIZES = -3,
  DISK_OPTIONS_INVALID_ESP_INDEX = -4,
  DISK_OPTIONS_INVALID_BOOT_PARTITION_INDEX = -5,
  DISK_FILE_ERROR = -6,
  DISK_WRITE_ERROR = -7,
} errors_e;

typedef struct {
  // 0 indicates "inferred", max size of about 4 terabytes, i.e. 4*1024^2 megabytes
  disk_size_b_t disk_size_b;
  // 512 is the only supported value, however multiples of 512 up to 4096 are allowed only experimentally
  block_size_b_t logical_block_size_b;
  // array of partition sizes in megabytes, max partition size of 8 terabytes
  partition_size_b_t partition_sizes_b[GPT_PARTITION_ARRAY_LENGTH];
  // index of EFI System Partition in the partition array, negative values indicate no ESP
  partition_index_t efi_system_partition_index;
  // index of Partition with its boot flag set, negative values indicate no partitions with boot flag
  partition_index_t boot_partition_index;
} disk_options_t;

errors_e create_disk_image(const char *file_name, const disk_options_t *options);

#endif //THATDISKCREATOR__DISK_H
