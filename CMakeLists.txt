cmake_minimum_required (VERSION 2.8)

set(CMAKE_C_FLAGS "-Wall -g")
set(CMAKE_EXPORT_COMPILE_COMMANDS "on")

project(broadcast)

add_executable (chat chat.c)

target_link_libraries(chat pthread)
