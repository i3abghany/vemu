cmake_minimum_required(VERSION 3.16)
project(EMU-V)

set(CMAKE_CXX_STANDARD 17)

add_executable(EMU_V src/main.cpp src/InstructionDecoder.cpp include/InstructionDecoder.h src/VEmu.cpp include/VEmu.h src/Instruction.cpp include/Instruction.h include/defs.h tests/decode_type_test.h)
add_executable(tests tests/run_tests.cpp src/InstructionDecoder.cpp include/InstructionDecoder.h tests/decode_wrong_test.h src/Instruction.cpp tests/decode_r_test.h tests/decode_several_instructions_test.h tests/decode_system_instructions_test.h tests/decode_shift_test.h)
