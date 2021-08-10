#ifndef THATDISKCREATOR__DISK_H
#define THATDISKCREATOR__DISK_H

#include "basic_types.h"
#include "decl.h"
#include "gpt.h"
#include <stdio.h>

typedef struct {
  // 0 indicates "inferred", max size of about 4 terabytes, i.e. 4*1024^2 megabytes
  disk_size_b_t disk_size_b;
  // supports up to 4096 bytes
  block_size_b_t logical_block_size_b;
  // array of partition sizes in megabytes, max partition size of 8 terabytes
  partition_size_b_t partition_sizes_b[GPT_PARTITION_ARRAY_LENGTH];
  // index of ESP in the partition array, negative values indicate no ESP
  partition_index_t esp_index;
} disk_options_t;

int8_t create_disk_image(const char *file_name, const disk_options_t *options);

#endif //THATDISKCREATOR__DISK_H
