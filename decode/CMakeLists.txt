cmake_minimum_required(VERSION 3.20)
project(decode)

find_package(Protobuf CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)


# 固定搭配，用于生成 .pb.h 和 .pb.cc
set(PROTO_FILE /home/axyz/usr/logger/logger/proto/effective_msg.proto)
get_filename_component(PROTO_DIR ${PROTO_FILE} DIRECTORY)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PROTO_DIR}
  ../logger/compress
  ../logger/context
  ../logger/crypt
  ../logger/formatter
  ../logger/mmap
  ../logger/sinks
  ../logger/utils
  ../logger
)

add_executable(test
  decode.cc
  decode_formatter.cc
  ../logger/compress/zstd_compress.cc
  ../logger/context/context.cc 
  ../logger/context/executor.cc 
  ../logger/context/thread_pool.cc
  ../logger/crypt/aes_crypt.cc
  ../logger/crypt/crypt.cc
  ../logger/formatter/effective_formatter.cc
  ../logger/mmap/mmap_aux.cc
  ../logger/mmap/mmap_linux.cc
  ../logger/sinks/effective_sink.cc
  ../logger/utils/sys_util_linux.cc
  ../logger/utils/file_util.cc
  ../logger/log_handle.cc
  ../logger/log_msg.cc
  ${PROTO_SRCS} 
)

target_link_libraries(test PRIVATE
                      protobuf::libprotobuf
                      zstd::libzstd
                      cryptopp::cryptopp
)