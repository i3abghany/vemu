RVCFLAGS=-march=rv64g -Wl,-Ttext=0x0 -nostdlib -O3
RVCC=riscv64-unknown-linux-gnu-gcc 
OBJ_COPY=riscv64-unknown-linux-gnu-objcopy

CC=g++
CFLAGS=-std=c++17 -O3

emu: main.o VEmu.o InstructionDecoder.o Instruction.o DRAM.o Bus.o util.o 
	$(CC) $(CFLAGS) -o emu main.o VEmu.o InstructionDecoder.o Instruction.o DRAM.o Bus.o util.o 

Instruction.o DRAM.o: ../include/defs.h

main.o: main.cpp VEmu.o 
	$(CC) $(CFLAGS) -c main.cpp 

VEmu.o: VEmu.cpp ../include/VEmu.h InstructionDecoder.o Bus.o util.o 
	$(CC) $(CFLAGS) -c VEmu.cpp 

InstructionDecoder.o: InstructionDecoder.cpp ../include/InstructionDecoder.h Instruction.o 
	$(CC) $(CFLAGS) -c InstructionDecoder.cpp 

Instruction.o: Instruction.cpp ../include/Instruction.h  
	$(CC) $(CFLAGS) -c Instruction.cpp

Bus.o: Bus.cpp ../include/Bus.h DRAM.o 
	$(CC) $(CFLAGS) -c Bus.cpp 

DRAM.o: DRAM.cpp ../include/DRAM.h 
	$(CC) $(CFLAGS) -c DRAM.cpp 

util.o: util.cpp ../include/util.h 
	$(CC) $(CFLAGS) -c util.cpp 

code.bin: code
	$(OBJ_COPY) -O binary code code.bin

code: fib.c
	$(RVCC) $(RVCFLAGS) -o code fib.c

clean:
	rm -f code code.bin *.o emu

.PHONY: clean emu 

