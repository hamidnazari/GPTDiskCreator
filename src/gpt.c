#include "gpt.h"
#include <string.h>

void populate_gpt_header(gpt_header_t *header_out, lba_t header_lba, lba_t backup_lba, lba_t first_lba, lba_t last_lba, crc_32_t partition_entries_crc_32) {
  gpt_header_t header = {
      .signature = "EFI PART",
      .revision = {0x00, 0x00, 0x01, 0x00},
      .header_size = {GPT_HEADER_SIZE_B},
      .header_lba = header_lba,
      .backup_lba = backup_lba,
      .first_usable_lba = first_lba,
      .last_usable_lba = last_lba,
      .partition_entries_lba = 2,
      .disk_guid = get_random_guid(),
      .partitions_count = {GPT_PARTITION_ARRAY_LENGTH},
      .partition_entry_size = sizeof(gpt_partition_t),
      .partition_entries_crc_32 = partition_entries_crc_32};

  header.header_crc_32 = calculate_crc_32((uint8_t *) &header, GPT_HEADER_SIZE_B);

  memcpy(header_out, &header, sizeof(gpt_header_t));
}

void populate_gpt_backup_header(gpt_header_t *header_out, const gpt_header_t *header) {
  memcpy(header_out, header, sizeof(gpt_header_t));

  header_out->header_crc_32 = 0;
  header_out->header_lba = header->backup_lba;
  header_out->backup_lba = header->header_lba;
  header_out->partition_entries_lba = header->last_usable_lba + 1;
  header_out->header_crc_32 = calculate_crc_32((uint8_t *) header_out, GPT_HEADER_SIZE_B);
}

static void format_partition_number(partition_name_t name, partition_index_t i) {
  if (i >= 100) name[10] = 0x30 + i / 100;
  if (i >= 10) name[11] = 0x30 + i / 10;
  name[12] = 0x30 + i;
}

void populate_gpt_partition(gpt_partition_t *partition_out, lba_t first_lba, lba_t last_lba, uint8_t boot_flag, bool is_esp, partition_index_t name_index) {
  gpt_partition_t partition = {
      .type_guid = parse_guid(is_esp ? GUID_EFI_SYSTEM_PARTITION : GUID_MICROSOFT_BASIC_DATA_PARTITION),
      .partition_guid = get_random_guid(),
      .first_lba = first_lba,
      .last_lba = last_lba,
      .attributes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 | boot_flag},
      .partition_name = u"Partition ",
  };

  format_partition_number(partition.partition_name, name_index);

  memcpy(partition_out, &partition, sizeof(gpt_partition_t));
}
