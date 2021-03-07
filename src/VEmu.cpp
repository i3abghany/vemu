#include "../include/VEmu.h"

VEmu::VEmu(std::string f_name) :
	bin_file_name(std::move(f_name)) 
{
	mode = Mode::Machine;
	pc = 0x80000000;
	bus = Bus{};
	regs.fill(0);
	csrs.fill(0);
	regs[2] = ADDR_BASE + DRAM::RAM_SIZE;
	read_file();
	init_func_map();
}

void VEmu::init_func_map()
{
	inst_funcs = {
        {IName::LB,       &VEmu::LB},
        {IName::LH,       &VEmu::LH},
        {IName::LW,       &VEmu::LW},
        {IName::LBU,      &VEmu::LBU},
        {IName::LHU,      &VEmu::LHU},
        {IName::LD,       &VEmu::LD},
        {IName::LWU,      &VEmu::LWU},
        {IName::ADDI,     &VEmu::ADDI},
        {IName::ADDIW,    &VEmu::ADDIW},
        {IName::SLTI,     &VEmu::SLTI},
        {IName::SLTIU,    &VEmu::SLTIU},
        {IName::XORI,     &VEmu::XORI},
        {IName::ORI,      &VEmu::ORI},
        {IName::ANDI,     &VEmu::ANDI},
        {IName::SLLI,     &VEmu::SLLI},
        {IName::SRLI,     &VEmu::SRLI},
        {IName::SRAI,     &VEmu::SRAI},
        {IName::SLLIW,    &VEmu::SLLIW},
        {IName::SRLIW,    &VEmu::SRLIW},
        {IName::SRAIW,    &VEmu::SRAIW},
        {IName::SRAW,     &VEmu::SRAW},
        {IName::SRLW,     &VEmu::SRLW},
        {IName::FENCE,    &VEmu::FENCE},
        {IName::FENCEI,   &VEmu::FENCEI},
        {IName::ECALL,    &VEmu::ECALL},
        {IName::EBREAK,   &VEmu::EBREAK},
        {IName::CSRRW,    &VEmu::CSRRW},
        {IName::CSRRS,    &VEmu::CSRRS},
        {IName::CSRRC,    &VEmu::CSRRC},
        {IName::CSRRWI,   &VEmu::CSRRWI},
        {IName::CSRRSI,   &VEmu::CSRRSI},
        {IName::CSRRCI,   &VEmu::CSRRCI},
        {IName::BEQ,      &VEmu::BEQ},
        {IName::BNE,      &VEmu::BNE},
        {IName::BLT,      &VEmu::BLT},
        {IName::BGE,      &VEmu::BGE},
        {IName::BLTU,     &VEmu::BLTU},
        {IName::BGEU,     &VEmu::BGEU},
        {IName::SB,       &VEmu::SB},
        {IName::SH,       &VEmu::SH},
        {IName::SW,       &VEmu::SW},
        {IName::SD,       &VEmu::SD},
        {IName::ADD,      &VEmu::ADD},
        {IName::ADDW,     &VEmu::ADDW},
        {IName::SUB,      &VEmu::SUB},
        {IName::SUBW,     &VEmu::SUBW},
        {IName::SLL,      &VEmu::SLL},
        {IName::SLLW,     &VEmu::SLLW},
        {IName::SLT,      &VEmu::SLT},
        {IName::SLTU,     &VEmu::SLTU},
        {IName::XOR,      &VEmu::XOR},
        {IName::SRL,      &VEmu::SRL},
        {IName::SRLW,     &VEmu::SRLW},
        {IName::LUI,      &VEmu::LUI},
        {IName::AUIPC,    &VEmu::AUIPC},
        {IName::JAL,      &VEmu::JAL},
        {IName::JALR,     &VEmu::JALR},
        {IName::MUL,      &VEmu::MUL},
        {IName::MULH,     &VEmu::MULH},
        {IName::MULHSU,   &VEmu::MULHSU},
        {IName::MULHU,    &VEmu::MULHU},
        {IName::DIV,      &VEmu::DIV},
        {IName::DIVU,     &VEmu::DIVU},
        {IName::REM,      &VEmu::REM},
        {IName::REMU,     &VEmu::REMU},
        {IName::MULW,     &VEmu::MULW},
        {IName::DIVW,     &VEmu::DIVW},
        {IName::DIVUW,    &VEmu::DIVUW},
        {IName::REMW,     &VEmu::REMW},
        {IName::REMUW,    &VEmu::REMUW},

        {IName::AMOSWAPW, &VEmu::AMOSWAPW},
        {IName::AMOADDW,  &VEmu::AMOADDW},
        {IName::AMOXORW,  &VEmu::AMOXORW},
        {IName::AMOANDW,  &VEmu::AMOANDW},
        {IName::AMOORW,   &VEmu::AMOORW},
        {IName::AMOMINW,  &VEmu::AMOMINW},
        {IName::AMOMAXW,  &VEmu::AMOMAXW},
        {IName::AMOMINUW, &VEmu::AMOMINUW},
        {IName::AMOMAXUW, &VEmu::AMOMAXUW},
        {IName::LRW,      &VEmu::LRW},
        {IName::SCW,      &VEmu::SCW},

        {IName::AMOSWAPD, &VEmu::AMOSWAPD},
        {IName::AMOADDD,  &VEmu::AMOADDD},
        {IName::AMOXORD,  &VEmu::AMOXORD},
        {IName::AMOANDD,  &VEmu::AMOANDD},
        {IName::AMOORD,   &VEmu::AMOORD},
        {IName::AMOMIND,  &VEmu::AMOMIND},
        {IName::AMOMAXD,  &VEmu::AMOMAXD},
        {IName::AMOMINUD, &VEmu::AMOMINUD},
        {IName::AMOMAXUD, &VEmu::AMOMAXUD},
        {IName::LRD,      &VEmu::LRD},
        {IName::SCD,      &VEmu::SCD},
        
        {IName::MRET,      &VEmu::MRET},
        {IName::SRET,      &VEmu::SRET},
        
        {IName::XXX,      &VEmu::XXX},
	};
}

void VEmu::read_file()
{
	std::ifstream ifs(bin_file_name, std::ios_base::binary);
	if (!ifs) {
		std::cout << "Could not open the file.\n";
		exit(EXIT_FAILURE);
	}
	std::filesystem::path file_path {bin_file_name};

	auto sz = std::filesystem::file_size(file_path);
	code_size = sz;

	uint64_t i = 0;
	while (sz--) {
		uint8_t c = static_cast<uint8_t>(ifs.get());
		store(ADDR_BASE + i, c, 8);
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
	if (reservation_set.count(addr)) {
		reservation_set.erase(addr);
	}

	bus.store(addr, data, sz);
}

uint32_t VEmu::get_4byte_aligned_instr(uint64_t i) 
{
	return static_cast<uint32_t>(bus.load(i, 32));
}

uint32_t VEmu::run()
{
	for (; pc < ADDR_BASE + code_size; pc += 4) {
		if (pc == 0x0) break;
		hex_instr = get_4byte_aligned_instr(pc);

		curr_instr = InstructionDecoder::the().decode(hex_instr);
		IName instr_iname = curr_instr.get_name();

		(inst_funcs[instr_iname])(this);
	}
	return 0;
}

void VEmu::dump_regs() {
 	for (int i = 0; i < 32; i++) {
 		std::cout << "{" 
			<< std::left 
			<< std::setfill(' ')
			<< std::setw(3)
			<< abi_map[i] 
			<< "} " << "regs[" 
			<< std::left 
			<< std::setw(2) 
			<< i << "] = " 
			<< std::hex
			<< regs[i] << 
			std::dec << '\n';
	}
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
	regs[rd] = static_cast<int64_t>(
		sext_byte(static_cast<uint8_t>(regs[rd] & 0xFF)));
}

void VEmu::LW()
{
	LWU();

	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(
		sext_word(static_cast<uint32_t>(regs[rd] & 0xFFFFFFFF)));
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
	regs[rd] = static_cast<int64_t>(
		sext_word(static_cast<uint16_t>(regs[rd] & 0xFFFF)));
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
	std::cout << "IN ECALL: " << std::hex << pc << std::endl;
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

	if (rd == 0) return;
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

	if (rd == 0) return;
	regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0;
}

void VEmu::SLTU()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
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

void VEmu::MUL()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = regs[rs1] * regs[rs2];
}

void VEmu::MULW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = 
        static_cast<int32_t>(regs[rs1] & 0xFFFFFFFF) * static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF);
}

void VEmu::MULH()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(
        ((__int128)regs[rs1] * (__int128)regs[rs2]) >> 64
    );
}

void VEmu::MULHU()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;
    uint64_t urs1 = static_cast<uint64_t>(regs[rs1]);
    uint64_t urs2 = static_cast<uint64_t>(regs[rs2]);

    regs[rd] = static_cast<int64_t>(
        (static_cast<unsigned __int128>(urs1) * static_cast<unsigned __int128>(urs2)) >> 64
    );
}

void VEmu::MULHSU()
{
	std::cout << "MULHSU not yet implemented.\n";
    exit(EXIT_FAILURE);
}

void VEmu::DIV()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

    // FIXME
    if (regs[rs2] == 0) {
        regs[rd] = 0xFFFFFFFF'FFFFFFFF;
    } else if (regs[rs1] == INT64_MIN && regs[rs2] == -1) {
        regs[rd] = regs[rs1];
    } else {
        regs[rd] = regs[rs1] / regs[rs2];
    }
}

void VEmu::DIVU()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

    // FIXME
    if (regs[rs2] == 0) {
        regs[rd] = 0xFFFFFFFF'FFFFFFFF;
        return;
    }

	regs[rd] = static_cast<int64_t>( 
        static_cast<uint64_t>(regs[rs1]) / static_cast<uint64_t>(regs[rs2])
    );
}

void VEmu::DIVW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

    // FIXME
    if (regs[rs2] == 0) {
        regs[rd] = 0xFFFFFFFF'FFFFFFFF;
        return;
    }

	regs[rd] = 
        static_cast<int32_t>(regs[rs1] & 0xFFFFFFFF) / static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF);
}

void VEmu::DIVUW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

    // FIXME
    if (regs[rs2] == 0) {
        regs[rd] = 0xFFFFFFFF'FFFFFFFF;
        return;
    }

	regs[rd] = static_cast<int64_t>( 
        static_cast<uint32_t>(regs[rs1] & 0xFFFFFFFF) / static_cast<uint32_t>(regs[rs2] & 0xFFFFFFFF)
    );
}

void VEmu::REM()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

    if (regs[rs2] == 0) {
        regs[rd] = regs[rs1];
    } else if (regs[rs1] == INT64_MIN && regs[rs2] == -1) {
        regs[rd] = 0;
    } else {
        regs[rd] = regs[rs1] % regs[rs2];
    }
}

void VEmu::REMU()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

    // FIXME
    if (regs[rs2] == 0) {
        regs[rd] = regs[rs1];
        return;
    }

	regs[rd] = static_cast<int64_t>( 
        static_cast<uint64_t>(regs[rs1]) % static_cast<uint64_t>(regs[rs2])
    );
}

void VEmu::REMW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

    // FIXME
    if (regs[rs2] == 0) {
        regs[rd] = static_cast<int64_t>(static_cast<int32_t>(regs[rs2]));
        return;
    }

	regs[rd] = 
        static_cast<int32_t>(regs[rs1]) % static_cast<int32_t>(regs[rs2]);
}

void VEmu::REMUW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	if (rd == 0) return;

    // FIXME
    if (regs[rs2] == 0) {
        regs[rd] = 0xFFFFFFFF'FFFFFFFF;
        return;
    }

	regs[rd] = static_cast<int64_t>( 
        static_cast<uint32_t>(regs[rs1] & 0xFFFFFFFF) % static_cast<uint32_t>(regs[rs2] & 0xFFFFFFFF)
    );
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
	auto rd = curr_instr.get_fields().rd;
	int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
	int64_t imm = static_cast<int64_t>(imm_32);

	if (rd == 0) return;
	regs[rd] = this->pc + imm;
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

	if (!reservation_set.count(addr)) {
		reservation_set.erase(addr);
		store(addr, regs[rs2], 32);

		if (rd == 0) return;
		regs[rd] = 0;
	} else {
		if (rd == 0) return;
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
	
	if (!reservation_set.count(addr)) {
		reservation_set.erase(addr);
		store(addr, regs[rs2], 64);

		if (rd == 0) return;
		regs[rd] = 0;
	} else {
		if (rd == 0) return;
		regs[rd] = 1;
	}
}

void VEmu::AMOSWAPW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t tmp = static_cast<int32_t>(load(regs[rs1], 32) & 0xFFFFFFFF);

	store(regs[rs1], static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF), 32);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOADDW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t tmp = static_cast<int32_t>(load(regs[rs1], 32) & 0xFFFFFFFF);
	int32_t rs2_val = static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF);

	int32_t res_32 = tmp + rs2_val;
	uint64_t res = static_cast<uint64_t>(static_cast<uint32_t>(res_32));

	store(regs[rs1], res, 32);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOANDW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t tmp = static_cast<int32_t>(load(regs[rs1], 32) & 0xFFFFFFFF);
	int32_t rs2_val = static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF);

	uint64_t res = tmp & rs2_val; 
	res &= 0xFFFFFFFF;
	
	store(regs[rs1], res, 32);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOORW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t tmp = static_cast<int32_t>(load(regs[rs1], 32) & 0xFFFFFFFF);
	int32_t rs2_val = static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF);

	uint64_t res = (tmp | rs2_val);

	res &= 0xFFFFFFFF;

	store(regs[rs1], res, 32);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOXORW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t tmp = static_cast<int32_t>(load(regs[rs1], 32) & 0xFFFFFFFF);
	int32_t rs2_val = static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF);

	uint64_t res = (tmp ^ rs2_val);

	res &= 0xFFFFFFFF;

	store(regs[rs1], res, 32);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOMINW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t tmp = static_cast<int32_t>(load(regs[rs1], 32) & 0xFFFFFFFF);
	int32_t rs2_val = static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF);

	uint64_t res = tmp < rs2_val ? 
		static_cast<uint64_t>(static_cast<uint32_t>(tmp)) : 
		static_cast<uint64_t>(static_cast<uint32_t>(rs2_val));

	store(regs[rs1], res, 32);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOMAXW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t tmp = static_cast<int32_t>(load(regs[rs1], 32) & 0xFFFFFFFF);
	int32_t rs2_val = static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF);

	uint64_t res = tmp > rs2_val ? 
		static_cast<uint64_t>(static_cast<uint32_t>(tmp)) : 
		static_cast<uint64_t>(static_cast<uint32_t>(rs2_val));

	store(regs[rs1], res, 32);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOMINUW()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint32_t tmp = static_cast<uint32_t>(load(regs[rs1], 32));
	uint32_t rs2_val = static_cast<uint32_t>(regs[rs2]);

	uint64_t res = tmp < rs2_val ? 
		static_cast<uint64_t>(tmp) : static_cast<uint64_t>(rs2_val);

	store(regs[rs1], res, 32);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOMAXUW() 
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int32_t tmp = static_cast<int32_t>(load(regs[rs1], 32) & 0xFFFFFFFF);
	int32_t rs2_val = static_cast<int32_t>(regs[rs2] & 0xFFFFFFFF);

	uint64_t res = tmp > rs2_val ? 
		static_cast<uint64_t>(static_cast<uint32_t>(tmp)) : 
		static_cast<uint64_t>(static_cast<uint32_t>(rs2_val));

	store(regs[rs1], res, 32);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}


void VEmu::AMOSWAPD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint64_t tmp = load(regs[rs1], 64);

	store(regs[rs1], regs[rs2], 64);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOADDD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int64_t tmp = load(regs[rs1], 64);
	int64_t rs2_val = regs[rs2] ;

	int64_t res = tmp + rs2_val;

	store(regs[rs1], res, 64);

	if (rd == 0) return;
	regs[rd] = tmp;
}

void VEmu::AMOXORD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint64_t tmp = load(regs[rs1], 64);
	uint64_t rs2_val = regs[rs2] ;

	uint64_t res = tmp ^ rs2_val;

	store(regs[rs1], res, 64);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOANDD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint64_t tmp = load(regs[rs1], 64);
	uint64_t rs2_val = regs[rs2] ;

	uint64_t res = tmp ^ rs2_val;

	store(regs[rs1], res, 64);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOORD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint64_t tmp = load(regs[rs1], 64);
	uint64_t rs2_val = regs[rs2] ;

	uint64_t res = tmp | rs2_val;

	store(regs[rs1], res, 64);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOMIND()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	int64_t tmp = load(regs[rs1], 64);
	int64_t rs2_val = regs[rs2];

	uint64_t res = tmp < rs2_val ? tmp : rs2_val;

	store(regs[rs1], res, 64);

	if (rd == 0) return;
	regs[rd] = tmp;
}

void VEmu::AMOMAXD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint64_t addr = regs[rs1];

	if (addr % 8 != 0) {
		std::cout << "unaligned address in AMOMAXD.";
		exit(EXIT_FAILURE);
	}

	int64_t tmp = load(addr, 64);
	int64_t rs2_val = regs[rs2] ;

	int64_t res = tmp > rs2_val ? tmp : rs2_val;

	store(addr, static_cast<uint64_t>(res), 64);

	if (rd == 0) return;
	regs[rd] = tmp;
}

void VEmu::AMOMINUD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint64_t tmp = load(regs[rs1], 64);
	uint64_t rs2_val = regs[rs2] ;

	uint64_t res = tmp < rs2_val ? tmp : rs2_val;

	store(regs[rs1], res, 64);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::AMOMAXUD()
{
	auto rs1 = curr_instr.get_fields().rs1;
	auto rs2 = curr_instr.get_fields().rs2;
	auto rd = curr_instr.get_fields().rd;

	uint64_t tmp = load(regs[rs1], 64);
	uint64_t rs2_val = regs[rs2] ;

	uint64_t res = tmp > rs2_val ? tmp : rs2_val;

	store(regs[rs1], res, 64);

	if (rd == 0) return;
	regs[rd] = static_cast<int64_t>(tmp);
}

void VEmu::MRET()
{
	pc = load_csr(MEPC) - 4;

	uint8_t mb = (load_csr(MSTATUS) >> 11) & 0b11;

	if (mb == 0x00) mode = Mode::User;
	else if (mb == 0x01) mode = Mode::Supervisor;
	else if (mb == 0x11) mode = Mode::Machine;
	else assert(false);

	uint8_t MPIE = (load_csr(MSTATUS) >> 7) & 0b1;

	// csr[MSTATUS][MPI] = csr[MSTATUS][MPIE];
	if (MPIE) {
		store_csr(MSTATUS, load_csr(MSTATUS) | (1 << 3));
	} else {
		store_csr(MSTATUS, load_csr(MSTATUS) & (~(1 << 3)));
	}

	// csr[MSTATUS][MPIE] = 1
	store_csr(MSTATUS, load_csr(MSTATUS) | (1 << 7));

	// csr[MSTATUS][MPP] = 0

	store_csr(MSTATUS, load_csr(MSTATUS) & (~(0b11 << 11)));
}

void VEmu::SRET()
{
	std::cout << "No yet implemented.\n";
	exit(1);
}


void VEmu::XXX()
{
	std::ios_base::fmtflags ft { std::cout.flags() };
	std::cout << "Faulty instruction: 0x";
	std::cout << std::setw(8) 
		<< std::setfill('0') 
		<< std::hex 
		<< hex_instr 
		<< std::endl;

	std::cout.flags(ft);

	dump_regs();

	exit(EXIT_FAILURE);
}

