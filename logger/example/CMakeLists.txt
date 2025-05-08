cmake_minimum_required(VERSION 3.20)
project(test)

find_package(Protobuf CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# 固定搭配，用于生成 .pb.h 和 .pb.cc
set(PROTO_FILE ../proto/effective_msg.proto)
get_filename_component(PROTO_DIR ${PROTO_FILE} DIRECTORY)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PROTO_DIR}
  ../compress
  ../context
  ../crypt
  ../formatter
  ../mmap
  ../sinks
  ../utils
  ../
)


add_executable(test
  test_log_macro.cc
  ../compress/zstd_compress.cc
  ../context/context.cc 
  ../context/executor.cc 
  ../context/thread_pool.cc
  ../crypt/aes_crypt.cc
  ../crypt/crypt.cc
  ../formatter/effective_formatter.cc
  ../mmap/mmap_aux.cc
  ../mmap/mmap_linux.cc
  ../sinks/effective_sink.cc
  ../utils/sys_util_linux.cc
  ../utils/file_util.cc
  ../log_handle.cc
  ../log_msg.cc
  ../log_handle.cc
  ../log_factory.cc
  ${PROTO_SRCS} 
  ${PROTO_HDRS}
)


target_link_libraries(test PRIVATE
                      protobuf::libprotobuf
                      zstd::libzstd
                      cryptopp::cryptopp
                      fmt::fmt
)