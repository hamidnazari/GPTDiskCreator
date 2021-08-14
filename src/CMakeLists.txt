cmake_minimum_required(VERSION 3.19)
project(ThatDiskCreator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror")

add_executable(ThatDiskCreator crc_32.c disk.c fat_32.c gpt.c guid.c main.c)

