#include "../include/VEmu.h"

VEmu::VEmu(std::string f_name) :
    bin_file_name(std::move(f_name))
{
    mode = Mode::Machine;
    pc = 0x80000000;
    bus = Bus{};
    iregs = RegFile{};
    csrs.fill(0);
    iregs.store_reg(2, ADDR_BASE + DRAM::RAM_SIZE);
    read_file();
    init_func_map();
#ifdef TEST_ENV
    test_flag_done = false;
#endif
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

        {IName::MRET,     &VEmu::MRET},
        {IName::SRET,     &VEmu::SRET},

        {IName::FLW,      &VEmu::FLW},
        {IName::FSW,      &VEmu::FSW},
        {IName::FADDS,    &VEmu::FADDS},
        {IName::FMADDS,   &VEmu::FMADDS},
        {IName::FMSUBS,   &VEmu::FMSUBS},
        {IName::FNMSUBS,  &VEmu::FNMSUBS},
        {IName::FNMADDS,  &VEmu::FNMADDS},
        {IName::FADDS,    &VEmu::FADDS},
        {IName::FSUBS,    &VEmu::FSUBS},
        {IName::FMULS,    &VEmu::FMULS},
        {IName::FDIVS,    &VEmu::FDIVS},
        {IName::FSGNJS,   &VEmu::FSGNJS},
        {IName::FSGNJNS,  &VEmu::FSGNJNS},
        {IName::FSGNJXS,  &VEmu::FSGNJXS},
        {IName::FMINS,    &VEmu::FMINS},
        {IName::FMAXS,    &VEmu::FMAXS},

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

std::pair<uint64_t, ReturnException> VEmu::load(uint64_t addr, size_t sz)
{
    if (addr < ADDR_BASE) {
        return {0, ReturnException::LoadAccessFault};
    } else { 
        return {bus.load(addr, sz), ReturnException::NormalExecutionReturn};
    }
}

void VEmu::dump_regs()
{
    iregs.dump_regs();
}

ReturnException VEmu::store(uint64_t addr, uint64_t data, size_t sz)
{
    // FIXME: only checks on aligned addresses
    // Will have to check for individual bytes.
    // For example: if the address 0x81000000
    // is in the reservation set, storing in
    // the address 0x810000001 will not
    // mark the word as not reserved.
    if (addr < ADDR_BASE) {
        return ReturnException::StoreAMOAccessFault;
    }
    if (reservation_set.count(addr)) {
        reservation_set.erase(addr);
    }

    bus.store(addr, data, sz);

    return ReturnException::NormalExecutionReturn;
}

std::pair<uint32_t, ReturnException> VEmu::get_4byte_aligned_instr(uint64_t i)
{
    auto load_ret = load(i, 32);
    if (load_ret.second == ReturnException::NormalExecutionReturn) {
        return {
            static_cast<uint32_t>(load_ret.first),
            ReturnException::NormalExecutionReturn
        };
    } else {
        return {
            0,
            ReturnException::InstructionAccessFault
        };
    }
}

uint32_t VEmu::run()
{
    for (; pc < ADDR_BASE + code_size; pc += 4) {
        if (pc == 0x0) break;

#ifdef TEST_ENV
        if (test_flag_done) return 0;
#endif
        auto aligned_instr = get_4byte_aligned_instr(pc); 
        if (aligned_instr.second != ReturnException::NormalExecutionReturn)
            trap(aligned_instr.second);
        if (is_fatal(aligned_instr.second))
            exit_fatally(aligned_instr.second);

        hex_instr = aligned_instr.first;

        curr_instr = InstructionDecoder::the().decode(hex_instr);
        IName instr_iname = curr_instr.get_name();

        auto ret = (inst_funcs[instr_iname])(this);
        if (ret != ReturnException::NormalExecutionReturn)
            trap(ret);
        if (is_fatal(ret))
            exit_fatally(ret);
    }
    dump_csrs();
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

void VEmu::store_csr(uint64_t addr, uint64_t val)
{
    if (addr == SIE) {
        csrs[MIE] &= !csrs[MIDELEG];
        csrs[MIE] |= (val & csrs[MIDELEG]);
    } else {
        csrs[addr] = val;
    }
}

void VEmu::dump_csrs()
{
    std::cout << std::hex;
    std::cout << "mstatus: " << load_csr(MSTATUS) << std::endl;
    std::cout << "mtvec: " << load_csr(MTVEC) << std::endl;
    std::cout << "mepc: " << load_csr(MEPC) << std::endl;
    std::cout << "mcause: " << load_csr(MCAUSE) << std::endl;

    std::cout << "sstatus: " << load_csr(SSTATUS) << std::endl;
    std::cout << "stvec: " << load_csr(STVEC) << std::endl;
    std::cout << "sepc: " << load_csr(SEPC) << std::endl;
    std::cout << "scause: " << load_csr(SCAUSE) << std::endl;
}

ReturnException VEmu::LB()
{
    LBU();

    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(
        sext_byte(static_cast<uint8_t>(iregs.load_reg(rd) & 0xFF)));
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LW()
{
    LWU();

    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(
        sext_word(static_cast<uint32_t>(iregs.load_reg(rd) & 0xFFFFFFFF)));
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LBU()
{
    auto base_reg = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    uint64_t mem_addr =
        static_cast<uint64_t>(static_cast<int64_t>(iregs.load_reg(base_reg)) + imm);

    auto load_ret = load(mem_addr, 8);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    auto res = static_cast<int64_t>(load_ret.first);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LH()
{
    LHU();

    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(
        sext_hword(static_cast<uint16_t>(iregs.load_reg(rd) & 0xFFFF)));
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LHU()
{
    auto base_reg = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    uint64_t mem_addr =
        static_cast<uint64_t>(static_cast<int64_t>(iregs.load_reg(base_reg)) + imm);

    auto load_ret = load(mem_addr, 16);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    auto res = static_cast<int64_t>(load_ret.first);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LD()
{
    auto base_reg = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    uint64_t mem_addr =
        static_cast<uint64_t>(static_cast<int64_t>(iregs.load_reg(base_reg)) + imm);

    auto load_ret = load(mem_addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    auto res = static_cast<int64_t>(load_ret.first);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LWU()
{
    auto base_reg = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    uint64_t mem_addr =
        static_cast<uint64_t>(static_cast<int64_t>(iregs.load_reg(base_reg)) + imm);

    auto load_ret = load(mem_addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    auto res = static_cast<int64_t>(load_ret.first);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::ADDI()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) + imm;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::ADDIW()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int64_t op = iregs.load_reg(rs1);

    int32_t res_32 = static_cast<int32_t>((op + imm) & 0xFFFFFFFF);

    auto res = static_cast<int64_t>(res_32);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLTI()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    auto rd  = curr_instr.get_fields().rd;
    auto rs1 = curr_instr.get_fields().rs1;

    auto res = (iregs.load_reg(rs1) < imm) ? 1 : 0;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

// SLTIU rd, rs1, 1 sets rd to 1
// if rs1 == 0, otherwise it sets
// it to 0.
ReturnException VEmu::SLTIU()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    auto rd  = curr_instr.get_fields().rd;
    auto rs1 = curr_instr.get_fields().rs1;

    auto res = (static_cast<uint64_t>(iregs.load_reg(rs1)) < static_cast<uint64_t>(imm)) ?
            1 : 0;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::XORI()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) ^imm;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::ORI()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) | imm;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::ANDI()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) & imm;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLLI()
{
    uint8_t shamt =
        static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    uint64_t op = static_cast<uint64_t>(iregs.load_reg(rs1));
    op <<= shamt;

    auto res = static_cast<int64_t>(op);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRLI()
{
    uint8_t shamt =
        static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    uint64_t op = static_cast<uint64_t>(iregs.load_reg(rs1));
    op >>= shamt;

    auto res = static_cast<int64_t>(op);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRAI()
{
    uint8_t shamt =
        static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(iregs.load_reg(rs1) >> shamt);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLLIW()
{
    uint8_t shamt =
        static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    uint32_t op = static_cast<uint32_t>(iregs.load_reg(rs1));
    op <<= shamt;

    auto res = static_cast<int64_t>(
        static_cast<int32_t>(op)
    );
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRLIW()
{
    uint8_t shamt =
        static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    uint32_t op = static_cast<uint32_t>(iregs.load_reg(rs1));
    op >>= shamt;

    auto res = static_cast<int64_t>(
        static_cast<int32_t>(op)
    );
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRAIW()
{
    uint8_t shamt =
        static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t op = static_cast<int32_t>(iregs.load_reg(rs1));
    op >>= shamt;

    auto res = static_cast<int64_t>(op);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FENCE()
{

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FENCEI()
{

    return ReturnException::NormalExecutionReturn;
}

#ifndef TEST_ENV
ReturnException VEmu::ECALL()
{
    switch(mode) {
        case Mode::User:
            return ReturnException::EnvironmentCallFromUserMode;
        case Mode::Supervisor:
            return ReturnException::EnvironmentCallFromSupervisorMode;
        case Mode::Machine:
            return ReturnException::EnvironmentCallFromMachineMode;
        default:
            exit(EXIT_FAILURE);
    }

    exit(EXIT_FAILURE);
}
#else
ReturnException VEmu::ECALL()
{
    if (pc == pass_pc) {
        std::cout << "Passed test: " << bin_file_name << '\n';
    }
    else {
        std::cout << "Failed test: " << bin_file_name << "PC: " << 
            std:: hex << pc << '\n';
    }
    test_flag_done = true;
    return ReturnException::NormalExecutionReturn;
}
#endif

ReturnException VEmu::EBREAK()
{

    return ReturnException::InstructionAddressBreakpoint;
}

ReturnException VEmu::CSRRW()
{
    auto rd = curr_instr.get_fields().rd;
    auto rs1 = curr_instr.get_fields().rs1;
    uint64_t csr_addr = curr_instr.get_fields().imm;

    csr_addr &= 0xFFF;

    uint64_t csr_val = load_csr(csr_addr);
    store_csr(csr_addr, iregs.load_reg(rs1));

    auto res = csr_val;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::CSRRS()
{
    auto rd = curr_instr.get_fields().rd;
    auto rs1 = curr_instr.get_fields().rs1;
    uint64_t csr_addr = curr_instr.get_fields().imm;

    csr_addr &= 0xFFF;

    uint64_t csr_val = load_csr(csr_addr);
    store_csr(csr_addr, csr_val | iregs.load_reg(rs1));

    auto res = csr_val;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::CSRRC()
{
    auto rd = curr_instr.get_fields().rd;
    auto rs1 = curr_instr.get_fields().rs1;
    uint64_t csr_addr = curr_instr.get_fields().imm;

    csr_addr &= 0xFFF;

    uint64_t csr_val = load_csr(csr_addr);

    store_csr(csr_addr, csr_val & (!iregs.load_reg(rs1)));

    auto res = csr_val;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::CSRRWI()
{
    uint64_t uimm = static_cast<uint64_t>(curr_instr.get_fields().rs1);
    auto rd = curr_instr.get_fields().rd;

    uint64_t csr_addr = curr_instr.get_fields().imm;
    csr_addr &= 0xFFF;

    uint64_t csr_val = load_csr(csr_addr);
    store_csr(csr_addr, uimm);

    auto res = csr_val;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::CSRRSI()
{
    uint64_t uimm = static_cast<uint64_t>(curr_instr.get_fields().rs1);
    auto rd = curr_instr.get_fields().rd;

    uint64_t csr_addr = curr_instr.get_fields().imm;
    csr_addr &= 0xFFF;

    uint64_t csr_val = load_csr(csr_addr);
    store_csr(csr_addr, csr_val | uimm);

    auto res = csr_val;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::CSRRCI()
{
    uint64_t uimm = static_cast<uint64_t>(curr_instr.get_fields().rs1);
    auto rd = curr_instr.get_fields().rd;

    uint64_t csr_addr = curr_instr.get_fields().imm;
    csr_addr &= 0xFFF;

    uint64_t csr_val = load_csr(csr_addr);
    store_csr(csr_addr, csr_val & (!uimm));

    auto res = csr_val;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::BEQ()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    if (iregs.load_reg(rs1) == iregs.load_reg(rs2)) {
        this->pc += imm;
        this->pc -= 4;
    }


    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::BNE()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    if (iregs.load_reg(rs1) != iregs.load_reg(rs2)) {
        this->pc += imm;
        this->pc -= 4;
    }

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::BLT()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    if (iregs.load_reg(rs1) < iregs.load_reg(rs2)) {
        this->pc += imm;
        this->pc -= 4;
    }

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::BGE()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    if (iregs.load_reg(rs1) >= iregs.load_reg(rs2)) {
        this->pc += imm;
        this->pc -= 4;
    }

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::BLTU()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    if (static_cast<uint64_t>(iregs.load_reg(rs1)) < static_cast<uint64_t>(iregs.load_reg(rs2))) {
        this->pc += imm;
        this->pc -= 4;
    }

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::BGEU()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    if (static_cast<uint64_t>(iregs.load_reg(rs1)) >= static_cast<uint64_t>(iregs.load_reg(rs2))) {
        this->pc += imm;
        this->pc -= 4;
    }

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SB()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm);

    uint64_t data = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0xFF;
    auto store_ret = store(iregs.load_reg(rs1) + offset, data, 8);

    return store_ret;
}

ReturnException VEmu::SH()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm);

    uint64_t data = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0xFFFF;
    auto store_ret = store(iregs.load_reg(rs1) + offset, data, 16);

    return store_ret;
}

ReturnException VEmu::SW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm);

    uint64_t data = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0xFFFFFFFF;
    auto store_ret = store(iregs.load_reg(rs1) + offset, data, 32);

    return store_ret;
}

ReturnException VEmu::SD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    int32_t offset = static_cast<int32_t>(curr_instr.get_fields().imm);

    uint64_t data = static_cast<uint64_t>(iregs.load_reg(rs2));
    auto store_ret = store(iregs.load_reg(rs1) + offset, data, 64);

    return store_ret;
}

ReturnException VEmu::ADD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) + iregs.load_reg(rs2);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::ADDW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    int32_t op1 = static_cast<int32_t>(iregs.load_reg(rs1));
    int32_t op2 = static_cast<int32_t>(iregs.load_reg(rs2));

    auto res = static_cast<int64_t>(op1 + op2);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SUB()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) - iregs.load_reg(rs2);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SUBW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    int32_t op1 = static_cast<int32_t>(iregs.load_reg(rs1));
    int32_t op2 = static_cast<int32_t>(iregs.load_reg(rs2));

    auto res = static_cast<int64_t>(op1 - op2);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLL()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x3F;

    auto res = static_cast<int64_t>(
        static_cast<uint64_t>(iregs.load_reg(rs1)) << shamt
    );

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLLW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x1F;

    uint32_t op =  static_cast<uint32_t>(iregs.load_reg(rs1));

    op <<= shamt;

    // FIXME: This works only if the 32-bit value is sign-extended
    // and then put to the register.
    // (I'll have to zero extend the signed 32-bit value if not so.)
    auto res = static_cast<int64_t>(
        static_cast<int32_t>(op)
    );

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLT()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = (iregs.load_reg(rs1) < iregs.load_reg(rs2)) ? 1 : 0;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLTU()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res =
        (static_cast<uint64_t>(iregs.load_reg(rs1)) < static_cast<uint64_t>(iregs.load_reg(rs2))) ?
            1 : 0;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::XOR()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) ^iregs.load_reg(rs2);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::MUL()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) * iregs.load_reg(rs2);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::MULW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res =
            static_cast<int32_t>(iregs.load_reg(rs1) & 0xFFFFFFFF) * static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF);
    iregs.store_reg(rd, res);


    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::MULH()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(
        ((__int128)iregs.load_reg(rs1) * (__int128)iregs.load_reg(rs2)) >> 64
    );

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::MULHU()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint64_t urs1 = static_cast<uint64_t>(iregs.load_reg(rs1));
    uint64_t urs2 = static_cast<uint64_t>(iregs.load_reg(rs2));

    auto res = static_cast<int64_t>(
        (static_cast<unsigned __int128>(urs1) * static_cast<unsigned __int128>(urs2)) >> 64
    );

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::MULHSU()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    int64_t irs1 = iregs.load_reg(rs1);
    uint64_t urs2 = static_cast<uint64_t>(iregs.load_reg(rs2));

    unsigned __int128 rs1_val =
        static_cast<unsigned __int128>(static_cast<__int128>(irs1));
    unsigned __int128 rs2_val =
        static_cast<unsigned __int128>(urs2);

    auto res = static_cast<int64_t>((rs1_val * rs2_val) >> 64);

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::DIV()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    int64_t res;

    if (iregs.load_reg(rs2) == 0) {
        res = 0xFFFFFFFF'FFFFFFFF;
    } else if (iregs.load_reg(rs1) == INT64_MIN && iregs.load_reg(rs2) == -1) {
        res = iregs.load_reg(rs1);
    } else {
        res = iregs.load_reg(rs1) / iregs.load_reg(rs2);
    }

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::DIVU()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    int64_t res;

    if (iregs.load_reg(rs2) == 0) {
        res = 0xFFFFFFFF'FFFFFFFF;
        iregs.store_reg(rd, res);
    } else {
        res = static_cast<int64_t>(
            static_cast<uint64_t>(iregs.load_reg(rs1)) / static_cast<uint64_t>(iregs.load_reg(rs2))
        );
        iregs.store_reg(rd, res);
    }

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::DIVW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    int32_t rs1_32 = static_cast<int32_t>(iregs.load_reg(rs1));
    int32_t rs2_32 = static_cast<int32_t>(iregs.load_reg(rs2));

    int64_t res;

    if (rs2_32 == 0) {
        res = 0xFFFFFFFF'FFFFFFFF;
    } else if (rs2_32 == -1 && rs1_32 == INT32_MIN) {
        res = static_cast<int64_t>(static_cast<int32_t>(rs1_32));
    }
    else {
        res = static_cast<int64_t>(rs1_32 / rs2_32);
    }

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::DIVUW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint32_t rs1_32 = static_cast<uint32_t>(iregs.load_reg(rs1));
    uint32_t rs2_32 = static_cast<uint32_t>(iregs.load_reg(rs2));

    int64_t res;

    if (rs2_32 == 0) {
        res = 0xFFFFFFFF'FFFFFFFF;
    }
    else {
        res = static_cast<int64_t>(static_cast<int32_t>(rs1_32 / rs2_32));
    }

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::REM()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    int64_t res;

    if (iregs.load_reg(rs2) == 0) {
        res = iregs.load_reg(rs1);
    } else if (iregs.load_reg(rs1) == INT64_MIN && iregs.load_reg(rs2) == -1) {
        res = 0;
    } else {
        res = iregs.load_reg(rs1) % iregs.load_reg(rs2);
    }
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::REMU()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    int64_t res;

    if (iregs.load_reg(rs2) == 0) {
        res = iregs.load_reg(rs1);
        iregs.store_reg(rd, res);
    } else {
        res = static_cast<int64_t>(
            static_cast<uint64_t>(iregs.load_reg(rs1)) % static_cast<uint64_t>(iregs.load_reg(rs2))
        );
        iregs.store_reg(rd, res);
    }

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::REMW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    int32_t rs1_32 = static_cast<int32_t>(iregs.load_reg(rs1));
    int32_t rs2_32 = static_cast<int32_t>(iregs.load_reg(rs2));

    int64_t res;

    if (rs2_32 == 0) {
        res = static_cast<int64_t>(rs1_32);
    } else if (rs2_32 == -1 && rs1_32 == INT32_MIN) {
        res = 0;
    } else {
        res =
                static_cast<int32_t>(iregs.load_reg(rs1)) % static_cast<int32_t>(iregs.load_reg(rs2));
    }
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::REMUW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint32_t rs1_32 = static_cast<uint32_t>(iregs.load_reg(rs1));
    uint32_t rs2_32 = static_cast<uint32_t>(iregs.load_reg(rs2));

    int64_t res;

    if (rs2_32 == 0) {
        res = static_cast<int64_t>(static_cast<int32_t>(rs1_32));
    } else {
        res = static_cast<int64_t>(static_cast<int32_t>(rs1_32 % rs2_32));
    }

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRL()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x3F;

    auto res = static_cast<int64_t>(
        static_cast<uint64_t>(iregs.load_reg(rs1)) >> shamt
    );
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRLW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x1F;

    uint32_t op =  static_cast<uint32_t>(iregs.load_reg(rs1));

    op >>= shamt;

    auto res = static_cast<int64_t>(
        static_cast<int32_t>(op)
    );

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRA()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x3F;

    auto res = static_cast<int64_t>(
        iregs.load_reg(rs1) >> shamt
    );
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRAW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x1F;

    int32_t op =  static_cast<int32_t>(iregs.load_reg(rs1));

    op >>= shamt;

    auto res = static_cast<int64_t>(op);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::OR()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) | iregs.load_reg(rs2);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AND()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) & iregs.load_reg(rs2);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::JAL()
{
    auto rd = curr_instr.get_fields().rd;
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    auto res = static_cast<int64_t>(this->pc + 4);
    iregs.store_reg(rd, res);

    this->pc += imm;
    this->pc -= 4;

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::JALR()
{
    auto rd = curr_instr.get_fields().rd;
    auto rs1 = curr_instr.get_fields().rs1;
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    auto res = static_cast<int64_t>(this->pc + 4);
    iregs.store_reg(rd, res);

    this->pc = static_cast<uint64_t>(iregs.load_reg(rs1) + imm);
    this->pc &= ~(0x1);
    this->pc -= 4;

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LUI()
{
    // the full 32-bit U-imm
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(imm_32);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AUIPC()
{
    // the full 32-bit U-imm
    auto rd = curr_instr.get_fields().rd;
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    auto res = this->pc + imm;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LRW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    reservation_set.insert(addr);

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    iregs.store_reg(rd, load_ret.first);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LRD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    reservation_set.insert(addr);

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }
    iregs.store_reg(rd, load_ret.first);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SCW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;
    uint64_t addr = iregs.load_reg(rs1);

    if (addr % 4 != 0) {
        return ReturnException::StoreAMOAddressMisaligned;
    }

    if (!reservation_set.count(addr)) {
        reservation_set.erase(addr);
        auto store_ret = store(addr, iregs.load_reg(rs2), 32);
        iregs.store_reg(rd, 0);
        return store_ret;
    } else {
        iregs.store_reg(rd, 1);
        return ReturnException::NormalExecutionReturn;
    }
}

ReturnException VEmu::SCD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;
    uint64_t addr = iregs.load_reg(rs1);

    if (addr % 8 != 0) {
        return ReturnException::StoreAMOAddressMisaligned;
    }

    if (!reservation_set.count(addr)) {
        reservation_set.erase(addr);
        auto store_ret = store(addr, iregs.load_reg(rs2), 64);
        iregs.store_reg(rd, 0);
        return store_ret;
    } else {
        iregs.store_reg(rd, 1);
        return ReturnException::NormalExecutionReturn;
    }
}

ReturnException VEmu::AMOSWAPW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int32_t tmp = static_cast<int32_t>(load_ret.first & 0xFFFFFFFF);
    
    auto store_ret = store(addr, static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF), 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    auto res = static_cast<int64_t>(tmp);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOADDW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int32_t tmp = static_cast<int32_t>(load_ret.first & 0xFFFFFFFF);
    int32_t rs2_val = static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF);

    int32_t res_32 = tmp + rs2_val;
    uint64_t res = static_cast<uint64_t>(static_cast<uint32_t>(res_32));

    auto store_ret = store(addr, res, 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOANDW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int32_t tmp = static_cast<int32_t>(load_ret.first & 0xFFFFFFFF);
    int32_t rs2_val = static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF);

    uint64_t res = tmp & rs2_val;
    res &= 0xFFFFFFFF;

    auto store_ret = store(addr, res, 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOORW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int32_t tmp = static_cast<int32_t>(load_ret.first & 0xFFFFFFFF);
    int32_t rs2_val = static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF);

    uint64_t res = (tmp | rs2_val);

    res &= 0xFFFFFFFF;

    auto store_ret = store(addr, res, 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOXORW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int32_t tmp = static_cast<int32_t>(load_ret.first & 0xFFFFFFFF);
    int32_t rs2_val = static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF);

    uint64_t res = (tmp ^ rs2_val);

    res &= 0xFFFFFFFF;

    auto store_ret = store(addr, res, 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOMINW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int32_t tmp = static_cast<int32_t>(load_ret.first & 0xFFFFFFFF);
    int32_t rs2_val = static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF);

    uint64_t res = tmp < rs2_val ?
        static_cast<uint64_t>(static_cast<uint32_t>(tmp)) :
        static_cast<uint64_t>(static_cast<uint32_t>(rs2_val));

    auto store_ret = store(addr, res, 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOMAXW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int32_t tmp = static_cast<int32_t>(load_ret.first & 0xFFFFFFFF);
    int32_t rs2_val = static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF);

    uint64_t res = tmp > rs2_val ?
        static_cast<uint64_t>(static_cast<uint32_t>(tmp)) :
        static_cast<uint64_t>(static_cast<uint32_t>(rs2_val));

    auto store_ret = store(addr, res, 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOMINUW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    uint32_t tmp = static_cast<uint32_t>(load_ret.first);
    uint32_t rs2_val = static_cast<uint32_t>(iregs.load_reg(rs2));

    uint64_t res = tmp < rs2_val ?
        static_cast<uint64_t>(tmp) : static_cast<uint64_t>(rs2_val);

    auto store_ret = store(addr, res, 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, sext_word(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOMAXUW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 4 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 32);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    uint32_t tmp = static_cast<uint32_t>(load_ret.first);
    uint32_t rs2_val = static_cast<uint32_t>(iregs.load_reg(rs2));

    uint32_t res = tmp > rs2_val ? tmp : rs2_val;

    auto store_ret = store(addr, res, 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, sext_word(tmp));

    return ReturnException::NormalExecutionReturn;
}


ReturnException VEmu::AMOSWAPD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    auto store_ret = store(addr, iregs.load_reg(rs2), 64);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(load_ret.first));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOADDD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int64_t tmp = static_cast<int64_t>(load_ret.first);
    int64_t rs2_val = iregs.load_reg(rs2) ;

    int64_t res = tmp + rs2_val;

    auto store_ret = store(addr, res, 64);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, tmp);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOXORD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    uint64_t tmp = load_ret.first;
    uint64_t rs2_val = iregs.load_reg(rs2) ;

    uint64_t res = tmp ^ rs2_val;

    auto store_ret = store(addr, res, 64);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOANDD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    uint64_t tmp = load_ret.first;
    uint64_t rs2_val = iregs.load_reg(rs2) ;

    uint64_t res = tmp & rs2_val;

    auto store_ret = store(addr, res, 64);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOORD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    uint64_t tmp = load_ret.first;
    uint64_t rs2_val = iregs.load_reg(rs2) ;

    uint64_t res = tmp | rs2_val;

    auto store_ret = store(addr, res, 64);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOMIND()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int64_t tmp = static_cast<int64_t>(load_ret.first);
    int64_t rs2_val = iregs.load_reg(rs2);

    uint64_t res = tmp < rs2_val ? tmp : rs2_val;

    auto store_ret = store(addr, res, 64);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, tmp);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOMAXD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    int64_t tmp = static_cast<int64_t>(load_ret.first);
    int64_t rs2_val = iregs.load_reg(rs2) ;

    int64_t res = tmp > rs2_val ? tmp : rs2_val;

    auto store_ret = store(addr, static_cast<uint64_t>(res), 64);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, tmp);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOMINUD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    uint64_t tmp = load_ret.first;
    uint64_t rs2_val = iregs.load_reg(rs2) ;

    uint64_t res = tmp < rs2_val ? tmp : rs2_val;

    auto store_ret = store(addr, static_cast<uint64_t>(res), 64);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AMOMAXUD()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto addr = iregs.load_reg(rs1);
    if (addr % 8 != 0) {
        return ReturnException::LoadAddressMisaligned;
    }

    auto load_ret = load(addr, 64);
    if (load_ret.second != ReturnException::NormalExecutionReturn) {
        return load_ret.second;
    }

    uint64_t tmp = load_ret.first;
    uint64_t rs2_val = iregs.load_reg(rs2) ;

    uint64_t res = tmp > rs2_val ? tmp : rs2_val;

    auto store_ret = store(iregs.load_reg(rs1), res, 64);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, static_cast<int64_t>(tmp));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::MRET()
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

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRET()
{
    pc = load_csr(SEPC) - 4;

    uint8_t mb = (load_csr(SSTATUS) >> 8) & 0b1;

    if (mb == 0x0) mode = Mode::User;
    else mode = Mode::Supervisor;

    uint8_t SPIE = (load_csr(SSTATUS) >> 5) & 1;

    // SSTATUS[SIE] = SSTATUS[SPIE]
    if (SPIE) {
        store_csr(SSTATUS, load_csr(SSTATUS) | (1 << 1));
    } else {
        store_csr(SSTATUS, load_csr(SSTATUS) & (~(1 << 1)));
    }

    // SSTATUS[SPIE] = 1
    store_csr(SSTATUS, load_csr(SSTATUS) | (1 << 5));

    // SSTATUS[SPP] = 0
    store_csr(SSTATUS, load_csr(SSTATUS) & (~(1 << 8)));

    return ReturnException::NormalExecutionReturn;
}


ReturnException VEmu::XXX()
{
    std::ios_base::fmtflags ft { std::cout.flags() };
    std::cout << "Faulty instruction: 0x";
    std::cout << std::setw(8)
        << std::setfill('0')
        << std::hex
        << hex_instr
        << "PC: "
        << pc
        << std::endl;


    std::cout.flags(ft);
    return ReturnException::IllegalInstruction;
}

void VEmu::trap(ReturnException e)
{
    uint64_t exception_pc = pc;
    uint8_t cause = static_cast<uint8_t>(e);
    Mode prev_mode = mode;

    /* Exceptions are handled usually in the Machine mode
       privilege level. However, some exceptions can be
       marked to be done at a lower privilege level. This marking
       is saved in the MEDELEG control and status register. When
       the current mode is user-mode, the exception is handled
    */
    bool do_deleg = (load_csr(MEDELEG) >> cause) & 1;

    if (prev_mode == Mode::User && do_deleg) {
        mode = Mode::Supervisor;

        pc = load_csr(STVEC) & (~(0b11));
        pc -= 4;

        store_csr(SEPC, exception_pc & (~1));

        store_csr(SCAUSE, cause);

        store_csr(STVAL, 0);

        // set previous interrupt enable to the current global
        // interrupt enable.
        uint8_t sie = (load_csr(SSTATUS) >> 1) & 1;
        if (sie) {
            store_csr(SSTATUS, load_csr(SSTATUS) | (1 << 5));
        } else {
            store_csr(SSTATUS, load_csr(SSTATUS) & (~(1 << 5)));
        }

        // set global interrupt bit to 0.
        store_csr(SSTATUS, load_csr(SSTATUS) & (~(1 << 1)));

        // SPP is set to 0 if previous mode is User mode,
        // 1 otherwise.
        if (prev_mode == Mode::User) {
            store_csr(SSTATUS, load_csr(SSTATUS) & (~(1 << 8)));
        } else {
            store_csr(SSTATUS, load_csr(SSTATUS) | (1 << 8));
        }
    } else {
        mode = Mode::Machine;

        pc = load_csr(MTVEC) & (~(0b11));
        pc -= 4;

        store_csr(MEPC, exception_pc & (~1));

        store_csr(MCAUSE, cause);

        store_csr(MTVAL, 0);

        // set previous interrupt enable to the current global
        // interrupt enable.
        uint8_t mie = (load_csr(MSTATUS) >> 3) & 1;
        if (mie == 1) {
            store_csr(MSTATUS, load_csr(MSTATUS) | (1 << 7));
        } else {
            store_csr(MSTATUS, load_csr(MSTATUS) & (~(1 << 7)));
        }

        // set global interrupt bit to 0.
        store_csr(MSTATUS, load_csr(MSTATUS) & (~(1 << 3)));

        switch (prev_mode) {
            case Mode::User:
                store_csr(MSTATUS, load_csr(MSTATUS) & (~(0b11 << 11)));
                break;
            case Mode::Supervisor:
                store_csr(MSTATUS, load_csr(MSTATUS) & (~(0b1 << 12)));
                store_csr(MSTATUS, load_csr(MSTATUS) | (0b1 << 11));
                break;
            case Mode::Machine:
                store_csr(MSTATUS, load_csr(MSTATUS) | (0b11 << 11));
                break;
            default:
                std::cout << "Unsupported(?) privilege mode.\n";
                exit(EXIT_FAILURE);
        }
    }
}

bool VEmu::is_fatal(ReturnException e)
{
    return e == ReturnException::LoadAccessFault ||
           e == ReturnException::InstructionAccessFault ||
           e == ReturnException::InstructionAddressMisaligned ||
           e == ReturnException::StoreAMOAddressMisaligned ||
           e == ReturnException::StoreAMOAccessFault;
}

void VEmu::exit_fatally(ReturnException e)
{
    std::cout << "Exit on exception: " << stringify_exception(e);
    std::cout << "\n" << std::hex << pc << std::endl;

    dump_regs();

    exit(EXIT_FAILURE);
}

std::string VEmu::stringify_exception(ReturnException e)
{
    if (e == ReturnException::EnvironmentCallFromUserMode)
        return "EnvironmentCallFromUserMode";
    else
        return "Exception";
}

ReturnException VEmu::FLW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto load_res = load(iregs.load_reg(rs1), 32);
    if (load_res.second != ReturnException::NormalExecutionReturn)
        return load_res.second;

    union {
        uint32_t i;
        float s;
    } a{};
    a.i = static_cast<uint32_t>(load_res.first);

    fregs.store_reg(rd, static_cast<double>(a.s));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FSW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;

    union {
        uint32_t i;
        float s;
    } a{};

    a.s = static_cast<float>(fregs.load_reg(rs2));
    auto store_res = store(iregs.load_reg(rs1), static_cast<uint64_t>(a.i), 32);

    return store_res;
}

ReturnException VEmu::FMADDS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rs3 = curr_instr.get_fields().rs3;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));
    float op3 = static_cast<float>(fregs.load_reg(rs3));

    float res = std::fma(op1, op2, op3);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FMSUBS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rs3 = curr_instr.get_fields().rs3;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));
    float op3 = static_cast<float>(fregs.load_reg(rs3));

    float res = std::fma(op1, op2, -op3);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FNMSUBS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rs3 = curr_instr.get_fields().rs3;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));
    float op3 = static_cast<float>(fregs.load_reg(rs3));

    float res = std::fma(-op1, op2, -op3);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FNMADDS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rs3 = curr_instr.get_fields().rs3;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));
    float op3 = static_cast<float>(fregs.load_reg(rs3));

    float res = std::fma(-op1, op2, op3);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FADDS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    float res = op1 + op2;

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FSUBS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    float res = op1 - op2;

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FMULS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    float res = op1 * op2;

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FDIVS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    float res = op1 / op2;

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FSQRTS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op = static_cast<float>(fregs.load_reg(rs1));

    float res = std::sqrt(op);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FSGNJS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    float res = copysignf(op1, op2);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FSGNJNS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    float res = copysignf(op1, -op2);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FSGNJXS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    bool s1 = (op1 < 0.0);
    bool s2 = (op2 < 0.0);

    float tmp = (s1 ^ s2) ? -1.0f : 1.0f;

    float res = copysignf(op1, tmp);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
}

bool VEmu::is_negative_zero(double d)
{
    return d == 0.0 && *((int *)(&d)) != 0;
}

bool VEmu::is_positive_zero(double d)
{
    return d == 0.0 && *((int *)(&d)) == 0;
}

ReturnException VEmu::FMINS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    bool op1_positive_zero = is_positive_zero(static_cast<double>(op1));
    bool op2_positive_zero = is_positive_zero(static_cast<double>(op2));

    bool op1_negative_zero = is_negative_zero(static_cast<double>(op1));
    bool op2_negative_zero = is_negative_zero(static_cast<double>(op2));

    if (op1_positive_zero && op2_negative_zero) {
        fregs.store_reg(rd, static_cast<double>(op2));
    } else if (op1_negative_zero && op2_positive_zero) {
        fregs.store_reg(rd, static_cast<double>(op1));
    } else {
        float res = std::fminf(op1, op2);
        fregs.store_reg(rd, static_cast<double>(res));
    }

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FMAXS()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    bool op1_positive_zero = is_positive_zero(static_cast<double>(op1));
    bool op2_positive_zero = is_positive_zero(static_cast<double>(op2));

    bool op1_negative_zero = is_negative_zero(static_cast<double>(op1));
    bool op2_negative_zero = is_negative_zero(static_cast<double>(op2));

    if (op1_positive_zero && op2_negative_zero) {
        fregs.store_reg(rd, static_cast<double>(op1));
    } else if (op1_negative_zero && op2_positive_zero) {
        fregs.store_reg(rd, static_cast<double>(op2));
    } else {
        float res = std::fmaxf(op1, op2);
        fregs.store_reg(rd, static_cast<double>(res));
    }

    return ReturnException::NormalExecutionReturn;
}
