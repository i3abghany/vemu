cmake_minimum_required(VERSION 3.16)
project(vemu)

set(CMAKE_CXX_STANDARD 17)

set(SRC_FILES 
    src/main.cpp 
    src/InstructionDecoder.cpp 
    src/VEmu.cpp 
    src/Instruction.cpp 
    src/util.cpp 
    src/DRAM.cpp
    src/Bus.cpp 
    src/RegFile.cpp
    src/FRegFile.cpp
    src/CLIC.cpp
    src/PLIC.cpp
    src/UART.cpp
)

include_directories(include)

add_executable(emu ${SRC_FILES})

add_executable(test_emu
    ${SRC_FILES} src/Tester.cpp
)

string(APPEND CMAKE_CXX_FLAGS "-pthread ")
string(APPEND CMAKE_CXX_FLAGS "-Wall ")
string(APPEND CMAKE_CXX_FLAGS "-Werror ")
string(APPEND CMAKE_CXX_FLAGS "-Wextra ")
string(APPEND CMAKE_CXX_FLAGS "-Wshadow ")
string(APPEND CMAKE_CXX_FLAGS "-Wswitch-default ")
string(APPEND CMAKE_CXX_FLAGS "-Wswitch-enum ")
string(APPEND CMAKE_CXX_FLAGS "-Wconversion ")
string(APPEND CMAKE_CXX_FLAGS "-Wunreachable-code ")

target_compile_definitions(test_emu PRIVATE -DTEST_ENV)
target_compile_options(test_emu PRIVATE -ggdb)
target_compile_options(test_emu PRIVATE -fno-omit-frame-pointer)

target_compile_options(emu PRIVATE -O3)
