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

# ==============test_tp cmake==============================

cmake_minimum_required(VERSION 3.20)
project(test)

include_directories(../context)
add_executable(test test_con.cc
                    ../context/thread_pool.cc
                    ../context/executor.cc
                    ../context/context.cc
                    )