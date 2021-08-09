#ifndef THATDISKCREATOR__DISK_H
#define THATDISKCREATOR__DISK_H

#include <stdio.h>

void write_mbr(FILE *file_ptr);

void write_gpt(FILE *file_ptr);

void write_volumes(FILE *file_ptr);

#endif //THATDISKCREATOR__DISK_H
