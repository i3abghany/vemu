cmake_minimum_required(VERSION 3.16)
project(EMU-V)

set(CMAKE_CXX_STANDARD 17)

add_executable(emu
    src/main.cpp
    src/InstructionDecoder.cpp
    include/InstructionDecoder.h
    src/VEmu.cpp
    include/VEmu.h
    src/Instruction.cpp
    include/Instruction.h
    include/defs.h
    src/util.cpp
    include/util.h
    src/DRAM.cpp
    include/DRAM.h
    src/Bus.cpp
    include/Bus.h
)
