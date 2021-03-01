CFLAGS=-march=rv64g -Wl,-Ttext=0x0 -nostdlib -O3
RVCC=riscv64-unknown-linux-gnu-gcc 
OBJ_COPY=riscv64-unknown-linux-gnu-objcopy

emu: main.o VEmu.o InstructionDecoder.o Instruction.o DRAM.o Bus.o util.o
	g++ -o emu main.o VEmu.o InstructionDecoder.o Instruction.o DRAM.o Bus.o util.o -std=c++17 -O3

main.o: main.cpp VEmu.o
	g++ -c main.cpp -std=c++17 -O3

VEmu.o: VEmu.cpp ../include/VEmu.h InstructionDecoder.o Bus.o util.o
	g++ -c VEmu.cpp -std=c++17 -O3

InstructionDecoder.o: InstructionDecoder.cpp ../include/InstructionDecoder.h Instruction.o
	g++ -c InstructionDecoder.cpp -std=c++17 -O3

Instruction.o: Instruction.cpp ../include/Instruction.h ../include/defs.h
	g++ -c Instruction.cpp -std=c++17 -O3

Bus.o: Bus.cpp ../include/Bus.h DRAM.o
	g++ -c Bus.cpp -std=c++17 -O3

DRAM.o: DRAM.cpp ../include/DRAM.h ../include/defs.h
	g++ -c DRAM.cpp -std=c++17 -O3

util.o: util.cpp ../include/util.h
	g++ -c util.cpp -std=c++17 -O3

code.bin: code
	$(OBJ_COPY) -O binary code code.bin

code: fib.c
	$(RVCC) $(CFLAGS) -o code fib.c

clean:
	rm code code.bin *.o 

