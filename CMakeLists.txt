cmake_minimum_required(VERSION 3.12)
project(async_tools)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
add_compile_options("-DASYNC_TASK_DEBUG")

add_executable(async_tools main.cpp async/stream/fwd.h async/stream/decl.h async/stream/impl.h async/task/task.hpp async/task/fwd.h async/task/decl.h async/task/impl.h async/stream/stream.hpp async/utils/utils.hpp async/utils/decl.h async/utils/impl.h utils.h console.h)