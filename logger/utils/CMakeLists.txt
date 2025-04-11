cmake_minimum_required(VERSION 3.20)

message(STATUS "Current system is: ${CMAKE_SYSTEM_NAME}")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(UTIL_SRCS sys_util_linux.cc)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(UTIL_SRCS sys_util_win.cc)
else()
  message(FATAL_ERROR "unsupported : ${CMAKE_SYSTEM_NAME}")
endif()

project(PageSize)

set(TEST_SRC test.cc)
set(FILE_SRC file_util.cc)

add_executable(app ${UTIL_SRCS} ${TEST_SRC} ${FILE_SRC})
