cmake_minimum_required(VERSION 3.20)
project(test)

include_directories(../formatter)
include_directories(../sinks)
include_directories(../utils)
include_directories(../)
add_executable(test test_handle.cc
                    ../formatter/default_formatter.cc
                    ../sinks/console_sink.cc
                    ../log_handle.cc
                    ../utils/sys_util_linux.cc
                    ../log_msg.cc
                    )
