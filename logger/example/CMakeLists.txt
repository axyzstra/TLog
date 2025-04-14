cmake_minimum_required(VERSION 3.20)

project(test)

include_directories(../utils)
include_directories(../mmap)

set(MMAP_SRCS ../mmap/mmap_aux.cc ../mmap/mmap_linux.cc)
set(UTIL_SRCS ../utils/file_util.cc ../utils/sys_util_linux.cc)

add_executable(app main.cc ${MMAP_SRCS} ${UTIL_SRCS})
  