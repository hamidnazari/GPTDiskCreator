#include "disk.h"
#include "crc_32.h"
#include "mbr.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// half for header itself, the other half for its backup
#define FAT_32_VOLUMES_LBA (GPT_RESERVED_B / GPT_BLOCK_SIZE_MAX_B / 2)

static partition_index_t get_valid_partitions_count(const disk_options_t *options) {
  partition_index_t i;

  for (i = 0; i < GPT_PARTITION_ARRAY_LENGTH; ++i) {
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

static errors_e verify_disk_options(const disk_options_t *options) {
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

  if (options->logical_block_size_b % 512 != 0) {
    return DISK_OPTIONS_INVALID_BLOCK_SIZE;
  }

  if (options->logical_block_size_b < GPT_BLOCK_SIZE_MIN_B || options->logical_block_size_b > GPT_BLOCK_SIZE_MAX_B) {
    return DISK_OPTIONS_INVALID_BLOCK_SIZE;
  }

  partition_index_t valid_partitions_count = get_valid_partitions_count(options);
  if (valid_partitions_count <= 0) {
    return DISK_OPTIONS_INVALID_PARTITION_SIZES;
  }

  return DISK_SUCCESS;
}

static size_t write(FILE *file_ptr, const void *ptr, size_t size, block_size_b_t block_size_b) {
  const size_t nitems = 1;
  const size_t count = fwrite(ptr, size, nitems, file_ptr);

  if (block_size_b > size) {
    static const char null_block[512] = {0};
    for (uint8_t i = 0; i < (block_size_b / size) - 1; ++i) {
      fwrite(null_block, sizeof(null_block), nitems, file_ptr);
    }
  }

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
  mbr_t mbr;
  populate_mbr(&mbr, options->disk_size_b, options->logical_block_size_b);

  write(file_ptr, &mbr, sizeof(mbr_t), options->logical_block_size_b);
}

static void populate_partition_array(gpt_partition_array_t partition_array_out, const disk_options_t *options) {
  lba_t next_lba = FAT_32_VOLUMES_LBA;

  for (partition_index_t i = 0; i < get_valid_partitions_count(options); ++i) {
    bool is_esp = (i == options->efi_system_partition_index);
    uint8_t boot_flag = (i == options->boot_partition_index) ? 1 : 0;

    const lba_t last_lba = get_block_last_lba(next_lba, options->partition_sizes_b[i], options->logical_block_size_b);

    gpt_partition_t partition;
    populate_gpt_partition(&partition, next_lba, last_lba, boot_flag, is_esp, i);

    memcpy(&partition_array_out[i], &partition, sizeof(gpt_partition_t));

    next_lba = last_lba + 1;
  }
}

static void write_gpt(FILE *file_ptr, const disk_options_t *options) {
  const lba_t disk_last_lba = get_disk_last_lba(options->disk_size_b, options->logical_block_size_b); // remove
  const uint8_t first_usable_lba = get_gpt_lba_count(options->logical_block_size_b);
  const lba_t last_usable_lba = disk_last_lba - get_backup_gpt_lba_count(options->logical_block_size_b);

  gpt_partition_array_t partition_array = {0};
  populate_partition_array(partition_array, options);
  crc_32_t partitions_crc_32 = calculate_crc_32((uint8_t *) &partition_array, sizeof(partition_array));

  // GPT Header
  gpt_header_t header;
  populate_gpt_header(&header, 1, disk_last_lba, first_usable_lba, last_usable_lba, partitions_crc_32);

  write(file_ptr, &header, sizeof(gpt_header_t), options->logical_block_size_b);

  // GPT entries
  for (partition_index_t i = 0; i < GPT_PARTITION_ARRAY_LENGTH; ++i) {
    write(file_ptr, &partition_array[i], sizeof(gpt_partition_t), 0);
  }

  seek_lba(file_ptr, options, get_backup_gpt_lba_count(options->logical_block_size_b), true);

  // Backup GPT entries
  for (partition_index_t i = 0; i < GPT_PARTITION_ARRAY_LENGTH; ++i) {
    write(file_ptr, &partition_array[i], sizeof(gpt_partition_t), 0);
  }

  gpt_header_t backup_header;
  populate_gpt_backup_header(&backup_header, &header);

  // Backup GPT header
  write(file_ptr, &backup_header, sizeof(gpt_header_t), options->logical_block_size_b);
}

static void write_volume(FILE *file_ptr, const disk_options_t *options, lba_t offset_lba, partition_index_t partition_index, char *label) {
  fat_32_ebpb_t ebpb;
  fat_32_fsinfo_t fsinfo;

  // Extended BIOS Parameter Block
  populate_fat_32_ebpb(&ebpb, options->partition_sizes_b[partition_index], options->logical_block_size_b, label);

  seek_lba(file_ptr, options, offset_lba, false);
  write(file_ptr, &ebpb, sizeof(fat_32_ebpb_t), options->logical_block_size_b);

  // FS Information Sector
  populate_fat_32_fsinfo(&fsinfo, &ebpb);

  write(file_ptr, &fsinfo, sizeof(fat_32_fsinfo_t), options->logical_block_size_b);

  // Backup Boot Sector
  seek_lba(file_ptr, options, offset_lba + ebpb.backup_boot_sector_cluster_number, false);
  write(file_ptr, &ebpb, sizeof(fat_32_ebpb_t), options->logical_block_size_b);

  // FAT Region
  uint32_t clusters[3] = {
      0x0FFFFF00 | ebpb.media_descriptor,
      0x0FFFFFFF,
      0x0FFFFFF8, // end-of-file for root directory
  };

  lba_t fat_region_lba = offset_lba + ebpb.reserved_sectors_count;

  for (uint8_t i = 0; i < ebpb.number_of_fats; ++i) {
    lba_t fat_lba = ebpb.sectors_per_fat_32 * i;
    seek_lba(file_ptr, options, fat_region_lba + fat_lba, false);
    write(file_ptr, &clusters, sizeof(clusters), 0);
  }
}

static void write_volumes(FILE *file_ptr, const disk_options_t *options) {
  lba_t next_lba = FAT_32_VOLUMES_LBA;
  label_t label = {0};

  for (partition_index_t i = 0; i < get_valid_partitions_count(options); ++i) {
    sprintf(label, "Volume %-3d", i);
    write_volume(file_ptr, options, next_lba, i, label);

    next_lba = get_block_last_lba(next_lba, options->partition_sizes_b[i], options->logical_block_size_b) + 1;
  }
}

errors_e create_disk_image(const char *file_name, const disk_options_t *options) {
  srand(time(NULL));

  int8_t options_verification = verify_disk_options(options);

  if (options_verification < 0) {
    return options_verification;
  }

  FILE *file_ptr = fopen(file_name, "w");

  if (file_ptr == NULL) {
    return DISK_FILE_ERROR;
  }

  write_mbr(file_ptr, options);
  write_gpt(file_ptr, options);
  write_volumes(file_ptr, options);

  fclose(file_ptr);

  return DISK_SUCCESS;
}
