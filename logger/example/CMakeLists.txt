# cmake_minimum_required(VERSION 3.20)

# project(test)

# include_directories(../utils)
# include_directories(../mmap)

# set(MMAP_SRCS ../mmap/mmap_aux.cc ../mmap/mmap_linux.cc)
# set(UTIL_SRCS ../utils/file_util.cc ../utils/sys_util_linux.cc)

# add_executable(app main.cc ${MMAP_SRCS} ${UTIL_SRCS})

# ==========================================

# cmake_minimum_required(VERSION 3.20)
# project(test)

# # 查找 GoogleTest
# find_package(GTest REQUIRED)
# include_directories(../)

# # 添加测试可执行文件
# add_executable(test test_msg.cc)

# # 链接库
# target_link_libraries(test GTest::gtest_main)


# ==============test_msg cmake==============================

# cmake_minimum_required(VERSION 3.20)
# project(test)

# find_package(fmt REQUIRED)

# add_definitions(-DENABLE_LOG)
# include_directories(../)
# add_executable(test test.cc)

# target_link_libraries(test fmt::fmt)

# ==============cmake test_handle==============================

# cmake_minimum_required(VERSION 3.20)
# project(test)

# include_directories(../formatter)
# include_directories(../sinks)
# include_directories(../utils)
# include_directories(../)
# add_executable(test test_handle.cc
#                     ../formatter/default_formatter.cc
#                     ../sinks/console_sink.cc
#                     ../log_handle.cc
#                     ../utils/sys_util_linux.cc
#                     ../log_msg.cc
#                     )


# ==============cmake test_compress==============================

cmake_minimum_required(VERSION 3.20)
project(test)

find_package(ZLIB REQUIRED)
# find_package(zstd CONFIG REQUIRED)

include_directories(../compress)
include_directories(../)
add_executable(test test_compress.cc
                    ../compress/zlib_compress.cc
                    # ../compress/zstd_compress.cc
                    )
target_link_libraries(test PRIVATE ZLIB::ZLIB)
# target_link_libraries(test PRIVATE zstd::libzstd)
