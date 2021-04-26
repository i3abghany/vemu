cmake_minimum_required(VERSION 3.16)
project(EMU-V)

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

add_executable(emu ${SRC_FILES})

add_executable(test_emu
    ${SRC_FILES} src/Tester.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -Werror -Wextra -Wshadow -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code")

target_compile_definitions(test_emu PRIVATE -DTEST_ENV)
