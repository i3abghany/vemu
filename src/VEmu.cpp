#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <type_traits>
#include <unistd.h>

#include <VEmu.h>

#ifdef SUPPORT_SOFTFLOAT
extern "C" {
#include <softfloat.h>
};
#endif

VEmu::VEmu(std::string f_name, uint64_t start_pc, uint64_t mem_size)
    : bin_file_name(std::move(f_name))
    , bus(mem_size)
    , pc(start_pc)
    , ram_size(mem_size)
{
    mode = Mode::Machine;
    iregs = RegFile {};
    fregs = FRegFile {};
#ifndef FUZZ_ENV
    mode = Mode::User;
    csrs.fill(0);
    init_misa();
#endif
    init_func_map();
    if (bin_file_name != "")
        read_file();
}

VEmu::VEmu(std::string f_name, const FileInfo& info, const std::string& arg,
           uint64_t mem_size)
    : VEmu("", info.entry_point, mem_size)
{
    bin_file_name = std::move(f_name);
    bus.get_mmu()->load_file(info);
    file_info = info;
    string_arg = arg;

    static constexpr size_t STACK_SIZE = 1 * 1024 * 1024;
    auto stack_base = bus.get_mmu()->allocate(STACK_SIZE);
    iregs.store_reg(REG_SP, stack_base + STACK_SIZE);

    push_to_stack(0, 64);
    push_to_stack(0, 64);

    // argv end
    push_to_stack(0, 64);

    auto argv2 = bus.get_mmu()->allocate(256);
    write_string_to_addr(arg, argv2);
    push_to_stack(argv2, 64);

    auto argv1 = bus.get_mmu()->allocate(8);
    write_string_to_addr("-x", argv1);
    push_to_stack(argv1, 64);

    auto argv0 = bus.get_mmu()->allocate(256);
    write_string_to_addr(bin_file_name, argv0);
    push_to_stack(argv0, 64);

    static constexpr uint64_t argc = 3;
    push_to_stack(argc, 64);
}

VEmu::VEmu(const std::vector<uint8_t>& bytes, uint64_t start_pc, uint64_t mem_size)
    : VEmu("", start_pc, mem_size)
{
    code_size = bytes.size();
    auto aligned_code_size = (code_size + 0xFFFF) & ~0xFFFF;
    auto base = bus.get_mmu()->allocate(aligned_code_size);
    pc = base;
    bus.get_mmu()->set_perms(base, code_size, PERM_WRITE);
    bus.get_mmu()->write_from(bytes, base);
    bus.get_mmu()->set_perms(base, code_size, PERM_EXEC | PERM_READ);
}

void VEmu::init_misa()
{
    uint64_t misa = 0;
    misa |= (1 << 20); // User-mode
    misa |= (1 << 18); // Supervisor-mode
    misa |= (1 << 12); // Integer multiply/divide
    misa |= (1 << 8); // Base integer ISA
    misa |= (1 << 5); // Single-precision floating-point
    misa |= (1 << 0); // Atomic ISA

    store_csr(MISA, misa);
}

void VEmu::push_to_stack(uint64_t data, size_t sz)
{
    auto sp = iregs.load_reg(2) - (sz / 8);
    store(sp, data, sz);
    iregs.store_reg(2, sp);
}

void VEmu::write_string_to_addr(const std::string& arg, uint64_t addr)
{
    std::vector<uint8_t> p(arg.size() + 1);
    for (size_t i = 0; i < p.size(); i++)
        p[i] = arg[i];
    p[arg.size()] = '\0';
    bus.get_mmu()->write_from(p, addr);
}

void VEmu::init_func_map()
{
    inst_funcs = {
        {IName::LB,        &VEmu::LB      },
        { IName::LH,       &VEmu::LH      },
        { IName::LW,       &VEmu::LW      },
        { IName::LBU,      &VEmu::LBU     },
        { IName::LHU,      &VEmu::LHU     },
        { IName::LD,       &VEmu::LD      },
        { IName::LWU,      &VEmu::LWU     },
        { IName::ADDI,     &VEmu::ADDI    },
        { IName::ADDIW,    &VEmu::ADDIW   },
        { IName::SLTI,     &VEmu::SLTI    },
        { IName::SLTIU,    &VEmu::SLTIU   },
        { IName::XORI,     &VEmu::XORI    },
        { IName::ORI,      &VEmu::ORI     },
        { IName::OR,       &VEmu::OR      },
        { IName::ANDI,     &VEmu::ANDI    },
        { IName::AND,      &VEmu::AND     },
        { IName::SLLI,     &VEmu::SLLI    },
        { IName::SRLI,     &VEmu::SRLI    },
        { IName::SRA,      &VEmu::SRA     },
        { IName::SRAI,     &VEmu::SRAI    },
        { IName::SLLIW,    &VEmu::SLLIW   },
        { IName::SRLIW,    &VEmu::SRLIW   },
        { IName::SRAIW,    &VEmu::SRAIW   },
        { IName::SRAW,     &VEmu::SRAW    },
        { IName::SRLW,     &VEmu::SRLW    },
        { IName::FENCE,    &VEmu::FENCE   },
        { IName::FENCEI,   &VEmu::FENCEI  },
        { IName::ECALL,    &VEmu::ECALL   },
        { IName::EBREAK,   &VEmu::EBREAK  },
        { IName::CSRRW,    &VEmu::CSRRW   },
        { IName::CSRRS,    &VEmu::CSRRS   },
        { IName::CSRRC,    &VEmu::CSRRC   },
        { IName::CSRRWI,   &VEmu::CSRRWI  },
        { IName::CSRRSI,   &VEmu::CSRRSI  },
        { IName::CSRRCI,   &VEmu::CSRRCI  },
        { IName::BEQ,      &VEmu::BEQ     },
        { IName::BNE,      &VEmu::BNE     },
        { IName::BLT,      &VEmu::BLT     },
        { IName::BGE,      &VEmu::BGE     },
        { IName::BLTU,     &VEmu::BLTU    },
        { IName::BGEU,     &VEmu::BGEU    },
        { IName::SB,       &VEmu::SB      },
        { IName::SH,       &VEmu::SH      },
        { IName::SW,       &VEmu::SW      },
        { IName::SD,       &VEmu::SD      },
        { IName::ADD,      &VEmu::ADD     },
        { IName::ADDW,     &VEmu::ADDW    },
        { IName::SUB,      &VEmu::SUB     },
        { IName::SUBW,     &VEmu::SUBW    },
        { IName::SLL,      &VEmu::SLL     },
        { IName::SLLW,     &VEmu::SLLW    },
        { IName::SLT,      &VEmu::SLT     },
        { IName::SLTU,     &VEmu::SLTU    },
        { IName::XOR,      &VEmu::XOR     },
        { IName::SRL,      &VEmu::SRL     },
        { IName::SRLW,     &VEmu::SRLW    },
        { IName::LUI,      &VEmu::LUI     },
        { IName::AUIPC,    &VEmu::AUIPC   },
        { IName::JAL,      &VEmu::JAL     },
        { IName::JALR,     &VEmu::JALR    },
        { IName::MUL,      &VEmu::MUL     },
        { IName::MULH,     &VEmu::MULH    },
        { IName::MULHSU,   &VEmu::MULHSU  },
        { IName::MULHU,    &VEmu::MULHU   },
        { IName::DIV,      &VEmu::DIV     },
        { IName::DIVU,     &VEmu::DIVU    },
        { IName::REM,      &VEmu::REM     },
        { IName::REMU,     &VEmu::REMU    },
        { IName::MULW,     &VEmu::MULW    },
        { IName::DIVW,     &VEmu::DIVW    },
        { IName::DIVUW,    &VEmu::DIVUW   },
        { IName::REMW,     &VEmu::REMW    },
        { IName::REMUW,    &VEmu::REMUW   },

        { IName::AMOSWAPW, &VEmu::AMOSWAPW},
        { IName::AMOADDW,  &VEmu::AMOADDW },
        { IName::AMOXORW,  &VEmu::AMOXORW },
        { IName::AMOANDW,  &VEmu::AMOANDW },
        { IName::AMOORW,   &VEmu::AMOORW  },
        { IName::AMOMINW,  &VEmu::AMOMINW },
        { IName::AMOMAXW,  &VEmu::AMOMAXW },
        { IName::AMOMINUW, &VEmu::AMOMINUW},
        { IName::AMOMAXUW, &VEmu::AMOMAXUW},
        { IName::LRW,      &VEmu::LRW     },
        { IName::SCW,      &VEmu::SCW     },

        { IName::AMOSWAPD, &VEmu::AMOSWAPD},
        { IName::AMOADDD,  &VEmu::AMOADDD },
        { IName::AMOXORD,  &VEmu::AMOXORD },
        { IName::AMOANDD,  &VEmu::AMOANDD },
        { IName::AMOORD,   &VEmu::AMOORD  },
        { IName::AMOMIND,  &VEmu::AMOMIND },
        { IName::AMOMAXD,  &VEmu::AMOMAXD },
        { IName::AMOMINUD, &VEmu::AMOMINUD},
        { IName::AMOMAXUD, &VEmu::AMOMAXUD},
        { IName::LRD,      &VEmu::LRD     },
        { IName::SCD,      &VEmu::SCD     },

        { IName::MRET,     &VEmu::MRET    },
        { IName::SRET,     &VEmu::SRET    },

        { IName::FLW,      &VEmu::FLW     },
        { IName::FSW,      &VEmu::FSW     },
        { IName::FMADDS,   &VEmu::FMADDS  },
        { IName::FMSUBS,   &VEmu::FMSUBS  },
        { IName::FNMSUBS,  &VEmu::FNMSUBS },
        { IName::FNMADDS,  &VEmu::FNMADDS },
        { IName::FADDS,    &VEmu::FADDS   },
        { IName::FSUBS,    &VEmu::FSUBS   },
        { IName::FMULS,    &VEmu::FMULS   },
        { IName::FDIVS,    &VEmu::FDIVS   },
        { IName::FSGNJS,   &VEmu::FSGNJS  },
        { IName::FSGNJNS,  &VEmu::FSGNJNS },
        { IName::FSGNJXS,  &VEmu::FSGNJXS },
        { IName::FMINS,    &VEmu::FMINS   },
        { IName::FMAXS,    &VEmu::FMAXS   },
        { IName::FCVTWS,   &VEmu::FCVTWS  },
        { IName::FCVTWUS,  &VEmu::FCVTWUS },
        { IName::FMVXW,    &VEmu::FMVXW   },
        { IName::FEQS,     &VEmu::FEQS    },
        { IName::FLTS,     &VEmu::FLTS    },
        { IName::FLES,     &VEmu::FLES    },
        { IName::FCVTSW,   &VEmu::FCVTSW  },
        { IName::FCVTSWU,  &VEmu::FCVTSWU },
        { IName::FMVWX,    &VEmu::FMVWX   },
        { IName::FCVTLS,   &VEmu::FCVTLS  },
        { IName::FCVTLUS,  &VEmu::FCVTLUS },
        { IName::FCVTSL,   &VEmu::FCVTSL  },
        { IName::FCVTSLU,  &VEmu::FCVTSLU },
        { IName::FCLASSS,  &VEmu::FCLASSS },

        { IName::XXX,      &VEmu::XXX     },
    };
}

void VEmu::read_file()
{
    std::ifstream ifs(bin_file_name, std::ios_base::binary);
    if (!ifs) {
        std::cout << "Could not open the file " << bin_file_name << "\n";
        exit(EXIT_FAILURE);
    }
    std::filesystem::path file_path { bin_file_name };
    auto sz = std::filesystem::file_size(file_path);
    code_size = static_cast<uint64_t>(sz);
    auto aligned_code_size = (code_size + 0xFFFF) & ~0xFFFF;
    auto base = bus.get_mmu()->allocate(aligned_code_size);
    pc = base;
    std::vector<uint8_t> content;
    content.reserve(code_size);
    std::copy_n(std::istreambuf_iterator<char>(ifs), code_size,
                std::back_inserter<std::vector<uint8_t>>(content));
    bus.get_mmu()->set_perms(base, code_size, PERM_WRITE);
    bus.get_mmu()->write_from(content, base);
    bus.get_mmu()->set_perms(base, code_size, PERM_EXEC | PERM_READ);
}

std::pair<uint64_t, ReturnException> VEmu::load(uint64_t addr, size_t sz)
{
    return bus.load(addr, sz);
}

void VEmu::dump_regs() { iregs.dump_regs(); }

std::array<int64_t, 32> VEmu::get_iregs() { return iregs.get_regs(); }

std::array<double, 32> VEmu::get_fregs() { return fregs.get_regs(); }

ReturnException VEmu::store(uint64_t addr, uint64_t data, size_t sz)
{
    return bus.store(addr, data, sz);
}

std::pair<uint32_t, ReturnException> VEmu::get_4byte_aligned_instr(uint64_t i)
{
#ifdef TEST_ENV
    auto load_ret = load(i, 32);
    return load_ret;
    if (load_ret.second == ReturnException::NormalExecutionReturn) {
        return { static_cast<uint32_t>(load_ret.first),
                 ReturnException::NormalExecutionReturn };
    } else {
        return { 0, ReturnException::InstructionAccessFault };
    }
#else
    auto load_ret = bus.get_mmu()->load_insn(i);
    return load_ret;
#endif
}

uint32_t VEmu::run()
{
    for (;; pc += 4) {
#ifndef FUZZ_ENV
        Interrupt i = check_pending_interrupt();
        if (i != Interrupt::NoInterrupt) {
            take_interrupt(i);
        }
#endif

#ifdef TEST_ENV
        if (test_flag_done)
            return 0;
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
    std::cout << "mstatus: " << load_csr(MSTATUS) << '\n';
    std::cout << "mtvec: " << load_csr(MTVEC) << '\n';
    std::cout << "mepc: " << load_csr(MEPC) << '\n';
    std::cout << "mcause: " << load_csr(MCAUSE) << '\n';

    std::cout << "sstatus: " << load_csr(SSTATUS) << '\n';
    std::cout << "stvec: " << load_csr(STVEC) << '\n';
    std::cout << "sepc: " << load_csr(SEPC) << '\n';
    std::cout << "scause: " << load_csr(SCAUSE) << '\n';
}

template <typename T> uint64_t sext_from(T w)
{
    return static_cast<uint64_t>(
        static_cast<int64_t>(static_cast<typename std::make_signed<T>::type>(w)));
}

ReturnException VEmu::LB()
{
    LBU();

    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(sext_from<uint8_t>(iregs.load_reg(rd) & 0xFFUL));
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LW()
{
    LWU();

    auto rd = curr_instr.get_fields().rd;

    auto res
        = static_cast<int64_t>(sext_from<uint32_t>(iregs.load_reg(rd) & 0xFFFFFFFFUL));
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LBU()
{
    auto base_reg = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    uint64_t mem_addr
        = static_cast<uint64_t>(static_cast<int64_t>(iregs.load_reg(base_reg)) + imm);

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

    auto res = static_cast<int64_t>(sext_from<uint16_t>(iregs.load_reg(rd) & 0xFFFFUL));
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LHU()
{
    auto base_reg = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    uint64_t mem_addr
        = static_cast<uint64_t>(static_cast<int64_t>(iregs.load_reg(base_reg)) + imm);

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

    uint64_t mem_addr
        = static_cast<uint64_t>(static_cast<int64_t>(iregs.load_reg(base_reg)) + imm);

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

    uint64_t mem_addr
        = static_cast<uint64_t>(static_cast<int64_t>(iregs.load_reg(base_reg)) + imm);

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

    auto rd = curr_instr.get_fields().rd;
    auto rs1 = curr_instr.get_fields().rs1;

    auto res = (iregs.load_reg(rs1) < imm) ? 1 : 0;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLTIU()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);

    auto rd = curr_instr.get_fields().rd;
    auto rs1 = curr_instr.get_fields().rs1;

    auto res = (static_cast<uint64_t>(iregs.load_reg(rs1)) < static_cast<uint64_t>(imm))
        ? 1
        : 0;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::XORI()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    int64_t imm = static_cast<int64_t>(imm_32);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) ^ imm;
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
    uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
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
    uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
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
    uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x3F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(iregs.load_reg(rs1) >> shamt);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLLIW()
{
    uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    uint32_t op = static_cast<uint32_t>(iregs.load_reg(rs1));
    op <<= shamt;

    auto res = static_cast<int64_t>(static_cast<int32_t>(op));
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRLIW()
{
    uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    uint32_t op = static_cast<uint32_t>(iregs.load_reg(rs1));
    op >>= shamt;

    auto res = static_cast<int64_t>(static_cast<int32_t>(op));
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRAIW()
{
    uint8_t shamt = static_cast<uint8_t>(curr_instr.get_fields().imm & 0x1F);
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t op = static_cast<int32_t>(iregs.load_reg(rs1));
    op >>= shamt;

    auto res = static_cast<int64_t>(op);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FENCE() { return ReturnException::NormalExecutionReturn; }

ReturnException VEmu::FENCEI() { return ReturnException::NormalExecutionReturn; }

#ifdef FUZZ_ENV
ReturnException VEmu::ECALL()
{
    auto syscall_number = iregs.load_reg(REG_A7);
    if (syscall_number == SYSCALL_NR_BRK) {
        uint64_t addr = iregs.load_reg(REG_A0);
        int64_t increment
            = addr == 0 ? 0 : (int64_t)addr - (int64_t)bus.get_mmu()->cur_alloc_ptr();
        if (increment < 0) {
            iregs.store_reg(REG_A0, ~0);
        } else {
            auto ret_base = bus.get_mmu()->allocate(increment);
            iregs.store_reg(REG_A0, ret_base + increment);
        }
    } else if (syscall_number == SYSCALL_NR_WRITE) {
        int64_t fd = iregs.load_reg(REG_A0);
        uint64_t buf = iregs.load_reg(REG_A1);
        size_t count = iregs.load_reg(REG_A2);
        auto str = bus.get_mmu()->read_to(buf, count).first;
        std::string s(str.begin(), str.end());
        if (fd == 1 || fd == 2) {
            std::cout << s << std::flush;
            iregs.store_reg(REG_A0, count);
        }
    } else if (syscall_number == SYSCALL_NR_OPEN) {
        auto pathname = iregs.load_reg(REG_A0);
        auto flags = iregs.load_reg(REG_A1);
        auto p = bus.get_mmu()->_read_null_terminated_string(pathname);
        int fd = open(p.c_str(), (int)flags);
        assert(fd != -1);
        if (fd == -1) {
            iregs.store_reg(REG_A0, -1);
            return ReturnException::NormalExecutionReturn;
        }

        uint64_t file_size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        char* buffer = (char*)malloc(file_size + 1);
        assert(buffer);
        int64_t r = read(fd, buffer, file_size);
        (void)r;
        buffer[file_size] = '\0';
        char* fname = (char*)malloc(p.size() + 1);
        strcpy(fname, p.c_str());
        file_table.push_back(FileHandle { buffer, (const char*)fname, fd, file_size, 0,
                                          FileType::DiskFile });
        iregs.store_reg(REG_A0, fd);
    } else if (syscall_number == SYSCALL_NR_EXIT) {
        exit((int)iregs.load_reg(REG_A0));
    } else if (syscall_number == SYSCALL_NR_FSTAT) {
        int64_t fd = iregs.load_reg(REG_A0);
        uint64_t statbuf = iregs.load_reg(REG_A1);
        bool exists = false;
        for (auto fh : file_table) {
            if (fh.fd == (int)fd) {
                exists = true;
                break;
            }
        }
        assert(exists);

        struct riscv_stat {
            uint64_t _st_dev;
            uint64_t _st_ino;
            uint32_t _st_mode;
            uint32_t _st_nlink;
            uint32_t _st_uid;
            uint32_t _st_gid;
            uint64_t _st_rdev;
            uint64_t _ppad1;
            int64_t _st_size;
            int32_t _st_blksize;
            int32_t _ppad2;
            uint64_t _st_blocks;
            uint64_t _st_atime;
            uint64_t _st_atimensec;
            uint64_t _st_mtime;
            uint64_t _st_mtimensec;
            uint64_t _st_ctime;
            uint64_t _st_ctimensec;
            int __glibc_reserved[2];
        } st;

        st._st_dev = (fd == 1) ? 0x18 : 0x802;
        st._st_ino = (fd == 1) ? 0x3 : 0x22068c;
        st._st_mode = (fd == 1) ? 0x2190 : 0x81fd;
        st._st_nlink = 0x1;
        st._st_uid = 0x3e8;
        st._st_gid = (fd == 1) ? 0x5 : 0x3e8;
        st._st_rdev = (fd == 1) ? 0x8800 : 0x0;
        st._st_size = (fd == 1) ? 0x0 : file_table.back().len;
        st._st_blksize = (fd == 1) ? 0x400 : 0x1000;
        st._st_blocks = (fd == 1) ? 0x0 : file_table.back().len / st._st_blksize;
        st._st_atime = 0x6440824f;
        st._st_mtime = 0x6440824a;
        st._st_ctime = 0x6440824a;

        auto ptr = reinterpret_cast<uint8_t*>(&st);
        std::vector<uint8_t> bytes(ptr, ptr + sizeof(riscv_stat));
        bus.get_mmu()->write_from(bytes, statbuf);
        iregs.store_reg(REG_A0, 0);
    } else if (syscall_number == SYSCALL_NR_LSEEK) {
        int64_t file = iregs.load_reg(REG_A0);
        int64_t offset = iregs.load_reg(REG_A1);
        int64_t whence = iregs.load_reg(REG_A2);
        for (auto& fh : file_table) {
            if (fh.fd == file) {
                if (whence == SEEK_SET)
                    fh.idx = offset;
                else if (whence == SEEK_CUR)
                    fh.idx += offset;
                else if (whence == SEEK_END)
                    fh.idx = fh.len + offset;
                else
                    assert(false);
                iregs.store_reg(REG_A0, fh.idx);
                return ReturnException::NormalExecutionReturn;
            }
        }
        iregs.store_reg(REG_A0, -1);
    } else if (syscall_number == SYSCALL_NR_READ) {
        int64_t fd = iregs.load_reg(REG_A0);
        uint64_t buf = iregs.load_reg(REG_A1);
        uint64_t count = iregs.load_reg(REG_A2);
        for (auto& fh : file_table) {
            if (fh.fd == fd) {
                const uint8_t* data_start = (const uint8_t*)fh.data;
                if (fh.idx + count >= fh.len)
                    count = fh.len - fh.idx;
                if (count == 0)
                    return ReturnException::NormalExecutionReturn;
                std::vector<uint8_t> data(data_start + fh.idx,
                                          data_start + fh.idx + count);
                fh.idx += count;
                bus.get_mmu()->write_from(data, buf);
                iregs.store_reg(REG_A0, count);
                return ReturnException::NormalExecutionReturn;
            }
        }
        assert(false);
    } else {
        std::cout << "Unsupported syscall: " << std::dec << syscall_number << ", pc: 0x"
                  << std::hex << pc << '\n';
        exit(EXIT_FAILURE);
    }

    return ReturnException::NormalExecutionReturn;
}
#elif defined(TEST_ENV)
ReturnException VEmu::ECALL()
{
    /* a0 is loaded with zero on ECALL in `pass`. */
    if (iregs.load_reg(REG_A0) == 0) {
        std::cout << "Passed\n";
    } else {
        std::cout << "Failed test: " << bin_file_name << '\n';
        std::cout << ("Failed on testcase #" + std::to_string(iregs.load_reg(REG_GP) >> 1)
                      + '\n');
        dump_regs();
    }
    test_flag_done = true;
    return ReturnException::NormalExecutionReturn;
}
#else
ReturnException VEmu::ECALL()
{
    switch (mode) {
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
#endif

ReturnException VEmu::EBREAK() { return ReturnException::InstructionAddressBreakpoint; }

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

    if (static_cast<uint64_t>(iregs.load_reg(rs1))
        < static_cast<uint64_t>(iregs.load_reg(rs2))) {
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

    if (static_cast<uint64_t>(iregs.load_reg(rs1))
        >= static_cast<uint64_t>(iregs.load_reg(rs2))) {
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

    auto res = static_cast<int64_t>(static_cast<uint64_t>(iregs.load_reg(rs1)) << shamt);

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SLLW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x1F;

    uint32_t op = static_cast<uint32_t>(iregs.load_reg(rs1));

    op <<= shamt;

    // FIXME: This works only if the 32-bit value is sign-extended
    // and then put to the register.
    // (I'll have to zero extend the signed 32-bit value if not so.)
    auto res = static_cast<int64_t>(static_cast<int32_t>(op));

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

    auto res = (static_cast<uint64_t>(iregs.load_reg(rs1))
                < static_cast<uint64_t>(iregs.load_reg(rs2)))
        ? 1
        : 0;
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::XOR()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = iregs.load_reg(rs1) ^ iregs.load_reg(rs2);
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

    auto res = static_cast<int32_t>(iregs.load_reg(rs1) & 0xFFFFFFFF)
        * static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::MULH()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(
        ((__int128)iregs.load_reg(rs1) * (__int128)iregs.load_reg(rs2)) >> 64);

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
        (static_cast<unsigned __int128>(urs1) * static_cast<unsigned __int128>(urs2))
        >> 64);

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

    unsigned __int128 rs1_val
        = static_cast<unsigned __int128>(static_cast<__int128>(irs1));
    unsigned __int128 rs2_val = static_cast<unsigned __int128>(urs2);

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
        csrs[FFLAGS] |= (0x8);
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
        csrs[FFLAGS] |= (0x8);
        res = 0xFFFFFFFF'FFFFFFFF;
        iregs.store_reg(rd, res);
    } else {
        res = static_cast<int64_t>(static_cast<uint64_t>(iregs.load_reg(rs1))
                                   / static_cast<uint64_t>(iregs.load_reg(rs2)));
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
        csrs[FFLAGS] |= (0x8);
        res = 0xFFFFFFFF'FFFFFFFF;
    } else if (rs2_32 == -1 && rs1_32 == INT32_MIN) {
        res = static_cast<int64_t>(static_cast<int32_t>(rs1_32));
    } else {
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
        csrs[FFLAGS] |= (0x8);
        res = 0xFFFFFFFF'FFFFFFFF;
    } else {
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
        res = static_cast<int64_t>(static_cast<uint64_t>(iregs.load_reg(rs1))
                                   % static_cast<uint64_t>(iregs.load_reg(rs2)));
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
        res = static_cast<int32_t>(iregs.load_reg(rs1))
            % static_cast<int32_t>(iregs.load_reg(rs2));
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

    auto res = static_cast<int64_t>(static_cast<uint64_t>(iregs.load_reg(rs1)) >> shamt);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRLW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x1F;

    uint32_t op = static_cast<uint32_t>(iregs.load_reg(rs1));

    op >>= shamt;

    auto res = static_cast<int64_t>(static_cast<int32_t>(op));

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRA()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x3F;

    auto res = static_cast<int64_t>(iregs.load_reg(rs1) >> shamt);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::SRAW()
{
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    uint8_t shamt = static_cast<uint64_t>(iregs.load_reg(rs2)) & 0x1F;

    int32_t op = static_cast<int32_t>(iregs.load_reg(rs1));

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
    auto rs1_val = iregs.load_reg(rs1);
    iregs.store_reg(rd, res);

    this->pc = static_cast<uint64_t>(rs1_val + imm);
    this->pc &= ~(0x1);
    this->pc -= 4;

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::LUI()
{
    int32_t imm_32 = static_cast<int32_t>(curr_instr.get_fields().imm);
    auto rd = curr_instr.get_fields().rd;

    auto res = static_cast<int64_t>(imm_32);
    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::AUIPC()
{
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

    auto store_ret = store(addr, iregs.load_reg(rs2), 32);
    iregs.store_reg(rd, 0);
    return store_ret;
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

    auto store_ret = store(addr, iregs.load_reg(rs2), 64);
    iregs.store_reg(rd, 0);
    return store_ret;
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

    auto store_ret
        = store(addr, static_cast<int32_t>(iregs.load_reg(rs2) & 0xFFFFFFFF), 32);
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

    uint64_t res = tmp < rs2_val ? static_cast<uint64_t>(static_cast<uint32_t>(tmp))
                                 : static_cast<uint64_t>(static_cast<uint32_t>(rs2_val));

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

    uint64_t res = tmp > rs2_val ? static_cast<uint64_t>(static_cast<uint32_t>(tmp))
                                 : static_cast<uint64_t>(static_cast<uint32_t>(rs2_val));

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

    uint64_t res
        = tmp < rs2_val ? static_cast<uint64_t>(tmp) : static_cast<uint64_t>(rs2_val);

    auto store_ret = store(addr, res, 32);
    if (store_ret != ReturnException::NormalExecutionReturn) {
        return store_ret;
    }

    iregs.store_reg(rd, sext_from<uint32_t>(tmp));

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

    iregs.store_reg(rd, sext_from<uint32_t>(tmp));

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
    int64_t rs2_val = iregs.load_reg(rs2);

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
    uint64_t rs2_val = iregs.load_reg(rs2);

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
    uint64_t rs2_val = iregs.load_reg(rs2);

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
    uint64_t rs2_val = iregs.load_reg(rs2);

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
    int64_t rs2_val = iregs.load_reg(rs2);

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
    uint64_t rs2_val = iregs.load_reg(rs2);

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
    uint64_t rs2_val = iregs.load_reg(rs2);

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

    if (mb == 0x00)
        mode = Mode::User;
    else if (mb == 0x01)
        mode = Mode::Supervisor;
    else if (mb == 0x11)
        mode = Mode::Machine;
    else
        assert(false);

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

    if (mb == 0x0)
        mode = Mode::User;
    else
        mode = Mode::Supervisor;

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
    std::cout << std::setw(8) << std::setfill('0') << std::hex << hex_instr
              << " PC: " << pc << '\n';

    std::cout.flags(ft);
    return ReturnException::IllegalInstruction;
}

Interrupt VEmu::check_pending_interrupt()
{
    if (mode == Mode::Machine) {
        if (((load_csr(MSTATUS) >> MSTATUS_MIE_POS) & 1) == 0) {
            return Interrupt::NoInterrupt;
        }
    } else if (mode == Mode::Supervisor) {
        if (((load_csr(SSTATUS) >> SSTATUS_SIE_POS) & 1) == 0) {
            return Interrupt::NoInterrupt;
        }
    }

    uint32_t irq = bus.uart_is_interrupting() ? UART_IRQ : 0;

    if (irq == UART_IRQ) {
        bus.store(PLIC_SCLAIM, 32, irq);
        store_csr(MIP, load_csr(MIP) | (1U << MIP_SEIP_POS));
    }

    const auto is_pending = [this](uint64_t interrupt_pos) -> bool {
        /* Check if the interrupt is enabled AND pending */
        uint64_t pending = load_csr(MIE) & load_csr(MIP);
        return pending & (1U << interrupt_pos);
    };

    if (is_pending(MIP_MEIP_POS)) {
        store_csr(MIP, load_csr(MIP) & ~(1U << MIP_MEIP_POS));
        return Interrupt::MachineExternalInterrupt;
    } else if (is_pending(MIP_MSIP_POS)) {
        store_csr(MIP, load_csr(MIP) & ~(1U << MIP_MSIP_POS));
        return Interrupt::MachineSoftwareInterrupt;
    } else if (is_pending(MIP_MTIP_POS)) {
        store_csr(MIP, load_csr(MIP) & ~(1U << MIP_MTIP_POS));
        return Interrupt::MachineTimerInterrupt;
    } else if (is_pending(MIP_SEIP_POS)) {
        store_csr(MIP, load_csr(MIP) & ~(1U << MIP_SEIP_POS));
        return Interrupt::SupervisorExternalInterrupt;
    } else if (is_pending(MIP_SSIP_POS)) {
        store_csr(MIP, load_csr(MIP) & ~(1U << MIP_SSIP_POS));
        return Interrupt::SupervisorSoftwareInterrupt;
    } else if (is_pending(MIP_STIP_POS)) {
        store_csr(MIP, load_csr(MIP) & ~(1U << MIP_STIP_POS));
        return Interrupt::SupervisorTimerInterrupt;
    }

    return Interrupt::NoInterrupt;
}

#ifdef FUZZ_ENV
void VEmu::trap(ReturnException e)
{
    (void)e;
    // std::cout << "Unexpected return exception: " << stringify_exception(e) << " @ pc:
    // 0x"
    //           << std::hex << pc << '\n';
}
#else
void VEmu::trap(ReturnException e)
{
    uint64_t exception_pc = pc;
    uint8_t cause = static_cast<uint8_t>(e);
    Mode prev_mode = mode;

    /* Exceptions are handled usually in the Machine mode
       privilege level. However, some exceptions can be
       marked to be done at a lower privilege level. This marking
       is saved in the MEDELEG control and status register. When
       the current mode is user-mode, the exception is handled in
       supervisor mode.
    */
    bool do_deleg = (load_csr(MEDELEG) >> cause) & 1;

    if ((prev_mode == Mode::User || prev_mode == Mode::Supervisor) && do_deleg) {
        mode = Mode::Supervisor;

        /* Set PC to the exception handler base address. */
        pc = load_csr(STVEC) & (~(0b1U));
        pc -= 4;

        /* SEPC contains the address of the instruciton that caused the
         * exception */
        store_csr(SEPC, exception_pc & (~1U));

        /* When a trap is taken in Supervisor mode, SCAUSE is written
         * with a code that indicates the cause of the trap.
         * */
        store_csr(SCAUSE, cause);

        /* Exception specific data that is used to assist the software.
         * Currently not utilized. */
        store_csr(STVAL, 0);

        /* SPIE is set to the Current SIE, and SIE is zero'd out. */
        uint8_t sie = (load_csr(SSTATUS) >> SSTATUS_SIE_POS) & 1U;
        if (sie) {
            store_csr(SSTATUS, load_csr(SSTATUS) | (1U << SSTATUS_SPIE_POS));
        } else {
            store_csr(SSTATUS, load_csr(SSTATUS) & (~(1U << SSTATUS_SPIE_POS)));
        }

        /* Set SIE to zero */
        store_csr(SSTATUS, load_csr(SSTATUS) & (~(1U << SSTATUS_SIE_POS)));

        /* SPP sets the previous mode, if user mode, it's set to zero. */
        if (prev_mode == Mode::User) {
            store_csr(SSTATUS, load_csr(SSTATUS) & (~(1U << SSTATUS_SPP_POS)));
        } else {
            store_csr(SSTATUS, load_csr(SSTATUS) | (1U << SSTATUS_SPP_POS));
        }
    } else {
        mode = Mode::Machine;

        /* PC is set to the respective exception handler. */
        pc = load_csr(MTVEC) & (~(0b1U));
        pc -= 4;

        /* MEPC is is set to the exception-raising PC. */
        store_csr(MEPC, exception_pc & (~0b1U));

        /* MCAUSE is set to the cause of the exception. */
        store_csr(MCAUSE, cause);

        /* Assisting information, currently not utilized. */
        store_csr(MTVAL, 0);

        /* MPIE is set to MIE, indicating previous interrupt enable. */
        uint8_t mie = (load_csr(MSTATUS) >> MSTATUS_MIE_POS) & 1U;
        if (mie == 1) {
            store_csr(MSTATUS, load_csr(MSTATUS) | (1U << MSTATUS_MPIE_POS));
        } else {
            store_csr(MSTATUS, load_csr(MSTATUS) & (~(1U << MSTATUS_MPIE_POS)));
        }

        /* Set MIE to zero. */
        store_csr(MSTATUS, load_csr(MSTATUS) & (~(1U << MSTATUS_MIE_POS)));

        /* MPP is a two-bit field. 00 -> User, 01 -> Supervisor, 11 -> Machine
         */
        switch (prev_mode) {
        case Mode::User:
            store_csr(MSTATUS, load_csr(MSTATUS) & (~(0b11U << 11U)));
            break;
        case Mode::Supervisor:
            store_csr(MSTATUS, load_csr(MSTATUS) & (~(0b1U << 12U)));
            store_csr(MSTATUS, load_csr(MSTATUS) | (0b1U << 11U));
            break;
        case Mode::Machine:
            store_csr(MSTATUS, load_csr(MSTATUS) | (0b11U << 11U));
            break;
        default:
            std::cout << "Unsupported(?) previous privilege mode.\n";
            exit(EXIT_FAILURE);
        }
    }
}
#endif

bool VEmu::is_fatal(ReturnException e)
{
    return e == ReturnException::LoadAccessFault
        || e == ReturnException::InstructionAccessFault
        || e == ReturnException::InstructionAddressMisaligned
        || e == ReturnException::StoreAMOAddressMisaligned
        || e == ReturnException::StoreAMOAccessFault;
}

void VEmu::exit_fatally(ReturnException e)
{
    std::cout << "Exit on exception: " << stringify_exception(e);
    std::cout << '\n' << std::hex << pc << '\n';

    dump_regs();

    exit(EXIT_FAILURE);
}

std::string VEmu::stringify_exception(ReturnException e)
{
    switch (e) {
    case ReturnException::InstructionAddressMisaligned:
        return "InstructionAddressMisaligned";
    case ReturnException::InstructionAccessFault:
        return "InstructionAccessFault";
    case ReturnException::IllegalInstruction:
        return "IllegalInstruction";
    case ReturnException::InstructionAddressBreakpoint:
        return "InstructionAddressBreakpoint";
    case ReturnException::LoadAddressMisaligned:
        return "LoadAddressMisaligned";
    case ReturnException::LoadAccessFault:
        return "LoadAccessFault";
    case ReturnException::StoreAMOAddressMisaligned:
        return "StoreAMOAddressMisaligned";
    case ReturnException::StoreAMOAccessFault:
        return "StoreAMOAccessFault";
    case ReturnException::EnvironmentCallFromUserMode:
        return "EnvironmentCallFromUserMode";
    case ReturnException::EnvironmentCallFromSupervisorMode:
        return "EnvironmentCallFromSupervisorMode";
    case ReturnException::EnvironmentCallFromMachineMode:
        return "EnvironmentCallFromMachineMode";
    case ReturnException::InstructionPageFault:
        return "InstructionPageFault";
    case ReturnException::LoadPageFault:
        return "LoadPageFault";
    case ReturnException::StoreAMOPageFault:
        return "StoreAMOPageFault";
    case ReturnException::NormalExecutionReturn:
        return "NormalExecutionReturn";
    case ReturnException::UninitializedMemoryAccess:
        return "UninitializedMemoryAccess";
    case ReturnException::ReadMemoryWithNoPermission:
        return "ReadMemoryWithNoPermission";
    default:
        std::cout << "stringify_exception: Unsupported exception.\n";
        exit(EXIT_FAILURE);
    }
}

/* This is a mess since this and trap() are essentially identical, obviously
 * there's a better solution, but for now this will suffice...
 */
void VEmu::take_interrupt(Interrupt i)
{
    uint64_t current_pc = pc;
    uint64_t cause = (1ULL << 63ULL) | static_cast<uint64_t>(i);
    Mode prev_mode = mode;

    /* Exceptions are handled usually in the Machine mode
       privilege level. However, some exceptions can be
       marked to be done at a lower privilege level. This marking
       is saved in the MEDELEG control and status register. When
       the current mode is user-mode, the exception is handled in
       supervisor mode.
    */
    bool do_deleg = (load_csr(MEDELEG) >> cause) & 1U;

    if ((prev_mode == Mode::User || prev_mode == Mode::Supervisor) && do_deleg) {
        mode = Mode::Supervisor;

        /* Set PC to the exception handler base address. */
        uint64_t vector = (load_csr(STVEC) & 1U) ? 4 * cause : 0;
        pc = (load_csr(STVEC) & ~(1U)) + vector;
        pc -= 4;

        /* SEPC contains the address of the instruciton that caused the
         * exception */
        store_csr(SEPC, current_pc & (~1U));

        /* When a trap is taken in Supervisor mode, SCAUSE is written
         * with a code that indicates the cause of the trap.
         * */
        store_csr(SCAUSE, cause);

        /* Exception specific data that is used to assist the software.
         * Currently not utilized. */
        store_csr(STVAL, 0);

        /* SPIE is set to the Current SIE, and SIE is zero'd out. */
        uint8_t sie = (load_csr(SSTATUS) >> SSTATUS_SIE_POS) & 1U;
        if (sie) {
            store_csr(SSTATUS, load_csr(SSTATUS) | (1U << SSTATUS_SPIE_POS));
        } else {
            store_csr(SSTATUS, load_csr(SSTATUS) & (~(1U << SSTATUS_SPIE_POS)));
        }

        /* Set SIE to zero */
        store_csr(SSTATUS, load_csr(SSTATUS) & (~(1U << SSTATUS_SIE_POS)));

        /* SPP sets the previous mode, if user mode, it's set to zero. */
        if (prev_mode == Mode::User) {
            store_csr(SSTATUS, load_csr(SSTATUS) & (~(1U << SSTATUS_SPP_POS)));
        } else {
            store_csr(SSTATUS, load_csr(SSTATUS) | (1U << SSTATUS_SPP_POS));
        }
    } else {
        mode = Mode::Machine;

        /* PC is set to the respective exception handler. */
        uint64_t vector = (load_csr(MTVEC) & 1U) ? cause * 4 : 0;
        pc = (load_csr(MTVEC) & (~(0b1U))) + vector;
        pc -= 4;

        /* MEPC is is set to the exception-raising PC. */
        store_csr(MEPC, current_pc & (~0b1U));

        /* MCAUSE is set to the cause of the exception. */
        store_csr(MCAUSE, cause);

        /* Assisting information, currently not utilized. */
        store_csr(MTVAL, 0);

        /* MPIE is set to MIE, indicating previous interrupt enable. */
        uint8_t mie = (load_csr(MSTATUS) >> MSTATUS_MIE_POS) & 1U;
        if (mie == 1) {
            store_csr(MSTATUS, load_csr(MSTATUS) | (1U << MSTATUS_MPIE_POS));
        } else {
            store_csr(MSTATUS, load_csr(MSTATUS) & (~(1U << MSTATUS_MPIE_POS)));
        }

        /* Set MIE to zero. */
        store_csr(MSTATUS, load_csr(MSTATUS) & (~(1U << MSTATUS_MIE_POS)));

        /* MPP is a two-bit field. 00 -> User, 01 -> Supervisor, 11 -> Machine
         */
        switch (prev_mode) {
        case Mode::User:
            store_csr(MSTATUS, load_csr(MSTATUS) & (~(0b11U << 11U)));
            break;
        case Mode::Supervisor:
            store_csr(MSTATUS, load_csr(MSTATUS) & (~(0b1U << 12U)));
            store_csr(MSTATUS, load_csr(MSTATUS) | (0b1U << 11U));
            break;
        case Mode::Machine:
            store_csr(MSTATUS, load_csr(MSTATUS) | (0b11U << 11U));
            break;
        default:
            std::cout << "Unsupported(?) previous privilege mode.\n";
            exit(EXIT_FAILURE);
        }
    }
}

ReturnException VEmu::FLW()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#endif
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;
    auto offs = curr_instr.get_fields().imm;

    auto addr = iregs.load_reg(rs1) + offs;

    auto load_res = load(addr, 32);
    if (load_res.second != ReturnException::NormalExecutionReturn)
        return load_res.second;

    union {
        uint32_t i;
        float s;
    } a {};
    a.i = static_cast<uint32_t>(load_res.first);

    fregs.store_reg(rd, static_cast<double>(a.s));

    return ReturnException::NormalExecutionReturn;
}

ReturnException VEmu::FSW()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#endif
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto offs = curr_instr.get_fields().imm;

    auto addr = iregs.load_reg(rs1) + offs;

    union {
        uint32_t i;
        float s;
    } a {};

    a.s = static_cast<float>(fregs.load_reg(rs2));
    auto store_res = store(addr, static_cast<uint64_t>(a.i), 32);

    return store_res;
}

ReturnException VEmu::FMADDS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
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
#endif
}

ReturnException VEmu::FMSUBS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
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
#endif
}

ReturnException VEmu::FNMSUBS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
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
#endif
}

ReturnException VEmu::FNMADDS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
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
#endif
}

#ifdef SUPPORT_SOFTFLOAT
void VEmu::update_float_flags()
{
    if (softfloat_exceptionFlags & softfloat_flag_inexact) {
        csrs[FFLAGS] |= (0x1);
    } else {
        csrs[FFLAGS] &= ~(0x1U);
    }

    if (softfloat_exceptionFlags & softfloat_flag_underflow) {
        csrs[FFLAGS] |= (0x2);
    } else {
        csrs[FFLAGS] &= ~(0x2U);
    }

    if (softfloat_exceptionFlags & softfloat_flag_overflow) {
        csrs[FFLAGS] |= (0x4);
    } else {
        csrs[FFLAGS] &= ~(0x4U);
    }

    if (softfloat_exceptionFlags & softfloat_flag_invalid) {
        csrs[FFLAGS] |= (0x10);
    } else {
        csrs[FFLAGS] &= ~(0x10U);
    }
}

void VEmu::reset_float_flags() { softfloat_exceptionFlags = 0; }
#endif

ReturnException VEmu::FADDS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    reset_float_flags();

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    uint32_t op1_bits;
    memcpy(&op1_bits, &op1, sizeof(uint32_t));

    uint32_t op2_bits;
    memcpy(&op2_bits, &op2, sizeof(uint32_t));

    float32_t res_bits = f32_add({ op1_bits }, { op2_bits });

    float res;
    memcpy(&res, &res_bits.v, sizeof(uint32_t));

    fregs.store_reg(rd, static_cast<double>(res));
    update_float_flags();

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FSUBS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    reset_float_flags();

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    uint32_t op1_bits;
    memcpy(&op1_bits, &op1, sizeof(uint32_t));

    uint32_t op2_bits;
    memcpy(&op2_bits, &op2, sizeof(uint32_t));

    float32_t res_bits = f32_sub({ op1_bits }, { op2_bits });

    float res;
    memcpy(&res, &res_bits.v, sizeof(uint32_t));

    if (std::isinf(op1) && std::isinf(op2) && !std::signbit(op1) && !std::signbit(op2)) {
        res = std::fabs(res);
    }

    fregs.store_reg(rd, static_cast<double>(res));
    update_float_flags();

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FMULS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    reset_float_flags();

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    float32_t op1_bits;
    memcpy(&op1_bits, &op1, sizeof(uint32_t));

    float32_t op2_bits;
    memcpy(&op2_bits, &op2, sizeof(uint32_t));

    float32_t res_bits = f32_mul(op1_bits, op2_bits);

    float res;
    memcpy(&res, &res_bits.v, sizeof(uint32_t));

    fregs.store_reg(rd, static_cast<double>(res));

    update_float_flags();
    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FDIVS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    reset_float_flags();

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float32_t op1_bits;
    memcpy(&op1_bits, &op1, sizeof(uint32_t));

    float op2 = static_cast<float>(fregs.load_reg(rs2));
    float32_t op2_bits;
    memcpy(&op2_bits, &op2, sizeof(uint32_t));

    float32_t res_bits = f32_div(op1_bits, op2_bits);

    float res;
    memcpy(&res, &res_bits.v, sizeof(uint32_t));

    fregs.store_reg(rd, static_cast<double>(res));

    update_float_flags();

    if (std::fabs(op2) == 0.0f) {
        csrs[FFLAGS] |= 0x8U;
    } else {
        csrs[FFLAGS] &= ~(0x8U);
    }

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FSQRTS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op = static_cast<float>(fregs.load_reg(rs1));
    float32_t op_bits;
    memcpy(&op_bits, &op, sizeof(uint32_t));

    float32_t res_bits = f32_sqrt(op_bits);
    float res;
    memcpy(&res, &res_bits.v, sizeof(uint32_t));

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FSGNJS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    float res = copysignf(op1, op2);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FSGNJNS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    float res = copysignf(op1, -op2);

    fregs.store_reg(rd, static_cast<double>(res));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FSGNJXS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
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
#endif
}

bool VEmu::is_negative_zero(double d)
{
    union {
        int64_t a;
        double d;
    } u;

    u.d = d;

    return d == 0.0 && u.a != 0;
}

bool VEmu::is_positive_zero(double d)
{
    union {
        int64_t a;
        double d;
    } u;

    u.d = d;

    return d == 0.0 && u.a == 0;
}

ReturnException VEmu::FMINS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
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
#endif
}

ReturnException VEmu::FMAXS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
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
#endif
}

ReturnException VEmu::FCVTWS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op = static_cast<float>(fregs.load_reg(rs1));
    op = std::round(op);

    iregs.store_reg(rd, static_cast<int64_t>(static_cast<int32_t>(op)));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FCVTWUS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op = static_cast<float>(fregs.load_reg(rs1));
    op = std::round(op);

    iregs.store_reg(rd, static_cast<uint64_t>((op)));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FMVXW()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    union {
        int32_t i;
        float s;
    } a {};

    a.s = static_cast<float>(fregs.load_reg(rs1));

    iregs.store_reg(rd, static_cast<int64_t>(a.i));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FEQS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    iregs.store_reg(rd, (op1 == op2) ? 1 : 0);

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FLTS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    iregs.store_reg(rd, (op1 < op2) ? 1 : 0);

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FLES()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rs2 = curr_instr.get_fields().rs2;
    auto rd = curr_instr.get_fields().rd;

    float op1 = static_cast<float>(fregs.load_reg(rs1));
    float op2 = static_cast<float>(fregs.load_reg(rs2));

    iregs.store_reg(rd, (op1 <= op2) ? 1 : 0);

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FCVTSW()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t op = static_cast<int32_t>(iregs.load_reg(rs1));

    fregs.store_reg(rd, static_cast<double>(static_cast<float>(op)));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FCVTSWU()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    uint32_t op = static_cast<uint32_t>(iregs.load_reg(rs1));

    fregs.store_reg(rd, static_cast<double>(static_cast<float>(op)));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FMVWX()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    union {
        uint32_t i;
        float s;
    } a {};

    a.i = static_cast<uint32_t>(iregs.load_reg(rs1));

    fregs.store_reg(rd, static_cast<double>(a.s));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FCVTLS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op = static_cast<float>(fregs.load_reg(rs1));
    op = std::round(op);

    iregs.store_reg(rd, static_cast<int64_t>(op));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FCVTLUS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op = static_cast<float>(fregs.load_reg(rs1));
    op = std::round(op);

    iregs.store_reg(rd, static_cast<int64_t>(static_cast<uint64_t>(op)));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FCVTSL()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    int32_t op = static_cast<int32_t>(iregs.load_reg(rs1));
    fregs.store_reg(rd, static_cast<double>(static_cast<float>(op)));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FCVTSLU()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    uint32_t op = static_cast<uint32_t>(iregs.load_reg(rs1));

    fregs.store_reg(rd, static_cast<double>(static_cast<float>(op)));

    return ReturnException::NormalExecutionReturn;
#endif
}

ReturnException VEmu::FCLASSS()
{
#ifndef SUPPORT_SOFTFLOAT
    return ReturnException::IllegalInstruction;
#else
    auto rs1 = curr_instr.get_fields().rs1;
    auto rd = curr_instr.get_fields().rd;

    float op = static_cast<float>(fregs.load_reg(rs1));
    int fpc = std::fpclassify(op);
    bool sign = std::signbit(op);
    int64_t res = 0;

    switch (fpc) {
    case FP_INFINITE:
        if (sign)
            res = 0;
        else
            res = 7;
        break;
    case FP_NORMAL:
        if (sign)
            res = 1;
        else
            res = 6;
        break;
    case FP_SUBNORMAL:
        if (sign)
            res = 2;
        else
            res = 5;
        break;
    case FP_ZERO:
        if (sign)
            res = 3;
        else
            res = 4;
        break;
    case FP_NAN:
        res = 9;
        break;
    default:
        break;
    }

    iregs.store_reg(rd, res);

    return ReturnException::NormalExecutionReturn;
#endif
}
