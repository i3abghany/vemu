#include "../include/VEmu.h"
#include <iostream>
#include <string>
#include "../include/InstructionDecoder.h"

int main() {
	std::string name = "./code.bin";
	VEmu em { name };
	uint32_t inst = em.run();

    return 0;
}
