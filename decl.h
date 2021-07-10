#ifndef THATDISKCREATOR__DECL_H
#define THATDISKCREATOR__DECL_H

#define DISK_SIZE_MB 1
#define DISK_SIZE (DISK_SIZE_MB * 1024 * 1024)
#define DISK_FILE_NAME "disk.hdd"

#define MBR_SIZE 512
#define GPT_HEADER_SIZE 92
// TODO: can be longer, min partition entries is 4
// TODO: replace GPT_LBA_COUNT with a variable
#define GPT_LBA_COUNT (1 + 32)

// TODO: support 1024, 2048, 4096, etc
#define LOGICAL_BLOCK_SIZE 512
#define LOGICAL_BLOCK_MAX (DISK_SIZE / LOGICAL_BLOCK_SIZE)
#define LOGICAL_BLOCK_THEORETICAL_MAX (-1ULL / LOGICAL_BLOCK_SIZE)

#endif //THATDISKCREATOR__DECL_H
