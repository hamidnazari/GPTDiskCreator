#include "disk.h"
#include "crc_32.h"
#include "guid.h"
#include "mbr.h"
#include "string.h"
#include <stdbool.h>

static const lba_t esp_first_lba = 2048; // TODO: replace with GPT_LBA_COUNT+1?

static partition_index_t get_valid_partitions_count(const disk_options_t *options) {
  partition_index_t i;

  for (i = 0; i <= GPT_PARTITION_ARRAY_LENGTH - 1; ++i) {
    if (options->partition_sizes_b[i] < FAT_32_VOLUME_SIZE_MIN_B || options->partition_sizes_b[i] > FAT_32_VOLUME_SIZE_MAX_B) {
      // stop counting as soon as an invalid partition is found
      break;
    }
  }

  return i;
}

static disk_size_b_t get_required_disk_size(const disk_options_t *options) {
  disk_size_b_t size = GPT_RESERVED_B;

  for (partition_index_t i = 0; i < get_valid_partitions_count(options); ++i) {
    size += options->partition_sizes_b[i];
  }

  return size;
}

static disk_size_b_t get_actual_disk_size(const disk_options_t *options) {
  return (options->disk_size_b > 0) ? options->disk_size_b : get_required_disk_size(options);
}

static errors_e verify_disk(const disk_options_t *options) {
  disk_size_b_t requested = get_actual_disk_size(options);
  disk_size_b_t required = get_required_disk_size(options);

  if (options->disk_size_b > 0 && options->disk_size_b <= DISK_SIZE_MIN_B) {
    return DISK_OPTIONS_INVALID_DISK_SIZE;
  }

  if (options->disk_size_b > DISK_SIZE_MAX_B) {
    return DISK_OPTIONS_INVALID_DISK_SIZE;
  }

  if (requested < required) {
    return DISK_OPTIONS_INVALID_DISK_SIZE;
  }

  uint8_t valid_partitions_count = get_valid_partitions_count(options);
  if (valid_partitions_count <= 0) {
    return DISK_OPTIONS_INVALID_PARTITION_SIZES;
  }

  return DISK_SUCCESS;
}

static size_t write(FILE *file_ptr, const void *ptr, size_t size) {
  const size_t nitems = 1;
  const size_t count = fwrite(ptr, size, nitems, file_ptr);

  if (count < nitems) {
    fprintf(stderr, "Write to disk was unsuccessful!\n");
  }

  return count;
}

static void seek_lba(FILE *file_ptr, const disk_options_t *options, lba_t lba, bool reverse) {
  // FIXME: unsigned lba to signed conversion.
  signed_lba_t signed_lba = reverse ? -lba : lba; // NOLINT(cppcoreguidelines-narrowing-conversions)
  disk_size_b_t disk_size = get_actual_disk_size(options);

  off_t offset = translate_lba_to_offset(signed_lba, disk_size, options->logical_block_size_b);

  fseeko(file_ptr, offset, SEEK_SET);
}

static void write_mbr(FILE *file_ptr, const disk_options_t *options) {
  mbr_entry_t pmbr_partition = {
      .boot_indicator = 0, // any value other than 0 is non-compliant
      .partition_type = 0xEE, // protective EFI GPT
      .first_sector = {0x00, 0x02, 0x00}, // from right after the first 512 bytes
      .last_sector = {0xFF, 0xFF, 0xFF}, // all the way to the end of the disk
      .first_lba = 1,
      .sectors_count = get_disk_last_lba(options->disk_size_b, options->logical_block_size_b),
  };

  mbr_t mbr = {
      .partition = {pmbr_partition, 0x00, 0x00, 0x00},
      .boot_signature = {0x55, 0xAA},
  };

  write(file_ptr, &mbr, options->logical_block_size_b);
}

static void write_gpt(FILE *file_ptr, const disk_options_t *options) {
  const uint8_t partition_entry_size = sizeof(gpt_entry_t);
  const uint8_t partition_entries_count = GPT_PARTITION_ARRAY_SIZE_B / partition_entry_size;
  const lba_t last_usable_lba = get_disk_last_lba(options->disk_size_b, options->logical_block_size_b) - GPT_LBA_COUNT;

  gpt_header_t header = {
      .signature = "EFI PART",
      .revision = {0x00, 0x00, 0x01, 0x00},
      .header_size = GPT_HEADER_SIZE_B,
      .header_lba = 1,
      .backup_lba = get_disk_last_lba(options->disk_size_b, options->logical_block_size_b),
      .first_usable_lba = GPT_LBA_COUNT + 1,
      .last_usable_lba = last_usable_lba,
      .partition_entries_lba = 2,
      .disk_guid = get_random_guid(),
      .partitions_count = partition_entries_count,
      .partition_entry_size = partition_entry_size,
  };

  const lba_t esp_last_lba = -1 + get_block_lba(esp_first_lba, options->partition_sizes_b[0], options->logical_block_size_b);

  const lba_t main_partition_last_lba = -1 + get_block_lba(esp_last_lba + 1, options->partition_sizes_b[1], options->logical_block_size_b);

  gpt_entry_t efi_system_partition = {
      .type_guid = parse_guid(GUID_EFI_SYSTEM_PARTITION),
      .partition_guid = get_random_guid(),
      .first_lba = esp_first_lba,
      .last_lba = esp_last_lba,
      .attributes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
      .partition_name = u"ESP",
  };

  gpt_entry_t main_partition = {
      .type_guid = parse_guid(GUID_MICROSOFT_BASIC_DATA_PARTITION),
      .partition_guid = get_random_guid(),
      .first_lba = esp_last_lba + 1, // FIXME: overflow when cluster size > logical block size
      .last_lba = main_partition_last_lba,
      .attributes = 0,
      .partition_name = u"Main Partition",
  };

  gpt_entry_t partitions[partition_entries_count] = {
      efi_system_partition,
      main_partition,
      0,
  };

  header.partition_entries_crc_32 = calculate_crc_32((uint8_t *) &partitions, sizeof(partitions));
  header.header_crc_32 = calculate_crc_32((uint8_t *) &header, GPT_HEADER_SIZE_B);

  // GPT Header
  write(file_ptr, &header, options->logical_block_size_b);

  // GPT entries
  for (uint8_t i = 0; i < partition_entries_count; ++i) {
    write(file_ptr, &partitions[i], sizeof(gpt_entry_t));
  }
  seek_lba(file_ptr, options, GPT_LBA_COUNT + 1, true);

  // Backup GPT entries
  for (uint8_t i = 0; i < partition_entries_count; ++i) {
    write(file_ptr, &partitions[i], sizeof(gpt_entry_t));
  }

  header.header_crc_32 = 0;
  header.header_lba = get_disk_last_lba(options->disk_size_b, options->logical_block_size_b);
  header.backup_lba = 1;
  header.partition_entries_lba =
      get_disk_last_lba(options->disk_size_b, options->logical_block_size_b) - GPT_LBA_COUNT + 1;
  header.header_crc_32 = calculate_crc_32((uint8_t *) &header, GPT_HEADER_SIZE_B);

  // Backup GPT header
  write(file_ptr, &header, options->logical_block_size_b);
}

static void write_volume(FILE *file_ptr, const disk_options_t *options, lba_t lba, partition_index_t partition_index, char *label) {
  fat_32_ebpb_t ebpb = {
      .jump_boot = {0xEB, 0x58, 0x90},
      .oem = "ThatOS64",
      .bytes_per_sector = options->logical_block_size_b,
      .sectors_per_cluster = FAT_32_CLUSTER_SIZE_B / options->logical_block_size_b,
      .reserved_sectors_count = 32,
      .number_of_fats = 2,
      .media_descriptor = 0xF8,
      .sectors_per_track = 32,
      .number_of_heads = 64,
      .large_total_sectors_count = options->partition_sizes_b[partition_index] / options->logical_block_size_b,
      .root_directory_cluster_number = 2,
      .fsinfo_sector_number = 1,
      .backup_boot_sector_cluster_number = 6,
      .drive_number = 0x80,
      .extended_boot_signature = 0x29,
      .serial_number = get_serial_number(),
      .system_identifier = "FAT32   ",
      .boot_signature = {0x55, 0xAA},
  };

  memcpy(&ebpb.volume_label, label, sizeof(ebpb.volume_label));

  ebpb.sectors_per_fat_32 = get_fat_size(ebpb.large_total_sectors_count, ebpb.reserved_sectors_count, ebpb.sectors_per_cluster, ebpb.number_of_fats);

  fat_32_fsinfo_t fsinfo = {
      .lead_signature = 0x41615252,
      .struct_signature = 0x61417272,
      .free_cluster_count = ebpb.large_total_sectors_count - ebpb.reserved_sectors_count - (ebpb.sectors_per_fat_32 * ebpb.number_of_fats) - 1,
      .next_free_cluster = 2,
      .trail_signature = 0xAA550000,
  };

  seek_lba(file_ptr, options, lba, false);
  write(file_ptr, &ebpb, sizeof(ebpb));

  // FS Information Sector
  write(file_ptr, &fsinfo, sizeof(fsinfo));

  // Backup Boot Sector
  seek_lba(file_ptr, options, lba + ebpb.backup_boot_sector_cluster_number, false);
  write(file_ptr, &ebpb, sizeof(ebpb));

  uint32_t clusters[3] = {
      0x0FFFFF00 | ebpb.media_descriptor,
      0x0FFFFFFF,
      0x0FFFFFF8, // end-of-file for root directory
  };

  lba_t fat_region_lba = lba + ebpb.reserved_sectors_count;

  // FAT Region
  for (uint8_t i = 0; i < ebpb.number_of_fats; ++i) {
    lba_t fat_lba = ebpb.sectors_per_fat_32 * i;
    seek_lba(file_ptr, options, fat_region_lba + fat_lba, false);
    write(file_ptr, &clusters, sizeof(clusters));
  }
}

static void write_volumes(FILE *file_ptr, const disk_options_t *options) {
  const lba_t esp_last_lba = -1 + get_block_lba(esp_first_lba, options->partition_sizes_b[0], options->logical_block_size_b) - 1;

  write_volume(file_ptr,
               options,
               esp_first_lba,
               0,
               "ESP Volume ");

  write_volume(file_ptr,
               options,
               esp_last_lba + 1,
               1,
               "Main Volume");
}

int8_t create_disk_image(const char *file_name, const disk_options_t *options) {
  int8_t verification_output = verify_disk(options);

  if (verification_output < 0) {
    return verification_output;
  }

  FILE *file_ptr = fopen(file_name, "we");

  if (file_ptr == NULL) {
    return DISK_FILE_ERROR;
  }

  write_mbr(file_ptr, options);
  write_gpt(file_ptr, options);
  write_volumes(file_ptr, options);

  fclose(file_ptr);

  return 0;
}
