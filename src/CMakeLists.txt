cmake_minimum_required(VERSION 3.19)
project(ThatDiskCreator C)

set(CMAKE_C_STANDARD 11)

add_executable(ThatDiskCreator crc_32.c disk.c fat_32.c gpt.c guid.c main.c)
