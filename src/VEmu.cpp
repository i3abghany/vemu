#include "../include/VEmu.h"

VEmu::VEmu(std::string f_name) :
	bin_file_name(std::move(f_name)) 
{
	read_file();
	inst_funcs = {
		{IName::LB,     &VEmu::LB},
		{IName::LH,     &VEmu::LH},
		{IName::LW,     &VEmu::LW},
		{IName::LBU,    &VEmu::LBU},
		{IName::LHU,    &VEmu::LHU},
		{IName::LD,     &VEmu::LD},
		{IName::LWU,    &VEmu::LWU},
		{IName::ADDI,   &VEmu::ADDI},
		{IName::ADDIW,  &VEmu::ADDIW},
		{IName::SLTI,   &VEmu::SLTI},
		{IName::SLTIU,  &VEmu::SLTIU},
		{IName::XORI,   &VEmu::XORI},
		{IName::ORI,    &VEmu::ORI},
		{IName::ANDI,   &VEmu::ANDI},
		{IName::SLLI,   &VEmu::SLLI},
		{IName::SRLI,   &VEmu::SRLI},
		{IName::SRAI,   &VEmu::SRAI},
		{IName::SLLIW,  &VEmu::SLLIW},
		{IName::SRLIW,  &VEmu::SRLIW},
		{IName::SRAIW,  &VEmu::SRAIW},
		{IName::FENCE,  &VEmu::FENCE},
		{IName::FENCEI, &VEmu::FENCEI},
		{IName::ECALL,  &VEmu::ECALL},
		{IName::EBREAK, &VEmu::EBREAK},
		{IName::CSRRW,  &VEmu::CSRRW},
		{IName::CSRRS,  &VEmu::CSRRS},
		{IName::CSRRC,  &VEmu::CSRRC},
		{IName::CSRRWI, &VEmu::CSRRWI},
		{IName::CSRRSI, &VEmu::CSRRSI},
		{IName::CSRRCI, &VEmu::CSRRCI},
		{IName::BEQ,    &VEmu::BEQ},
		{IName::BNE,    &VEmu::BNE},
		{IName::BLT,    &VEmu::BLT},
		{IName::BGE,    &VEmu::BGE},
		{IName::BLTU,   &VEmu::BLTU},
		{IName::BGEU,   &VEmu::BGEU},
		{IName::SB,     &VEmu::SB},
		{IName::SH,     &VEmu::SH},
		{IName::SW,     &VEmu::SW},
		{IName::SD,     &VEmu::SD},
		{IName::ADD,    &VEmu::ADD},
		{IName::SUB,    &VEmu::SUB},
		{IName::SUBW,   &VEmu::SUBW},
		{IName::SLL,    &VEmu::SLL},
		{IName::SLLW,   &VEmu::SLLW},
		{IName::SLT,    &VEmu::SLT},
		{IName::SLTU,   &VEmu::SLTU},
		{IName::XOR,    &VEmu::XOR},
		{IName::SRL,    &VEmu::SRL},
		{IName::SRLW,   &VEmu::SRLW},
		{IName::XXX,    &VEmu::XXX},
		{IName::LUI,    &VEmu::LUI},
		{IName::AUIPC,  &VEmu::AUIPC},
		{IName::XXX,    &VEmu::XXX},
	};
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

uint32_t VEmu::get_4byte_aligned_instr(uint32_t i) 
{
	uint32_t inst = code[i];
	inst |= static_cast<uint32_t>(code[i + 1]) << 8;
	inst |= static_cast<uint32_t>(code[i + 2]) << 16;
	inst |= static_cast<uint32_t>(code[i + 3]) << 24;

	return inst;
}

uint32_t VEmu::run()
{
	for (int i = 0; i < code.size(); i += 4) {
		uint32_t inst = get_4byte_aligned_instr(i);

		auto decoded_inst = InstructionDecoder::the().decode(inst);

		std::cout << decoded_inst;

	}

	return 0;
}

void VEmu::LB()
{
}

void VEmu::LH()
{
}

void VEmu::LW()
{
}

void VEmu::LBU()
{
}

void VEmu::LHU()
{
}

void VEmu::LD()
{
}

void VEmu::LWU()
{
}

void VEmu::ADDI()
{
}

void VEmu::ADDIW()
{
}

void VEmu::SLTI()
{
}

void VEmu::SLTIU()
{
}

void VEmu::XORI()
{
}

void VEmu::ORI()
{
}

void VEmu::ANDI()
{
}

void VEmu::SLLI()
{
}

void VEmu::SRLI()
{
}

void VEmu::SRAI()
{
}

void VEmu::SLLIW()
{
}

void VEmu::SRLIW()
{
}

void VEmu::SRAIW()
{
}

void VEmu::FENCE()
{
}

void VEmu::FENCEI()
{
}

void VEmu::ECALL()
{
}

void VEmu::EBREAK()
{
}

void VEmu::CSRRW()
{
}

void VEmu::CSRRS()
{
}

void VEmu::CSRRC()
{
}

void VEmu::CSRRWI()
{
}

void VEmu::CSRRSI()
{
}

void VEmu::CSRRCI()
{
}

void VEmu::BEQ()
{
}

void VEmu::BNE()
{
}

void VEmu::BLT()
{
}

void VEmu::BGE()
{
}

void VEmu::BLTU()
{
}

void VEmu::BGEU()
{
}

void VEmu::SB()
{
}

void VEmu::SH()
{
}

void VEmu::SW()
{
}

void VEmu::SD()
{
}

void VEmu::ADD()
{
}

void VEmu::SUB()
{
}

void VEmu::SUBW()
{
}

void VEmu::SLL()
{
}

void VEmu::SLLW()
{
}

void VEmu::SLT()
{
}

void VEmu::SLTU()
{
}

void VEmu::XOR()
{
}

void VEmu::SRL()
{
}

void VEmu::SRLW()
{
}

void VEmu::SRA()
{
}

void VEmu::SRAW()
{
}

void VEmu::OR()
{
}

void VEmu::AND()
{
}

void VEmu::JAL()
{
}

void VEmu::JALR()
{
}

void VEmu::LUI()
{
}

void VEmu::AUIPC()
{
}

void VEmu::XXX()
{
}
