#include "guid.h"
#include <uuid/uuid.h>

static guid_t create_guid(const uuid_t uuid) {
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

guid_t parse_guid(const char *uuid_string) {
  uuid_t uuid;
  uuid_parse(uuid_string, uuid);

  return create_guid(uuid);
}

guid_t get_random_guid() {
  uuid_t uuid;
  uuid_generate_random(uuid);

  return create_guid(uuid);
}
