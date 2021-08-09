#ifndef THATDISKCREATOR__WRITE_H
#define THATDISKCREATOR__WRITE_H


#include "crc_32.h"
#include "fat_32.h"
#include "gpt.h"
#include "guid.h"
#include "mbr.h"
#include <printf.h>
#include <stdlib.h>
#include <string.h>


static const uint64_t esp_first_lba_index = 2048; // TODO: replace with GPT_LBA_COUNT+1?


static size_t write(FILE *file_ptr, const void *ptr, size_t size) {
  const size_t nitems = 1;
  const size_t count = fwrite(ptr, size, nitems, file_ptr);

  if (count < nitems) {
    printf("Write to disk was unsuccessful!\n");
  }

  return count;
}


static void seek_lba(FILE *file_ptr, uint64_t index) {
  // FIXME: fseeko's offset is signed 64bit whereas LBA is unsigned 64bit. Trouble!
  fseeko(file_ptr, get_lba(index), SEEK_SET);
}


void write_mbr(FILE *file_ptr) {
  mbr_entry_t pmbr_partition = {
      .boot_indicator = 0, // any value other than 0 is non-compliant
      .partition_type = 0xEE, // protective EFI GPT
      .first_sector = {0x00, 0x02, 0x00}, // from right after the first 512 bytes
      .last_sector = {0xFF, 0xFF, 0xFF}, // all the way to the end of the disk
      .first_lba = 1,
      .sectors_count = LOGICAL_BLOCK_MAX - 1,
  };

  mbr_t mbr = {
      .partition = {pmbr_partition, 0x00, 0x00, 0x00},
      .boot_signature = {0x55, 0xAA},
  };

  write(file_ptr, &mbr, LOGICAL_BLOCK_SIZE_B);
}


void write_gpt(FILE *file_ptr) {
  const uint32_t partition_entry_size = sizeof(gpt_entry_t);
  const uint8_t partition_entries_count = GPT_PARTITION_ARRAY_SIZE_B / partition_entry_size;
  const uint64_t last_usable_lba = LOGICAL_BLOCK_MAX - GPT_LBA_COUNT - 1;

  gpt_header_t header = {
      .signature = "EFI PART",
      .revision = {0x00, 0x00, 0x01, 0x00},
      .header_size = GPT_HEADER_SIZE_B,
      .header_lba = 1,
      .backup_lba = LOGICAL_BLOCK_MAX - 1,
      .first_usable_lba = GPT_LBA_COUNT + 1,
      .last_usable_lba = last_usable_lba,
      .partition_entries_lba = 2,
      .disk_guid = get_random_guid(),
      .partitions_count = partition_entries_count,
      .partition_entry_size = partition_entry_size,
  };

  const uint64_t esp_last_lba_index = allocate_lba(esp_first_lba_index, ESP_SIZE_B) - 1;
  const uint64_t main_partition_last_lba_index = allocate_lba(esp_last_lba_index + 1, MAIN_VOLUME_SIZE_B) - 1;

  gpt_entry_t efi_system_partition = {
      .type_guid = parse_guid(GUID_EFI_SYSTEM_PARTITION),
      .partition_guid = get_random_guid(),
      .first_lba = esp_first_lba_index,
      .last_lba = esp_last_lba_index,
      .attributes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
      .partition_name = u"ESP",
  };

  gpt_entry_t main_partition = {
      .type_guid = parse_guid(GUID_MICROSOFT_BASIC_DATA_PARTITION),
      .partition_guid = get_random_guid(),
      .first_lba = esp_last_lba_index + 1, // FIXME: overflow when cluster size > logical block size
      .last_lba = main_partition_last_lba_index,
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
  write(file_ptr, &header, LOGICAL_BLOCK_SIZE_B);

  // GPT entries
  for (uint8_t i = 0; i < partition_entries_count; ++i) {
    write(file_ptr, &partitions[i], sizeof(gpt_entry_t));
  }
  seek_lba(file_ptr, -GPT_LBA_COUNT - 1);

  // Backup GPT entries
  for (uint8_t i = 0; i < partition_entries_count; ++i) {
    write(file_ptr, &partitions[i], sizeof(gpt_entry_t));
  }

  header.header_crc_32 = 0;
  header.header_lba = LOGICAL_BLOCK_MAX - 1;
  header.backup_lba = 1;
  header.partition_entries_lba = LOGICAL_BLOCK_MAX - GPT_LBA_COUNT;
  header.header_crc_32 = calculate_crc_32((uint8_t *) &header, GPT_HEADER_SIZE_B);

  // Backup GPT header
  write(file_ptr, &header, LOGICAL_BLOCK_SIZE_B);
}


static void write_volume(FILE *file_ptr, uint64_t offset, uint32_t size, char *label) {
  fat_32_ebpb_t ebpb = {
      .jump_boot = {0xEB, 0x58, 0x90},
      .oem = "ThatOS64",
      .bytes_per_sector = LOGICAL_BLOCK_SIZE_B,
      .sectors_per_cluster = FAT_32_CLUSTER_SIZE_B / LOGICAL_BLOCK_SIZE_B,
      .reserved_sectors_count = 32,
      .number_of_fats = 2,
      .media_descriptor = 0xF8,
      .sectors_per_track = 32,
      .number_of_heads = 64,
      .large_total_sectors_count = size / LOGICAL_BLOCK_SIZE_B,
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

  ebpb.sectors_per_fat_32 =
      get_fat_size(ebpb.large_total_sectors_count, ebpb.reserved_sectors_count, ebpb.sectors_per_cluster, ebpb.number_of_fats);

  fat_32_fsinfo_t fsinfo = {
      .lead_signature = 0x41615252,
      .struct_signature = 0x61417272,
      .free_cluster_count = ebpb.large_total_sectors_count - ebpb.reserved_sectors_count - (ebpb.sectors_per_fat_32 * ebpb.number_of_fats) - 1,
      .next_free_cluster = 2,
      .trail_signature = 0xAA550000,
  };

  seek_lba(file_ptr, offset);
  write(file_ptr, &ebpb, sizeof(ebpb));

  // FS Information Sector
  write(file_ptr, &fsinfo, sizeof(fsinfo));

  // Backup Boot Sector
  seek_lba(file_ptr, offset + ebpb.backup_boot_sector_cluster_number);
  write(file_ptr, &ebpb, sizeof(ebpb));

  uint32_t clusters[3] = {
      0x0FFFFF00 | ebpb.media_descriptor,
      0x0FFFFFFF,
      0x0FFFFFF8, // end-of-file for root directory
  };

  uint64_t fat_region_offset = offset + ebpb.reserved_sectors_count;

  // FAT Region
  for (uint8_t i = 0; i < ebpb.number_of_fats; ++i) {
    uint64_t fat_offset = ebpb.sectors_per_fat_32 * i;
    seek_lba(file_ptr, fat_region_offset + fat_offset);
    write(file_ptr, &clusters, sizeof(clusters));
  }
}

void write_volumes(FILE *file_ptr) {
  const uint64_t esp_last_lba_index = allocate_lba(esp_first_lba_index, ESP_SIZE_B) - 1;

  write_volume(file_ptr, esp_first_lba_index, ESP_SIZE_B, "ESP Volume ");
  write_volume(file_ptr, esp_last_lba_index + 1, MAIN_VOLUME_SIZE_B, "Main Volume");
}


#endif //THATDISKCREATOR__WRITE_H
