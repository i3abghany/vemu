#pragma once

#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include <fstream>

#include "InstructionDecoder.h"

class VEmu {
public:
	VEmu(std::string f_name);

	uint32_t run();

	void read_file();

private:
	constexpr static size_t REGS_NUM = 32;
	std::array<uint64_t, REGS_NUM> regs;
	
	std::vector<uint8_t> code;

	std::string bin_file_name;
};

