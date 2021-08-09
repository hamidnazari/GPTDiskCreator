#ifndef THATDISKCREATOR__DECL_H
#define THATDISKCREATOR__DECL_H

// TODO: determine minimum size: 33MB, e.g. 32MB (block size 512) + 1MB for MBR+GPT
#define DISK_SIZE_MB 100
#define DISK_SIZE_B (DISK_SIZE_MB * 1024 * 1024)
#define DISK_FILE_NAME "disk.hdd"

#define ESP_SIZE_B (40 * 1024 * 1024)
#define MAIN_VOLUME_SIZE_B (58 * 1024 * 1024)

#define MBR_SIZE_B 512
#define GPT_HEADER_SIZE_B 92
// TODO: can be longer, min partition entries is 4
// TODO: replace GPT_LBA_COUNT with a variable
#define GPT_LBA_COUNT (1 + 32)
#define GPT_PARTITION_ARRAY_SIZE_B (16 * 1024)

// TODO: add support 1024, 2048, 4096, etc
#define LOGICAL_BLOCK_SIZE_B 512
// total number of blocks we should fit in this GPT disk under creation
#define LOGICAL_BLOCK_MAX (DISK_SIZE_B / LOGICAL_BLOCK_SIZE_B)
// total number of blocks we can possibly fit in a GPT disk of this block size
#define LOGICAL_BLOCK_THEORETICAL_MAX (-1ULL / LOGICAL_BLOCK_SIZE_B)
// total number of blocks we can practically fit in this GPT disk due to this codebase limitations
#define LOGICAL_BLOCK_PRACTICAL_MAX ((-1ULL / LOGICAL_BLOCK_SIZE_B) >> 1)

#define DISK_SIZE_MAX_B (LOGICAL_BLOCK_PRACTICAL_MAX * LOGICAL_BLOCK_SIZE_B)

#define FAT_32_VOLUME_SIZE_MAX_GB 32
// TODO: add support for bigger disk sizes and hence cluster sizes
#define FAT_32_CLUSTER_SIZE_B 512

#endif //THATDISKCREATOR__DECL_H
