cmake_minimum_required(VERSION 3.16)
project(vemu)

set(CMAKE_CXX_COMPILER g++-12)
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
    src/CLINT.cpp
    src/PLIC.cpp
    src/UART.cpp
)

include_directories(include)

set(SOFTFLOAT_MAIN_DIR "vendor/SoftFloat")
set(SOFTFLOAT_SOURCE_DIR "${SOFTFLOAT_MAIN_DIR}/source")
set(SPECIALIZED_TYPE 8086-SSE)

if(SUPPORT_SOFTFLOAT)
include_directories(softfloat PUBLIC
        "${SOFTFLOAT_SOURCE_DIR}/${SPECIALIZED_TYPE}/"
        "${SOFTFLOAT_SOURCE_DIR}/include"
        "${SOFTFLOAT_MAIN_DIR}/build/Win64-MinGW-w64/"
        )
    string(APPEND CMAKE_CXX_FLAGS "-DSUPPORT_SOFTFLOAT ")
endif()

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
target_compile_options(test_emu PRIVATE -fprofile-arcs)
target_compile_options(test_emu PRIVATE -ftest-coverage)
target_compile_options(test_emu PRIVATE --coverage)

target_compile_options(emu PRIVATE -O3)

target_link_libraries(test_emu PRIVATE -lgcov)
target_link_libraries(test_emu PRIVATE --coverage)

if (SUPPORT_SOFTFLOAT)
    target_link_libraries(test_emu PUBLIC ${CMAKE_SOURCE_DIR}/${SOFTFLOAT_MAIN_DIR}/build/Win64-MinGW-w64/softfloat.a)
    target_link_libraries(emu PUBLIC ${CMAKE_SOURCE_DIR}/${SOFTFLOAT_MAIN_DIR}/build/Win64-MinGW-w64/softfloat.a)
endif()