#include "../include/VEmu.h"
#include <iostream>

VEmu::VEmu(std::string f_name) :
	bin_file_name(std::move(f_name)) 
{
	read_file();
}

void VEmu::read_file()
{
	std::ifstream ifs(bin_file_name, std::ios_base::binary);
	std::filesystem::path file_path {bin_file_name};

	auto sz = std::filesystem::file_size(file_path);

	char byte;

	while (sz--) {
		code.push_back(static_cast<uint8_t>(ifs.get()));
	}
}

uint32_t VEmu::run()
{
	for (int i = 0; i < code.size(); i += 4) {
		uint32_t inst = code[i];
		inst |= ((uint32_t)code[i + 1]) << 8;
		inst |= ((uint32_t)code[i + 2]) << 16;
		inst |= ((uint32_t)code[i + 3]) << 24;

		auto decoded_inst = InstructionDecoder::the().decode(inst);

		std::cout << decoded_inst;

	}

	return 0;
}
