CFLAGS=-Wl,-Ttext=0x0 -nostdlib
CC=riscv64-unknown-linux-gnu-gcc -march=rv64g
OBJ_COPY=riscv64-unknown-linux-gnu-objcopy

all: comp makebin

comp: code.s
	$(CC) $(CFLAGS) -o code code.s

makebin: code
	$(OBJ_COPY) -O binary code code.bin


clean:
	rm code code.bin
