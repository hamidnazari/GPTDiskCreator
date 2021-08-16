#include "fat_32.h"
#include <string.h>
#include <time.h>

uint32_t get_fat_32_size(uint32_t total_sectors_count, uint16_t reserved_sectors_count, uint8_t sectors_per_cluster, uint8_t number_of_fats) {
  // http://msdn.microsoft.com/en-us/windows/hardware/gg463080.aspx
  uint32_t magic = (128 * sectors_per_cluster) + number_of_fats / 2;
  uint32_t fat_size = (total_sectors_count - reserved_sectors_count + magic - 1) / magic;
  return fat_size;
}

uint32_t get_serial_number() {
  time_t timestamp = time(NULL);
  struct tm *now = localtime(&timestamp);

  uint16_t top_word_1 = ((now->tm_hour & 0xFF) << 8) | (now->tm_min & 0xFF);
  uint16_t top_word_2 = 1900 + (now->tm_year & 0xFF);

  uint16_t bottom_word_1 = (((now->tm_mon + 1) & 0xFF) << 8) | (now->tm_mday & 0xFF);
  uint16_t bottom_word_2 = (now->tm_sec & 0xFF) << 8;

  return ((top_word_1 + top_word_2) << 16) | (bottom_word_1 + bottom_word_2);
}

uint16_t get_cluster_size(partition_size_b_t partition_size_b) {
  if (partition_size_b <= mb(64)) return 512;
  if (partition_size_b <= mb(128)) return kb(1);
  if (partition_size_b <= mb(256)) return kb(2);
  if (partition_size_b <= gb(8)) return kb(8);
  if (partition_size_b <= gb(16)) return kb(16);
  return kb(32);
}

void populate_fat_32_ebpb(fat_32_ebpb_t *ebpb_out,
                          partition_size_b_t partition_size_b,
                          block_size_b_t sector_size_b,
                          label_t label) {
  fat_32_ebpb_t ebpb = {
      .jump_boot = {0xEB, 0x58, 0x90},
      .oem = "ThatOS64",
      .bytes_per_sector = sector_size_b,
      .sectors_per_cluster = get_cluster_size(partition_size_b) / sector_size_b,
      .reserved_sectors_count = 32,
      .number_of_fats = 2,
      .media_descriptor = 0xF8,
      .sectors_per_track = 32,
      .number_of_heads = 64,
      .large_total_sectors_count = partition_size_b / sector_size_b,
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

  ebpb.sectors_per_fat_32 = get_fat_32_size(ebpb.large_total_sectors_count,
                                            ebpb.reserved_sectors_count,
                                            ebpb.sectors_per_cluster,
                                            ebpb.number_of_fats);

  memcpy(ebpb_out, &ebpb, sizeof(fat_32_ebpb_t));
}

static inline uint32_t get_free_cluster_count(const fat_32_ebpb_t *ebpb) {
  return ebpb->large_total_sectors_count - ebpb->reserved_sectors_count - (ebpb->sectors_per_fat_32 * ebpb->number_of_fats) - 1;
}

void populate_fat_32_fsinfo(fat_32_fsinfo_t *fsinfo_out, const fat_32_ebpb_t *ebpb) {
  fat_32_fsinfo_t fsinfo = {
      .lead_signature = 0x41615252,
      .struct_signature = 0x61417272,
      .free_cluster_count = get_free_cluster_count(ebpb),
      .next_free_cluster = 2,
      .trail_signature = 0xAA550000,
  };

  memcpy(fsinfo_out, &fsinfo, sizeof(fat_32_fsinfo_t));
}
