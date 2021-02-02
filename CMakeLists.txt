cmake_minimum_required(VERSION 3.16)
project(EMU-V)

set(CMAKE_CXX_STANDARD 17)

add_executable(EMU_V main.cpp InstructionDecoder.cpp InstructionDecoder.h VEmu.cpp VEmu.h Instruction.cpp Instruction.h defs.h)