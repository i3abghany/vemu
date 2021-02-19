#include "../include/VEmu.h"

VEmu::VEmu(std::string f_name) :
	bin_file_name(std::move(f_name)) 
{
	pc = 0x00000000;
	regs.fill(0);
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
		{IName::ADDW,   &VEmu::ADDW},
		{IName::SUB,    &VEmu::SUB},
		{IName::SUBW,   &VEmu::SUBW},
		{IName::SLL,    &VEmu::SLL},
		{IName::SLLW,   &VEmu::SLLW},
		{IName::SLT,    &VEmu::SLT},
		{IName::SLTU,   &VEmu::SLTU},
		{IName::XOR,    &VEmu::XOR},
		{IName::SRL,    &VEmu::SRL},
		{IName::SRLW,   &VEmu::SRLW},
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
	for (; pc < code.size(); pc += 4) {
		uint32_t inst = get_4byte_aligned_instr(pc);

		curr_instr = InstructionDecoder::the().decode(inst);
		IName instr_iname = curr_instr.get_name();

		(inst_funcs[instr_iname])(this);
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
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	regs[rd] = regs[rs1] + imm;
}

void VEmu::ADDIW()
{
	int32_t imm = static_cast<int32_t>(curr_instr.get_fields().imm);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	int32_t op1 = static_cast<int32_t>(regs[rs1] & static_cast<uint32_t>(-1));

	regs[rd] = static_cast<int64_t>(op1 + imm);
}

void VEmu::SLTI()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	auto rd  = curr_instr.get_fields().rd;
	auto rs1 = curr_instr.get_fields().rs1;

	regs[rd] = (regs[rs1] < imm) ? 1 : 0;
}

// SLTIU rd, rs1, 1 sets rd to 1 
// if rs1 == 0, otherwise it sets 
// it to 0.
void VEmu::SLTIU()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	auto rd  = curr_instr.get_fields().rd;
	auto rs1 = curr_instr.get_fields().rs1;

	regs[rd] = (static_cast<uint64_t>(regs[rs1]) < static_cast<uint64_t>(imm)) ? 1 : 0;
}

void VEmu::XORI()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	regs[rd] = regs[rs1] ^ imm;
}

void VEmu::ORI()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	regs[rd] = regs[rs1] | imm;
}

void VEmu::ANDI()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	regs[rd] = regs[rs1] & imm;
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
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (regs[rs1] == regs[rs2]) {
		this->pc += imm;
	}

}

void VEmu::BNE()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (regs[rs1] != regs[rs2]) {
		this->pc += imm;
	}
}

void VEmu::BLT()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (regs[rs1] < regs[rs2]) {
		this->pc += imm;
	}
}

void VEmu::BGE()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (regs[rs1] > regs[rs2]) {
		this->pc += imm;
	}
}

void VEmu::BLTU()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (static_cast<uint64_t>(regs[rs1]) < static_cast<uint64_t>(regs[rs2])) {
		this->pc += imm;
	}
}

void VEmu::BGEU()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (static_cast<uint64_t>(regs[rs1]) > static_cast<uint64_t>(regs[rs2])) {
		this->pc += imm;
	}
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
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	regs[rd] = regs[rs1] + regs[rs2];
}

void VEmu::ADDW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t op1 = static_cast<int32_t>(regs[rs1]);
	int32_t op2 = static_cast<int32_t>(regs[rs2]);

	regs[rd] = static_cast<int64_t>(op1 + op2);
}

void VEmu::SUB()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	regs[rd] = regs[rs1] - regs[rs2];
}

void VEmu::SUBW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t op1 = static_cast<int32_t>(regs[rs1]);
	int32_t op2 = static_cast<int32_t>(regs[rs2]);

	regs[rd] = static_cast<int64_t>(op1 - op2);
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
