#include <iostream>
#include <string>
#include "../include/VEmu.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "Provide a program.\n";
		return 1;
	}
	VEmu em { std::string {argv[1]} };
	em.run();

    return 0;
}
