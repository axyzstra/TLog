cmake_minimum_required(VERSION 3.20)
project(test)


find_package(Protobuf CONFIG REQUIRED)
find_package(GTest REQUIRED)

# 固定搭配，用于生成 .pb.h 和 .pb.cc
set(PROTO_FILE ../proto/effective_msg.proto)
get_filename_component(PROTO_DIR ${PROTO_FILE} DIRECTORY)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

include_directories(../
                  ${CMAKE_CURRENT_BINARY_DIR}
                  ${PROTO_DIR}
)
add_executable(test test_protobuf.cc
                    ../log_msg.cc
                    ${PROTO_SRCS} 
                    ${PROTO_HDRS}
                    )

target_link_libraries(test PRIVATE 
                      protobuf::libprotobuf
                      GTest::GTest
                      GTest::Main
)
