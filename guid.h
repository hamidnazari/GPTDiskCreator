#ifndef THATDISKCREATOR__GUID_H
#define THATDISKCREATOR__GUID_H

#include <stdint.h>
#include <string.h>
#include <uuid/uuid.h>

// INFO about EFI GUID PDF 2.9 Specs page 2178 - 2179
// We have to generate this based off the criteria listed in the UEFI 2.9 spec.

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

guid_t create_guid(const uuid_t uuid) {
  guid_t guid = {
      .time_low = uuid[0] << 24 | uuid[1] << 16 | uuid[2] << 8 | uuid[3], // should be encoded as little endian
      .time_middle = uuid[4] << 8 | uuid[5], // should be encoded as little endian
      .time_high_and_version = uuid[6] << 8 | uuid[7], // should be encoded as little endian
      .clock_seq_high_and_reserved = uuid[8],
      .clock_sequence_low = uuid[9],
      .node = {uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]},
  };

  return guid;
}

guid_t parse_guid(const uuid_string_t uuid_string) {
  uuid_t uuid;
  uuid_parse(uuid_string, uuid);

  return create_guid(uuid);
}

guid_t get_random_guid() {
  uuid_t uuid;
  uuid_generate_random(uuid);

  return create_guid(uuid);
}

#endif // THATDISKCREATOR__GUID_H
