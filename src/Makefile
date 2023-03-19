RVCFLAGS=-march=rv64g -Wl,-Ttext=0x0 -nostdlib -O3
RVCC=riscv64-unknown-linux-gnu-gcc 
OBJ_COPY=riscv64-unknown-linux-gnu-objcopy

CC=g++
CFLAGS=-std=c++17 -ggdb -Wall -Werror -Wextra -Wshadow -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code 

OBJ_FILES = main.o VEmu.o InstructionDecoder.o Instruction.o MMU.o Bus.o util.o RegFile.o

tests: CFLAGS += -DTEST_ENV

emu: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o emu $(OBJ_FILES)

tests: $(OBJ_FILES) Tester.o
	$(CC) $(CFLAGS) -o emu $(OBJ_FILES) Tester.o \
	&& ./emu

Instruction.o MMU.o: ../include/defs.h

main.o: main.cpp VEmu.o 
	$(CC) $(CFLAGS) -c main.cpp 

VEmu.o: VEmu.cpp ../include/VEmu.h InstructionDecoder.o Bus.o util.o 
	$(CC) $(CFLAGS) -c VEmu.cpp 

Tester.o: Tester.cpp ../include/Tester.h VEmu.cpp ../include/VEmu.h
	$(CC) $(CFLAGS) -c Tester.cpp 

InstructionDecoder.o: InstructionDecoder.cpp ../include/InstructionDecoder.h Instruction.o 
	$(CC) $(CFLAGS) -c InstructionDecoder.cpp 

Instruction.o: Instruction.cpp ../include/Instruction.h  
	$(CC) $(CFLAGS) -c Instruction.cpp

Bus.o: Bus.cpp ../include/Bus.h MMU.o 
	$(CC) $(CFLAGS) -c Bus.cpp 

MMU.o: MMU.cpp ../include/MMU.h 
	$(CC) $(CFLAGS) -c MMU.cpp 

util.o: util.cpp ../include/util.h 
	$(CC) $(CFLAGS) -c util.cpp 

RegFile.o: RegFile.cpp ../include/RegFile.h 
	$(CC) $(CFLAGS) -c RegFile.cpp 

code.bin: code
	$(OBJ_COPY) -O binary code code.bin

code: fib.c
	$(RVCC) $(RVCFLAGS) -o code fib.c

clean:
	rm -f code code.bin *.o emu

.PHONY: clean emu tests

