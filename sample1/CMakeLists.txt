# CMakeList.txt : sample1 の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.8)

# ソースをこのプロジェクトの実行可能ファイルに追加します。

set(CMAKE_ASM_NASM_LINK_EXECUTABLE "ld <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf64")

project(sample1 CXX ASM_NASM)

add_executable (a "hoge_asm.asm" "sample1.cpp" "sample1.h")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(a PRIVATE -g -Fdwarf)
endif()


# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
