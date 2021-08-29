# GPTDiskCreator
This is a GPT (GUID Partition Table) virtual disk file creator library that follows the
UEFI Specification Version 2.9 (March 2021) document accessible from [here](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_9_2021_03_18.pdf).

## How to Build
In the root directory of the repository, run the following commands:

```
cmake -S src -B build
cmake --build build
```

Now you'll have an executable which you can run by executing:

```
./build/ThatDiskCreator
```

Which will generate a file called `disk.hdd` in the same directory it was run from.

You can change the parameters for the output in `src/main.c`.

## The API
This library takes in a struct of type `disk_options_t` and outputs a virtual disk file.

The parameters are as follows:

```c
typedef struct {
  // 0 indicates "inferred", max size of about 4 terabytes, i.e. 4*1024^2 megabytes
  disk_size_b_t disk_size_b;
  
  // 512 is the only supported value, however multiples of 512 up to 4096 are allowed only experimentally
  block_size_b_t logical_block_size_b;
  
  // array of partition sizes in megabytes, max partition size of 8 terabytes
  partition_size_b_t partition_sizes_b[GPT_PARTITION_ARRAY_LENGTH];
  
  // index of EFI System Partition in the partition array, negative values indicate no ESP
  partition_index_t efi_system_partition_index;
  
  // index of Partition with its boot flag set, negative values indicate no partitions with boot flag
  partition_index_t boot_partition_index;
  
} disk_options_t;
```


