cmake_minimum_required (VERSION 3.8)
set(CMAKE_ASM_NASM_LINK_EXECUTABLE "ld <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf64")

project(sample3 CXX ASM_NASM)

add_executable (b "Injector.cpp" "Injector.h" "main.cpp")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(b PRIVATE -g -Fdwarf)
endif()