#include "fat_32.h"
#include <time.h>

uint32_t get_fat_size(uint32_t size, uint16_t reserved_sectors_count, uint8_t sectors_per_cluster, uint8_t number_of_fats) {
  // http://msdn.microsoft.com/en-us/windows/hardware/gg463080.aspx
  uint32_t magic = (128 * sectors_per_cluster) + number_of_fats / 2;
  uint32_t fat_size = (size - reserved_sectors_count + magic - 1) / magic;
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

uint16_t get_cluster_size(partition_size_b_t partition_size) {
  if (partition_size <= mb(64))
    return 512;
  if (partition_size <= mb(128))
    return kb(1);
  if (partition_size <= mb(256))
    return kb(2);
  if (partition_size <= gb(8))
    return kb(8);
  if (partition_size <= gb(16))
    return kb(16);
  return kb(32);
}
