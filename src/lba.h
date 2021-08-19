#ifndef THATDISKCREATOR__LBA_H
#define THATDISKCREATOR__LBA_H

#include "basic_types.h"
#include <stdio.h>

typedef uint64_t lba_t;
typedef int64_t signed_lba_t;

// returns the last Logical Block Address on a given disk
lba_t get_disk_last_lba(disk_size_b_t disk_size, block_size_b_t logical_block_size);

// returns the last Logical Block Address index for a given size and offset
lba_t get_block_last_lba(lba_t offset, uint32_t size, block_size_b_t logical_block_size);

// returns disk offset for a given Logical Block Address
off_t translate_lba_to_offset(signed_lba_t lba, disk_size_b_t disk_size, block_size_b_t logical_block_size);

#endif //THATDISKCREATOR__LBA_H
