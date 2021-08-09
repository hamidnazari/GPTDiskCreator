#ifndef THATDISKCREATOR__GUID_H
#define THATDISKCREATOR__GUID_H

#include <stdint.h>

#define GUID_EFI_SYSTEM_PARTITION "c12a7328-f81f-11d2-ba4b-00a0c93ec93b"
#define GUID_MICROSOFT_BASIC_DATA_PARTITION "ebd0a0a2-b9e5-4433-87c0-68b6b72699c7"

// 16 bytes long
typedef struct {
  uint32_t time_low; // should be encoded as little-endian
  uint16_t time_middle; // should be encoded as little-endian
  uint16_t time_high_and_version; // should be encoded as little-endian
  uint8_t clock_seq_high_and_reserved;
  uint8_t clock_sequence_low;
  uint8_t node[6];

} __attribute__((packed)) __attribute__((aligned(16))) guid_t;

guid_t parse_guid(const char *uuid_string);

guid_t get_random_guid();

#endif // THATDISKCREATOR__GUID_H
