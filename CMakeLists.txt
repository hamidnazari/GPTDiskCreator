cmake_minimum_required(VERSION 3.19)
project(ThatDiskCreator C)

set(CMAKE_C_STANDARD 11)

add_executable(ThatDiskCreator main.c gpt.h guid.h mbr.h decl.h write.h)
