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
)

include_directories(include)

add_executable(emu_dbg ${SRC_FILES})
add_executable(emu ${SRC_FILES})

add_executable(test_emu
    ${SRC_FILES} src/Tester.cpp
)

string(CONCAT ${CXX_FLAGS},
    "-Wall "
    "-Werror "
    "-Wextra "
    "-Wshadow "
    "-Wswitch-default "
    "-Wswitch-enum "
    "-Wconversion "
    "-Wunreachable-code ")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

target_compile_definitions(test_emu PRIVATE -DTEST_ENV)

target_compile_options(emu_dbg PRIVATE "-ggdb")

target_compile_options(test_emu PRIVATE "-ggdb")

target_compile_options(emu PRIVATE "-O3")
