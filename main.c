#include "write.h"
#include <printf.h>
#include <stdlib.h>


int main() {
  if (LOGICAL_BLOCK_MAX > LOGICAL_BLOCK_PRACTICAL_MAX) {
    fprintf(stderr, "Disk size and block size too large for this program.\n");
    exit(1);
  }

  FILE *file_ptr = fopen(DISK_FILE_NAME, "we");

  if (file_ptr == NULL) {
    fprintf(stderr, "Could not create file: '%s'\n", DISK_FILE_NAME);
    exit(1);
  }

  write_mbr(file_ptr);
  write_gpt(file_ptr);
  write_volumes(file_ptr);

  fclose(file_ptr);

  return 0;
}
