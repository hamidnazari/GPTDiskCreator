#ifndef THATDISKCREATOR__MBR_H
#define THATDISKCREATOR__MBR_H

#include <stdint.h>

// 16 bytes long
typedef struct {
  uint8_t boot_indicator;
  uint8_t first_sector[3];
  uint8_t partition_type;
  uint8_t last_sector[3];
  uint32_t first_lba;
  uint32_t sectors_count;
} __attribute__((packed)) __attribute__((aligned(16))) mbr_entry_t;

// 512 bytes long
typedef struct {
  uint8_t bootstrap[440]; // unused by UEFI
  uint8_t disk_signature[4]; // unused by UEFI. set to 0.
  uint8_t copy_protection[2]; // unused by UEFI. set to 0.
  mbr_entry_t partition[4]; // only the first partition is used by UEFI
  uint8_t boot_signature[2];
} __attribute__((packed)) __attribute__((aligned(512))) mbr_t;

#endif // THATDISKCREATOR__MBR_H
