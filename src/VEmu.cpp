#include "../include/VEmu.h"

VEmu::VEmu(std::string f_name) :
	bin_file_name(std::move(f_name)) 
{
	mode = Mode::Machine;
	pc = 0x80000000;
	bus = Bus{};
	regs.fill(0);
	regs[2] = ADDR_BASE + DRAM::RAM_SIZE;
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
		{IName::JAL,    &VEmu::JAL},
		{IName::JALR,   &VEmu::JALR},
		{IName::LRW,    &VEmu::LRW},
		{IName::LRD,    &VEmu::LRD},
		{IName::SCW,    &VEmu::SCW},
		{IName::SCD,    &VEmu::SCD},
		{IName::XXX,    &VEmu::XXX},
	};
}

void VEmu::read_file()
{
	std::ifstream ifs(bin_file_name, std::ios_base::binary);
	std::filesystem::path file_path {bin_file_name};

	auto sz = std::filesystem::file_size(file_path);
	code_size = sz;

	uint64_t i = 0;
	while (sz--) {
		store(ADDR_BASE + i, static_cast<uint8_t>(ifs.get()), 8);
		i++;
	}
}

uint64_t VEmu::load(uint64_t addr, size_t sz)
{
	return bus.load(addr, sz);
}

void VEmu::store(uint64_t addr, uint64_t data, size_t sz)
{
	// FIXME: only checks on aligned addresses
	// Will have to check for individual bytes.
	// For example: if the address 0x81000000 
	// is in the reservation set, storing in 
	// the address 0x810000001 will not
	// mark the word as not reserved.
	if (reservation_set.contains(addr)) {
		reservation_set.remove(addr);
	}

	store(addr, data, sz);
}

uint32_t VEmu::get_4byte_aligned_instr(uint32_t i) 
{
	return bus.load(i, 32);
}

uint32_t VEmu::run()
{
	uint64_t msse = 0, mssd = 0;
	for (; pc < ADDR_BASE + code_size; pc += 4) {
		auto t1 = std::chrono::high_resolution_clock::now();
		if (pc == 0x0) break;
		uint32_t inst = get_4byte_aligned_instr(pc);

		curr_instr = InstructionDecoder::the().decode(inst);
		IName instr_iname = curr_instr.get_name();
		auto t2 = std::chrono::high_resolution_clock::now();

		(inst_funcs[instr_iname])(this);
		auto t3 = std::chrono::high_resolution_clock::now();

		msse += std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
		mssd += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
	}

 	for (int i = 0; i < 32; i++)
 		std::cout << "regs[" << i << "] = " << regs[i] << '\n';

	std::cout << "Decoding in: " << mssd << std::endl;
	std::cout << "Exec in: " << msse << std::endl;
	return 0;
}

uint64_t VEmu::load_csr(uint64_t addr)
{
	if (addr == SIE) {
		return csrs[MIE] & csrs[MIDELEG];
	} else {
		return csrs[addr];
	}
}

void VEmu::store_csr(uint64_t addr, uint64_t val) {
	if (addr == SIE) {
		csrs[MIE] &= !csrs[MIDELEG];
		csrs[MIE] |= (val & csrs[MIDELEG]);
	} else {
		csrs[addr] = val;
	}
}

void VEmu::LB()
{
	LBU();

	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(sext_byte(regs[rd]));
}

void VEmu::LW()
{
	LWU();

	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
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

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(bus.load(mem_addr, 8));
}

void VEmu::LH()
{
	LHU();

	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
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

	if (rd == 0) return;
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

	if (rd == 0) return;
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

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(bus.load(mem_addr, 32));
}

void VEmu::ADDI()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
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

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(res);
}

void VEmu::SLTI()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	auto rd  = curr_instr.get_fields().rd;
	auto rs1 = curr_instr.get_fields().rs1;

	if (rd == 0) return;
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

	if (rd == 0) return;
	regs[rd] = 
		(static_cast<uint64_t>(regs[rs1]) < static_cast<uint64_t>(imm)) ?
			1 : 0;
}

void VEmu::XORI()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = regs[rs1] ^ imm;
}

void VEmu::ORI()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = regs[rs1] | imm;
}

void VEmu::ANDI()
{
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = regs[rs1] & imm;
}

void VEmu::SLLI()
{
	uint8_t shamt = 
		static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	uint64_t op = static_cast<uint64_t>(regs[rs1]); 
	op <<= shamt;

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(op);
}

void VEmu::SRLI()
{
	uint8_t shamt = 
		static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	uint64_t op = static_cast<uint64_t>(regs[rs1]); 
	op >>= shamt;

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(op);
}

void VEmu::SRAI()
{
	uint8_t shamt = 
		static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(regs[rs1] >>= shamt);
}

void VEmu::SLLIW()
{
	uint8_t shamt = 
		static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	uint32_t op = static_cast<uint32_t>(regs[rs1]);
	op <<= shamt;

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(
		static_cast<int32_t>(op)
	);
}

void VEmu::SRLIW()
{
	uint8_t shamt = 
		static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	uint32_t op = static_cast<uint32_t>(regs[rs1]);
	op >>= shamt;

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(
		static_cast<int32_t>(op)
	);
}

void VEmu::SRAIW()
{
	uint8_t shamt = 
		static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;

	int32_t op = static_cast<int32_t>(regs[rs1]);
	op >>= shamt;

	if (rd == 0) return;
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
	auto rd = curr_instr.get_fields().rd;
	auto rs1 = curr_instr.get_fields().rs1;
	uint64_t csr_addr = curr_instr.get_fields().imm;

	csr_addr &= 0xFFF;

	uint64_t csr_val = load_csr(csr_addr);
	store_csr(csr_addr, regs[rs1]);

	regs[rd] = csr_val;
}

void VEmu::CSRRS()
{
	auto rd = curr_instr.get_fields().rd;
	auto rs1 = curr_instr.get_fields().rs1;
	uint64_t csr_addr = curr_instr.get_fields().imm;

	csr_addr &= 0xFFF;

	uint64_t csr_val = load_csr(csr_addr);
	store_csr(csr_addr, csr_val | regs[rs1]);

	if (rd == 0) return;
	regs[rd] = csr_val;
}

void VEmu::CSRRC()
{
	auto rd = curr_instr.get_fields().rd;
	auto rs1 = curr_instr.get_fields().rs1;
	uint64_t csr_addr = curr_instr.get_fields().imm;

	csr_addr &= 0xFFF;

	uint64_t csr_val = load_csr(csr_addr);

	store_csr(csr_addr, csr_val & (!regs[rs1]));
	
	if (rd == 0) return;
	regs[rd] = csr_val;
}

void VEmu::CSRRWI()
{
	uint64_t uimm = static_cast<uint64_t>(curr_instr.get_fields().rs1);
	auto rd = curr_instr.get_fields().rd;

	uint64_t csr_addr = curr_instr.get_fields().imm;
	csr_addr &= 0xFFF;

	uint64_t csr_val = load_csr(csr_addr);
	store_csr(csr_addr, uimm);

	if (rd == 0) return;
	regs[rd] = csr_val;
}

void VEmu::CSRRSI()
{
	uint64_t uimm = static_cast<uint64_t>(curr_instr.get_fields().rs1);
	auto rd = curr_instr.get_fields().rd;

	uint64_t csr_addr = curr_instr.get_fields().imm;
	csr_addr &= 0xFFF;

	uint64_t csr_val = load_csr(csr_addr);
	store_csr(csr_addr, csr_val | uimm);

	if (rd == 0) return;
	regs[rd] = csr_val;
}

void VEmu::CSRRCI()
{
	uint64_t uimm = static_cast<uint64_t>(curr_instr.get_fields().rs1);
	auto rd = curr_instr.get_fields().rd;

	uint64_t csr_addr = curr_instr.get_fields().imm;
	csr_addr &= 0xFFF;

	uint64_t csr_val = load_csr(csr_addr);
	store_csr(csr_addr, csr_val & (!uimm));

	if (rd == 0) return;
	regs[rd] = csr_val;
}

void VEmu::BEQ()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (regs[rs1] == regs[rs2]) {
		this->pc += imm;
		this->pc -= 4;
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
		this->pc -= 4;
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
		this->pc -= 4;
	}
}

void VEmu::BGE()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (regs[rs1] >= regs[rs2]) {
		this->pc += imm;
		this->pc -= 4;
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
		this->pc -= 4;
	}
}

void VEmu::BGEU()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (static_cast<uint64_t>(regs[rs1]) >= static_cast<uint64_t>(regs[rs2])) {
		this->pc += imm;
		this->pc -= 4;
	}
}

void VEmu::SB()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm); 

	uint64_t data = static_cast<uint64_t>(regs[rs2]) & 0xFF;
	store(regs[rs1] + offset, data, 8);
}

void VEmu::SH()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm); 

	uint64_t data = static_cast<uint64_t>(regs[rs2]) & 0xFFFF;
	store(regs[rs1] + offset, data, 16);
}

void VEmu::SW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm); 

	uint64_t data = static_cast<uint64_t>(regs[rs2]) & 0xFFFFFFFF;
	store(regs[rs1] + offset, data, 32);
}

void VEmu::SD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm); 

	uint64_t data = static_cast<uint64_t>(regs[rs2]);
	store(regs[rs1] + offset, data, 64);
}

void VEmu::ADD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = regs[rs1] + regs[rs2];
}

void VEmu::ADDW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t op1 = static_cast<int32_t>(regs[rs1]);
	int32_t op2 = static_cast<int32_t>(regs[rs2]);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(op1 + op2);
}

void VEmu::SUB()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = regs[rs1] - regs[rs2];
}

void VEmu::SUBW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t op1 = static_cast<int32_t>(regs[rs1]);
	int32_t op2 = static_cast<int32_t>(regs[rs2]);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(op1 - op2);
}

void VEmu::SLL()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint8_t shamt = static_cast<uint64_t>(regs[rs2]) & 0x3F;

	if (rd == 0) return;
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

	if (rd == 0) return;
	// FIXME: This works only if the 32-bit value is sign-extended
	// and then put to the register. 
	// (I'll have to zero extend the signed 32-bit value if not so.)
	regs[rd] = static_cast<int64_t>(
		static_cast<int32_t>(op)
	);
}

void VEmu::SLT()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;


	regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0;
}

void VEmu::SLTU()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;


	regs[rd] = 
		(static_cast<uint64_t>(regs[rs1]) < static_cast<uint64_t>(regs[rs2])) ? 1 : 0;
}

void VEmu::XOR()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = regs[rs1] ^ regs[rs2];
}

void VEmu::SRL()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint8_t shamt = static_cast<uint64_t>(regs[rs2]) & 0x3F;

	if (rd == 0) return;
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

	if (rd == 0) return;
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
	
	if (rd == 0) return;

	// FIXME: This works only if the 32-bit value is sign-extended
	// and then put to the register. 
	// (I'll have to zero extend the signed 32-bit value if not so.)
	regs[rd] = static_cast<int64_t>(op);
}

void VEmu::OR()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

	regs[rd] = regs[rs1] | regs[rs2];
}

void VEmu::AND()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

	regs[rd] = regs[rs1] & regs[rs2];
}

void VEmu::JAL()
{
	auto rd = curr_instr.get_fields().rd;
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (rd != 0) {
		regs[rd] = static_cast<int64_t>(this->pc + 4);
	}
	
	this->pc += imm;
	this->pc -= 4;
}

void VEmu::JALR()
{
	auto rd = curr_instr.get_fields().rd;
	auto rs1 = curr_instr.get_fields().rs1;
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (rd != 0) {
		regs[rd] = static_cast<int64_t>(this->pc + 4);
	}

	this->pc = static_cast<uint64_t>(regs[rs1] + imm);
	this->pc &= ~(0x1);
	this->pc -= 4;
}

void VEmu::LUI()
{
	// the full 32-bit U-imm
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm); 
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(imm_32);
}

void VEmu::AUIPC()
{
	// the full 32-bit U-imm
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	this->pc += imm;
}

void VEmu::LRW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;
	uint64_t addr = regs[rs1];

	if (addr % 4 != 0) {
		std::cout << "Unaligned access in LRW.";
		exit(EXIT_FAILURE);
	}

	reservation_set.insert(addr);

	if (rd == 0) return;
	regs[rd] = bus.load(addr, 32);
}

void VEmu::LRD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rd = curr_instr.get_fields().rd;
	uint64_t addr = regs[rs1];

	if (addr % 8 != 0) {
		std::cout << "Unaligned access in LRD.";
		exit(EXIT_FAILURE);
	}
	
	reservation_set.insert(addr);
	
	if (rd == 0) return;
	regs[rd] = bus.load(addr, 64);
}

void VEmu::SCW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;
	uint64_t addr = regs[rs1];

	if (addr % 4 != 0) {
		std::cout << "Unaligned access in SCW.";
		exit(EXIT_FAILURE);
	}

	if (!reservation_set.contains(addr)) {
		reservation_set.remove(addr);
		store(addr, regs[rs2], 32);

		if (rd == 0) return;
		regs[rd] = 0;
	} else {
		regs[rd] = 1;
	}
}

void VEmu::SCD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;
	uint64_t addr = regs[rs1];

	if (addr % 8 != 0) {
		std::cout << "Unaligned access in SCD.";
		exit(EXIT_FAILURE);
	}
	
	if (!reservation_set.contains(addr)) {
		reservation_set.remove(addr);
		store(addr, regs[rs2], 64);

		if (rd == 0) return;
		regs[rd] = 0;
	} else {
		regs[rd] = 1;
	}
}

void VEmu::XXX()
{
	std::cout << "Faulty instruction.";
	exit(EXIT_FAILURE);
}
