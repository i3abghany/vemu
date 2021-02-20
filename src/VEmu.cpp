#include "../include/VEmu.h"

VEmu::VEmu(std::string f_name) :
	bin_file_name(std::move(f_name)) 
{
	pc = 0x80000000;
	bus = Bus{};
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
	code_size = sz;

	char byte;
	uint64_t i = 0;
	while (sz--) {
		bus.store(ADDR_BASE + i, static_cast<uint8_t>(ifs.get()), 8);
		i++;
	}
}

uint32_t VEmu::get_4byte_aligned_instr(uint32_t i) 
{
	return bus.load(i, 32);
}

uint32_t VEmu::run()
{
	for (; pc < ADDR_BASE + code_size; pc += 4) {
		uint32_t inst = get_4byte_aligned_instr(pc);

		curr_instr = InstructionDecoder::the().decode(inst);
		IName instr_iname = curr_instr.get_name();

		(inst_funcs[instr_iname])(this);
	}

	return 0;
}

void VEmu::LB()
{
	LBU();

	auto rd = curr_instr.get_fields().rd;

	regs[rd] = static_cast<int64_t>(sext_byte(regs[rd]));
}

void VEmu::LW()
{
	LWU();

	auto rd = curr_instr.get_fields().rd;

	regs[rd] = static_cast<int64_t>(sext_word(regs[rd]));
}

void VEmu::LBU()
{
	auto base_reg = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	// add as signed then convert to unsigned.
	uint64_t mem_addr = 
		static_cast<uint64_t>(static_cast<int64_t>(regs[base_reg]) + imm);

	regs[rd] = static_cast<int64_t>(bus.load(mem_addr, 8));
}

void VEmu::LH()
{
	LHU();

	auto rd = curr_instr.get_fields().rd;

	regs[rd] = static_cast<int64_t>(sext_hword(regs[rd]));
}

void VEmu::LHU()
{
	auto base_reg = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	// add as signed then convert to unsigned.
	uint64_t mem_addr = 
		static_cast<uint64_t>(static_cast<int64_t>(regs[base_reg]) + imm);

	regs[rd] = static_cast<int64_t>(bus.load(mem_addr, 16));
}

void VEmu::LD()
{
	auto base_reg = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	// add as signed then convert to unsigned.
	uint64_t mem_addr = 
		static_cast<uint64_t>(static_cast<int64_t>(regs[base_reg]) + imm);

	regs[rd] = static_cast<int64_t>(bus.load(mem_addr, 64));
}

void VEmu::LWU()
{
	auto base_reg = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	// add as signed then convert to unsigned.
	uint64_t mem_addr = 
		static_cast<uint64_t>(static_cast<int64_t>(regs[base_reg]) + imm);

	regs[rd] = static_cast<int64_t>(bus.load(mem_addr, 32));
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
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	int64_t op = regs[rs1];

	int32_t res = static_cast<int32_t>((op + imm) & 0xFFFFFFFF);

	regs[rd] = static_cast<int64_t>(res);
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
	uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	uint64_t op = static_cast<uint64_t>(regs[rs1]); 
	op <<= shamt;

	regs[rd] = static_cast<int64_t>(op);
}

void VEmu::SRLI()
{
	uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	uint64_t op = static_cast<uint64_t>(regs[rs1]); 
	op >>= shamt;

	regs[rd] = static_cast<int64_t>(op);
}

void VEmu::SRAI()
{
	uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	regs[rd] = static_cast<int64_t>(regs[rs1] >>= shamt);
}

void VEmu::SLLIW()
{
	uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	uint32_t op = static_cast<uint32_t>(regs[rs1]);
	op <<= shamt;

	regs[rd] = static_cast<int64_t>(
		static_cast<int32_t>(op)
	);
}

void VEmu::SRLIW()
{
	uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	uint32_t op = static_cast<uint32_t>(regs[rs1]);
	op >>= shamt;

	regs[rd] = static_cast<int64_t>(
		static_cast<int32_t>(op)
	);
}

void VEmu::SRAIW()
{
	uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	int32_t op = static_cast<int32_t>(regs[rs1]);
	op >>= shamt;

	regs[rd] = static_cast<int64_t>(op);
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
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm); 

	uint64_t data = static_cast<uint64_t>(regs[rs2]) & 0xFF;
	bus.store(regs[rs1] + offset, data, 8);
}

void VEmu::SH()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm); 

	uint64_t data = static_cast<uint64_t>(regs[rs2]) & 0xFFFF;
	bus.store(regs[rs1] + offset, data, 16);
}

void VEmu::SW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm); 

	uint64_t data = static_cast<uint64_t>(regs[rs2]) & 0xFFFFFFFF;
	bus.store(regs[rs1] + offset, data, 32);
}

void VEmu::SD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm); 

	uint64_t data = static_cast<uint64_t>(regs[rs2]);
	bus.store(regs[rs1] + offset, data, 64);
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
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint8_t shamt = static_cast<uint64_t>(regs[rs2]) & 0x3F;

	regs[rd] = static_cast<int64_t>(
		static_cast<uint64_t>(regs[rs1]) << shamt
	);
}

void VEmu::SLLW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint8_t shamt = static_cast<uint64_t>(regs[rs2]) & 0x1F;

	uint32_t op =  static_cast<uint32_t>(regs[rs1]); 

	op <<= shamt;

	// FIXME: This works only if the 32-bit value is sign-extended
	// and then put to the register. 
	// (I'll have to zero extend the signed 32-bit value if not so.)
	regs[rd] = static_cast<int64_t>(
		static_cast<int32_t>(op)
	);
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
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint8_t shamt = static_cast<uint64_t>(regs[rs2]) & 0x3F;

	regs[rd] = static_cast<int64_t>(
		static_cast<uint64_t>(regs[rs1]) >> shamt
	);
}

void VEmu::SRLW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint8_t shamt = static_cast<uint64_t>(regs[rs2]) & 0x1F;

	uint32_t op =  static_cast<uint32_t>(regs[rs1]); 

	op >>= shamt;

	// FIXME: This works only if the 32-bit value is sign-extended
	// and then put to the register. 
	// (I'll have to zero extend the signed 32-bit value if not so.)
	regs[rd] = static_cast<int64_t>(
		static_cast<int32_t>(op)
	);
}

void VEmu::SRA()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint8_t shamt = static_cast<uint64_t>(regs[rs2]) & 0x3F;

	regs[rd] = static_cast<int64_t>(
		regs[rs1] >> shamt
	);
}

void VEmu::SRAW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint8_t shamt = static_cast<uint64_t>(regs[rs2]) & 0x1F;

	int32_t op =  static_cast<int32_t>(regs[rs1]);

	op >>= shamt;
	
	// FIXME: This works only if the 32-bit value is sign-extended
	// and then put to the register. 
	// (I'll have to zero extend the signed 32-bit value if not so.)
	regs[rd] = static_cast<int64_t>(op);
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
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm); // the full 32-bit U-imm
	auto rd = curr_instr.get_fields().rd;

	regs[rd] = static_cast<int64_t>(imm_32);
}

void VEmu::AUIPC()
{
}

void VEmu::XXX()
{
	std::cout << "Faulty instruction.";
	exit(EXIT_FAILURE);
}
