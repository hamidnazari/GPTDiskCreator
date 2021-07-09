#ifndef THATDISKCREATOR__GUID_H
#define THATDISKCREATOR__GUID_H

#include <stdint.h>

// INFO about EFI GUID PDF 2.9 Specs page 2178 - 2179
// We have to generate this based off the criteria listed in the UEFI 2.9 spec.

// 16 bytes long
typedef struct {
  uint8_t time_low[4];
  uint8_t time_middle[2];
  uint8_t time_high_and_version[2];
  uint8_t clock_seq_high_and_reserved;
  uint8_t clock_sequence_low;
  uint8_t node[6];

} __attribute__((packed)) guid_t;

#endif // THATDISKCREATOR__GUID_H
