cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_STANDARD 11)

project(syscall_identification CXX)

add_executable(identify_syscalls main.cpp Syscall.cpp)

include_directories(/usr/local/include)

target_compile_options(identify_syscalls PRIVATE -g)

target_link_libraries(identify_syscalls parseAPI dyninstAPI instructionAPI symtabAPI common)
