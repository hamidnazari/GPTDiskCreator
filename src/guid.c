#include "guid.h"
#include <stdlib.h>
#include <string.h>

static guid_t create_guid(const guid_bytestream_t stream) {
  guid_t guid = {
      .time_low = stream[0] << 24 | stream[1] << 16 | stream[2] << 8 | stream[3], // should be encoded as little endian
      .time_middle = stream[4] << 8 | stream[5], // should be encoded as little endian
      .time_high_and_version = stream[6] << 8 | stream[7], // should be encoded as little endian
      .clock_seq_high_and_reserved = stream[8],
      .clock_sequence_low = stream[9],
      .node = {stream[10], stream[11], stream[12], stream[13], stream[14], stream[15]},
  };

  return guid;
}

guid_t parse_guid(guid_string_t guid_string) {
  guid_bytestream_t stream;

  for (size_t i = 0, j = 0; i < strlen(guid_string); i += 2, ++j) {
    if (guid_string[i] == '-') ++i;

    char byte[3] = {0};
    strncpy(byte, guid_string + i, 2);

    stream[j] = strtol(byte, NULL, 16);
  }

  return create_guid(stream);
}

// https://datatracker.ietf.org/doc/html/rfc4122#section-4.4
guid_t get_random_guid() {
  guid_bytestream_t stream;

  for (size_t i = 0; i < sizeof(stream); ++i) {
    stream[i] = rand() % (UINT8_MAX + 1);
  }

  guid_t guid = create_guid(stream);

  // set the two most significant bits (bits 6 and 7) to zero and one, respectively.
  guid.clock_seq_high_and_reserved = (2 << 6) | (guid.clock_seq_high_and_reserved >> 2);

  // set the four most significant bits (bits 12 through 15) of the 4-bit version number.
  guid.time_high_and_version = (4 << 12) | (guid.time_high_and_version >> 4);

  return guid;
}

