#include <InstructionDecoder.h>

InstructionDecoder::InstructionDecoder()
{
    init_fixed_instrs();
}

void InstructionDecoder::init_fixed_instrs()
{
    instr_cache[0x30200073] = Instruction(
        Instruction::Type::R,
        IName::MRET,
        Fields {
            .OPCode = 0b1110011,
            .rd = 0b00000,
            .funct3 = 0b000,
            .rs1 = 0b00000,
            .rs2 = 0b00010,
            .funct7 = 0b0011000,
            .shamt32 = 0,
            .shamt64 = 0,
            .funct6 = 0,
            .imm = 0,
            .funct2 = 0,
            .rs3 = 0,
        },
        "MRET"
    );

    instr_cache[0x10200073] = Instruction {
        Instruction::Type::R,
        IName::SRET,
        Fields {
            .OPCode = 0b1110011,
            .rd = 0b00000,
            .funct3 = 0b000,
            .rs1 = 0b00000,
            .rs2 = 0b00010,
            .funct7 = 0b0001000,
            .shamt32 = 0,
            .shamt64 = 0,
            .funct6 = 0,
            .imm = 0,
            .funct2 = 0,
            .rs3 = 0,
        },
        "SRET"
    };
}

InstructionDecoder &InstructionDecoder::the()
{
    static InstructionDecoder inst;
    return inst;
}

Instruction InstructionDecoder::decode(const uint32_t inst)
{
    if (instr_cache.find(inst) != instr_cache.end())
        return instr_cache.at(inst);

    auto t = instr_type(inst);
    switch (t) {
        case Instruction::Type::R:
            return instr_cache[inst] = decode_r(inst);
        case Instruction::Type::R4:
            return instr_cache[inst] = decode_r4(inst);
        case Instruction::Type::I:
            return instr_cache[inst] = decode_i(inst);
        case Instruction::Type::S:
            return instr_cache[inst] = decode_s(inst);
        case Instruction::Type::B:
            return instr_cache[inst] = decode_b(inst);
        case Instruction::Type::U:
            return instr_cache[inst] = decode_u(inst);
        case Instruction::Type::J:
            return instr_cache[inst] = decode_j(inst);
        case Instruction::Type::WRONG: default:
            return Instruction(t, IName::XXX, Fields{}, "XXX");
    }
}

const std::map<IName, std::string> InstructionDecoder::inst_string_names = {
        {IName::LB,             "LB"},
        {IName::LH,             "LH"},
        {IName::LW,             "LW"},
        {IName::LBU,           "LBU"},
        {IName::LHU,           "LHU"},
        {IName::LD,             "LD"},
        {IName::LWU,           "LWU"},
        {IName::ADDI,         "ADDI"},
        {IName::ADDIW,       "ADDIW"},
        {IName::SLTI,         "SLTI"},
        {IName::SLTIU,       "SLTIU"},
        {IName::XORI,         "XORI"},
        {IName::ORI,           "ORI"},
        {IName::ANDI,         "ANDI"},
        {IName::SLLI,         "SLLI"},
        {IName::SRLI,         "SRLI"},
        {IName::SRAI,         "SRAI"},
        {IName::SLLIW,       "SLLIW"},
        {IName::SRLIW,       "SRLIW"},
        {IName::SRLW,         "SRLW"},
        {IName::SRAIW,       "SRAIW"},
        {IName::SRAW,         "SRAW"},
        {IName::FENCE,       "FENCE"},
        {IName::FENCEI,     "FENCEI"},
        {IName::ECALL,       "ECALL"},
        {IName::EBREAK,     "EBREAK"},
        {IName::CSRRW,       "CSRRW"},
        {IName::CSRRS,       "CSRRS"},
        {IName::CSRRC,       "CSRRC"},
        {IName::CSRRWI,     "CSRRWI"},
        {IName::CSRRSI,     "CSRRSI"},
        {IName::CSRRCI,     "CSRRCI"},
        {IName::BEQ,           "BEQ"},
        {IName::BNE,           "BNE"},
        {IName::BLT,           "BLT"},
        {IName::BGE,           "BGE"},
        {IName::BLTU,         "BLTU"},
        {IName::BGEU,         "BGEU"},
        {IName::SB,             "SB"},
        {IName::SH,             "SH"},
        {IName::SW,             "SW"},
        {IName::SD,             "SD"},
        {IName::ADD,           "ADD"},
        {IName::ADDW,         "ADDW"},
        {IName::SUB,           "SUB"},
        {IName::SUBW,         "SUBW"},
        {IName::SLL,           "SLL"},
        {IName::SLLW,         "SLLW"},
        {IName::SLT,           "SLT"},
        {IName::SLTU,         "SLTU"},
        {IName::XOR,           "XOR"},
        {IName::SRL,           "SRL"},
        {IName::SRLW,         "SRLW"},
        {IName::LUI,           "LUI"},
        {IName::AUIPC,       "AUIPC"},
        {IName::JAL,           "JAL"},
        {IName::JALR,         "JALR"},
        {IName::MUL,           "MUL"},
        {IName::MULH,         "MULH"},
        {IName::MULHSU,     "MULHSU"},
        {IName::MULHU,       "MULHU"},
        {IName::DIV,           "DIV"},
        {IName::DIVU,         "DIVU"},
        {IName::REM,           "REM"},
        {IName::REMU,         "REMU"},
        {IName::MULW,         "MULW"},
        {IName::DIVW,         "DIVW"},
        {IName::DIVUW,       "DIVUW"},
        {IName::REMW,         "REMW"},
        {IName::REMUW,       "REMUW"},

        {IName::LRW,           "LRW"},
        {IName::SCW,           "SCW"},
        {IName::AMOSWAPW, "AMOSWAPW"},
        {IName::AMOADDW,   "AMOADDW"},
        {IName::AMOXORW,   "AMOXORW"},
        {IName::AMOANDW,   "AMOANDW"},
        {IName::AMOORW,     "AMOORW"},
        {IName::AMOMINW,   "AMOMINW"},
        {IName::AMOMAXW,   "AMOMAXW"},
        {IName::AMOMINUW, "AMOMINUW"},
        {IName::AMOMAXUW, "AMOMAXUW"},

        {IName::LRD,           "LRD"},
        {IName::SCD,           "SCD"},
        {IName::AMOSWAPD, "AMOSWAPD"},
        {IName::AMOADDD,   "AMOADDD"},
        {IName::AMOXORD,   "AMOXORD"},
        {IName::AMOANDD,   "AMOANDD"},
        {IName::AMOORD,     "AMOORD"},
        {IName::AMOMIND,   "AMOMIND"},
        {IName::AMOMAXD,   "AMOMAXD"},
        {IName::AMOMINUD, "AMOMINUD"},
        {IName::AMOMAXUD, "AMOMAXUD"},

        {IName::SRET,         "SRET"},
        {IName::MRET,         "MRET"},
        
        {IName::FLW,           "FLW"},
        {IName::FSW,           "FSW"},
        {IName::FMADDS,     "FMADDS"},
        {IName::FMSUBS,     "FMSUBS"},
        {IName::FMSUBS,     "FMSUBS"},
        {IName::FNMSUBS,   "FNMSUBS"},
        {IName::FADDS,       "FADDS"},
        {IName::FSUBS,       "FSUBS"},
        {IName::FMULS,       "FMULS"},
        {IName::FDIVS,       "FDIVS"},
        {IName::FSQRTS,     "FSQRTS"},
        {IName::FSGNJS,     "FSGNJS"},
        {IName::FSGNJNS,   "FSGNJNS"},
        {IName::FSGNJXS,   "FSGNJXS"},
        {IName::FMINS,       "FMINS"},
        {IName::FMAXS,       "FMAXS"},
        {IName::FCVTWS,     "FCVTWS"},
        {IName::FCVTWUS,   "FCVTWUS"},
        {IName::FMVXW,       "FMVXW"},
        {IName::FEQS,         "FEQS"},
        {IName::FLTS,         "FLTS"},
        {IName::FLES,         "FLES"},
        {IName::FCLASSS,   "FCLASSS"},
        {IName::FCVTSW,     "FCVTSW"},
        {IName::FCVTSWU,   "FCVTSWU"},
        {IName::FMVWX,       "FMVWX"},

        {IName::FCVTLS,     "FCVTLS"},
        {IName::FCVTLUS,   "FCVTLUS"},
        {IName::FCVTLS,     "FCVTLS"},
        {IName::FCVTSLU,   "FCVTSLU"},

        {IName::XXX,           "XXX"},
};

const std::map<IName, uint8_t> InstructionDecoder::i_opcodes = {
        {IName::LB,     0b0000011},
        {IName::LBU,    0b0000011},
        {IName::LH,     0b0000011},
        {IName::LHU,    0b0000011},
        {IName::LW,     0b0000011},
        {IName::LD,     0b0000011},
        {IName::LWU,    0b0000011},
        {IName::ADDI,   0b0010011},
        {IName::ADDIW,  0b0011011},
        {IName::SLTI,   0b0010011},
        {IName::SLTIU,  0b0010011},
        {IName::XORI,   0b0010011},
        {IName::ORI,    0b0010011},
        {IName::ANDI,   0b0010011},
        {IName::SLLI,   0b0010011}, // RV64I shifts
        {IName::SRLI,   0b0010011}, // RV64I shifts
        {IName::SRAI,   0b0010011}, // RV64I shifts
        {IName::SLLIW,  0b0011011}, // RV32I shifts
        {IName::SRLIW,  0b0011011}, // RV32I shifts
        {IName::SRAIW,  0b0011011}, // RV32I shifts
        {IName::JALR,   0b1100111},
        {IName::FENCE,  0b0001111},
        {IName::FENCEI, 0b0001111},
        {IName::ECALL,  0b1110011},
        {IName::EBREAK, 0b1110011},
        {IName::CSRRW,  0b1110011},
        {IName::CSRRS,  0b1110011},
        {IName::CSRRC,  0b1110011},
        {IName::CSRRWI, 0b1110011},
        {IName::CSRRSI, 0b1110011},
        {IName::CSRRCI, 0b1110011},
        {IName::FLW,    0b0000111},
};

const std::map<IName, uint8_t> InstructionDecoder::r_opcodes = {
        {IName::ADD,      0b0110011},
        {IName::ADDW,     0b0111011},
        {IName::SUB,      0b0110011},
        {IName::SUBW,     0b0111011},
        {IName::SLL,      0b0110011},
        {IName::SLLW,     0b0111011},
        {IName::SLT,      0b0110011},
        {IName::SLTU,     0b0110011},
        {IName::XOR,      0b0110011},
        {IName::SRL,      0b0110011},
        {IName::SRLW,     0b0111011},
        {IName::SRA,      0b0110011},
        {IName::SRAW,     0b0111011},
        {IName::OR,       0b0110011},
        {IName::AND,      0b0110011},

        {IName::MUL,      0b0110011},
        {IName::MULH,     0b0110011},
        {IName::MULHSU,   0b0110011},
        {IName::MULHU,    0b0110011},
        {IName::DIV,      0b0110011},
        {IName::DIVU,     0b0110011},
        {IName::REM,      0b0110011},
        {IName::REMU,     0b0110011},
        {IName::MULW,     0b0111011},
        {IName::DIVW,     0b0111011},
        {IName::DIVUW,    0b0111011},
        {IName::REMW,     0b0111011},
        {IName::REMUW,    0b0111011},

        {IName::LRW,      0b0101111},
        {IName::SCW,      0b0101111},
        {IName::AMOSWAPW, 0b0101111},
        {IName::AMOADDW,  0b0101111},
        {IName::AMOXORW,  0b0101111},
        {IName::AMOANDW,  0b0101111},
        {IName::AMOORW,   0b0101111},
        {IName::AMOMINW,  0b0101111},
        {IName::AMOMAXW,  0b0101111},
        {IName::AMOMINUW, 0b0101111},
        {IName::AMOMAXUW, 0b0101111},
        {IName::AMOSWAPD, 0b0101111},
        {IName::AMOADDD,  0b0101111},
        {IName::AMOXORD,  0b0101111},
        {IName::AMOANDD,  0b0101111},
        {IName::AMOORD,   0b0101111},
        {IName::AMOMIND,  0b0101111},
        {IName::AMOMAXD,  0b0101111},
        {IName::AMOMINUD, 0b0101111},
        {IName::AMOMAXUD, 0b0101111},
        {IName::LRD,      0b0101111},
        {IName::SCD,      0b0101111},

        {IName::FADDS,    0b1010011},
        {IName::FSUBS,    0b1010011},
        {IName::FMULS,    0b1010011},
        {IName::FDIVS,    0b1010011},
        {IName::FSQRTS,   0b1010011},
        {IName::FSGNJS,   0b1010011},
        {IName::FSGNJNS,  0b1010011},
        {IName::FSGNJXS,  0b1010011},
        {IName::FMINS,    0b1010011},
        {IName::FMAXS,    0b1010011},
        {IName::FCVTWS,   0b1010011},
        {IName::FCVTWUS,  0b1010011},
        {IName::FMVXW,    0b1010011},
        {IName::FEQS,     0b1010011},
        {IName::FLTS,     0b1010011},
        {IName::FLES,     0b1010011},
        {IName::FCLASSS,  0b1010011},
        {IName::FCVTSW,   0b1010011},
        {IName::FCVTSWU,  0b1010011},
        {IName::FMVWX,    0b1010011},

        {IName::FCVTLS,   0b1010011},
        {IName::FCVTLUS,  0b1010011},
        {IName::FCVTLS,   0b1010011},
        {IName::FCVTSLU,  0b1010011},
};

const std::map<IName, uint8_t> InstructionDecoder::r4_opcodes = {
        {IName::FMADDS,  0b1000011},
        {IName::FMSUBS,  0b1000111},
        {IName::FNMSUBS, 0b1001011},
        {IName::FNMADDS, 0b1001111},
        {IName::FNMADDS, 0b1001111},
};

const std::map<IName, uint8_t> InstructionDecoder::b_opcodes = {
        {IName::BEQ,  0b1100011},
        {IName::BNE,  0b1100011},
        {IName::BLT,  0b1100011},
        {IName::BGE,  0b1100011},
        {IName::BLTU, 0b1100011},
        {IName::BGEU, 0b1100011},
};

const std::map<IName, uint8_t> InstructionDecoder::s_opcodes = {
        {IName::SB,  0b0100011},
        {IName::SH,  0b0100011},
        {IName::SW,  0b0100011},
        {IName::SD,  0b0100011},
        {IName::FSW, 0b0100111},
};

const std::map<IName, uint8_t> InstructionDecoder::j_opcodes = {
        {IName::JAL, 0b1101111},
};

const std::map<IName, uint8_t> InstructionDecoder::u_opcodes = {
        {IName::LUI,   0b0110111},
        {IName::AUIPC, 0b0010111},
};

const std::map<IName, uint8_t> InstructionDecoder::i_funct3 = {
        {IName::LB,     0b000},
        {IName::LH,     0b001},
        {IName::LW,     0b010},
        {IName::LBU,    0b100},
        {IName::LHU,    0b101},
        {IName::LD,     0b011},
        {IName::LWU,    0b110},
        {IName::ADDI,   0b000},
        {IName::ADDIW,  0b000},
        {IName::SLTI,   0b010},
        {IName::SLTIU,  0b011},
        {IName::XORI,   0b100},
        {IName::ORI,    0b110},
        {IName::ANDI,   0b111},
        {IName::SLLI,   0b001},
        {IName::SRLI,   0b101},
        {IName::SRAI,   0b101},
        {IName::SLLIW,  0b001},
        {IName::SRLIW,  0b101},
        {IName::SRAIW,  0b101},
        {IName::ECALL,  0b000},
        {IName::EBREAK, 0b000},
        {IName::FENCE,  0b000},
        {IName::FENCEI, 0b001},
        {IName::CSRRW,  0b001},
        {IName::CSRRS,  0b010},
        {IName::CSRRC,  0b011},
        {IName::CSRRWI, 0b101},
        {IName::CSRRSI, 0b110},
        {IName::CSRRCI, 0b111},
        {IName::JALR,   0b000},
        {IName::FLW,    0b010},
};

const std::map<IName, uint8_t> InstructionDecoder::r_funct3 = {
        {IName::ADD,      0b000},
        {IName::ADDW,     0b000},
        {IName::SUB,      0b000},
        {IName::SUBW,     0b000},
        {IName::SLL,      0b001},
        {IName::SLLW,     0b001},
        {IName::SRLW,     0b101},
        {IName::SRAW,     0b101},
        {IName::SLT,      0b010},
        {IName::SLTU,     0b011},
        {IName::XOR,      0b100},
        {IName::SRL,      0b101},
        {IName::SRA,      0b101},
        {IName::OR,       0b110},
        {IName::AND,      0b111},
        {IName::LRW,      0b010},
        {IName::SCW,      0b010},
        {IName::MUL,      0b000},
        {IName::MULH,     0b001},
        {IName::MULHSU,   0b010},
        {IName::MULHU,    0b011},
        {IName::DIV,      0b100},
        {IName::DIVU,     0b101},
        {IName::REM,      0b110},
        {IName::REMU,     0b111},
        {IName::MULW,     0b000},
        {IName::DIVW,     0b100},
        {IName::DIVUW,    0b101},
        {IName::REMW,     0b110},
        {IName::REMUW,    0b111},
        {IName::AMOSWAPW, 0b010},
        {IName::AMOADDW,  0b010},
        {IName::AMOXORW,  0b010},
        {IName::AMOANDW,  0b010},
        {IName::AMOORW,   0b010},
        {IName::AMOMINW,  0b010},
        {IName::AMOMAXW,  0b010},
        {IName::AMOMINUW, 0b010},
        {IName::AMOMAXUW, 0b010},

        {IName::AMOSWAPD, 0b011},
        {IName::AMOADDD,  0b011},
        {IName::AMOXORD,  0b011},
        {IName::AMOANDD,  0b011},
        {IName::AMOORD,   0b011},
        {IName::AMOMIND,  0b011},
        {IName::AMOMAXD,  0b011},
        {IName::AMOMINUD, 0b011},
        {IName::AMOMAXUD, 0b011},
        {IName::LRD,      0b011},
        {IName::SCD,      0b011},

        {IName::FSGNJS,   0b000},
        {IName::FSGNJNS,  0b001},
        {IName::FSGNJXS,  0b010},
        {IName::FMINS,    0b000},
        {IName::FMAXS,    0b001},
        {IName::FMVXW,    0b000},
        {IName::FEQS,     0b010},
        {IName::FLTS,     0b001},
        {IName::FLES,     0b000},
        {IName::FCLASSS,  0b001},
        {IName::FMVWX,    0b000},
};

const std::map<IName, uint8_t> InstructionDecoder::r4_funct2 = {
        {IName::FMADDS,  0b00},
        {IName::FMSUBS,  0b00},
        {IName::FNMSUBS, 0b00},
        {IName::FNMADDS, 0b00},
        {IName::FNMADDS, 0b00},
};

const std::map<IName, uint8_t> InstructionDecoder::b_funct3 = {
        {IName::BEQ,  0b000},
        {IName::BNE,  0b001},
        {IName::BLT,  0b100},
        {IName::BGE,  0b101},
        {IName::BLTU, 0b110},
        {IName::BGEU, 0b111},
};

const std::map<IName, uint8_t> InstructionDecoder::s_funct3 = {
        {IName::SB,  0b000},
        {IName::SH,  0b001},
        {IName::SW,  0b010},
        {IName::SD,  0b011},
        {IName::FSW, 0b010},
};

const std::map<IName, uint8_t> InstructionDecoder::r_funct7 = {
        {IName::ADD,      0b0000000},
        {IName::ADDW,     0b0000000},
        {IName::SUB,      0b0100000},
        {IName::SUBW,     0b0100000},
        {IName::SLL,      0b0000000},
        {IName::SLLW,     0b0000000},
        {IName::SLT,      0b0000000},
        {IName::SLTU,     0b0000000},
        {IName::XOR,      0b0000000},
        {IName::SRL,      0b0000000},
        {IName::SRLW,     0b0000000},
        {IName::SRA,      0b0100000},
        {IName::SRAW,     0b0100000},
        {IName::OR,       0b0000000},
        {IName::AND,      0b0000000},

        {IName::MUL,      0b0000001},
        {IName::MULH,     0b0000001},
        {IName::MULHSU,   0b0000001},
        {IName::MULHU,    0b0000001},
        {IName::DIV,      0b0000001},
        {IName::DIVU,     0b0000001},
        {IName::REM,      0b0000001},
        {IName::REMU,     0b0000001},
        {IName::MULW,     0b0000001},
        {IName::DIVW,     0b0000001},
        {IName::DIVUW,    0b0000001},
        {IName::REMW,     0b0000001},
        {IName::REMUW,    0b0000001},

        // Atomic 32-bit word instructions
        // Least significant 2 bits are aq and rl.
        {IName::LRW,      0b0001000},
        {IName::SCW,      0b0001100},
        {IName::AMOSWAPW, 0b0000100},
        {IName::AMOADDW,  0b0000000},
        {IName::AMOXORW,  0b0010000},
        {IName::AMOANDW,  0b0110000},
        {IName::AMOORW,   0b0100000},
        {IName::AMOMINW,  0b1000000},
        {IName::AMOMAXW,  0b1010000},
        {IName::AMOMINUW, 0b1100000},
        {IName::AMOMAXUW, 0b1110000},

        // Atomic 64-bit word instructions
        // Least significant 2 bits are aq and rl.
        {IName::LRD,      0b0001000},
        {IName::SCD,      0b0001100},
        {IName::AMOSWAPD, 0b0000100},
        {IName::AMOADDD,  0b0000000},
        {IName::AMOXORD,  0b0010000},
        {IName::AMOANDD,  0b0110000},
        {IName::AMOORD,   0b0100000},
        {IName::AMOMIND,  0b1000000},
        {IName::AMOMAXD,  0b1010000},
        {IName::AMOMINUD, 0b1100000},
        {IName::AMOMAXUD, 0b1110000},

        {IName::FADDS,    0b0000000},
        {IName::FSUBS,    0b0000100},
        {IName::FMULS,    0b0001000},
        {IName::FDIVS,    0b0001100},
        {IName::FSQRTS,   0b0101100},
        {IName::FSGNJS,   0b0010000},
        {IName::FSGNJNS,  0b0010000},
        {IName::FSGNJXS,  0b0010000},
        {IName::FMINS,    0b0010100},
        {IName::FMAXS,    0b0010100},
        {IName::FCVTWS,   0b1100000},
        {IName::FCVTWUS,  0b1100000},
        {IName::FMVXW,    0b1110000},
        {IName::FEQS,     0b1010000},
        {IName::FLTS,     0b1010000},
        {IName::FLES,     0b1010000},
        {IName::FCLASSS,  0b1110000},
        {IName::FCVTSW,   0b1101000},
        {IName::FCVTSWU,  0b1101000},
        {IName::FMVWX,    0b1111000},

        {IName::FCVTLS,   0b1100000},
        {IName::FCVTLUS,  0b1100000},
        {IName::FCVTLS,   0b1101000},
        {IName::FCVTSLU,  0b1101000},
};

const std::map<IName, uint8_t> InstructionDecoder::i_funct7 = {
        {IName::SLLI,  0b0000000},
        {IName::SRLI,  0b0000000},
        {IName::SRAI,  0b0100000},
        {IName::SLLIW, 0b0000000},
        {IName::SRLIW, 0b0000000},
        {IName::SRAIW, 0b0100000},
};

const std::map<IName, uint8_t> InstructionDecoder::i_funct6 = {
        {IName::SLLI,  0b000000},
        {IName::SRLI,  0b000000},
        {IName::SRAI,  0b010000},
};

Instruction::Type InstructionDecoder::instr_type(uint8_t op)
{
    for (const auto& [k, v] : i_opcodes) {
        if (op == v) return Instruction::Type::I;
    }

    for (const auto& [k, v] : j_opcodes) {
        if (op == v) return Instruction::Type::J;
    }

    for (const auto& [k, v] : s_opcodes) {
        if (op == v) return Instruction::Type::S;
    }

    for (const auto& [k, v] : b_opcodes) {
        if (op == v) return Instruction::Type::B;
    }

    for (const auto& [k, v] : r_opcodes) {
        if (op == v) return Instruction::Type::R;
    }

    for (const auto& [k, v] : r4_opcodes) {
        if (op == v) return Instruction::Type::R4;
    }

    for (const auto& [k, v] : u_opcodes) {
        if (op == v) return Instruction::Type::U;
    }

    return Instruction::Type::WRONG;
}

Instruction::Type InstructionDecoder::instr_type(uint32_t inst)
{
    uint8_t op = extract_opcode(inst);
    return instr_type(op);
}

Fields InstructionDecoder::get_fields(uint32_t inst)
{
    uint8_t funct3 = extract_funct3(inst);
    uint8_t funct7 = extract_funct7(inst);
    uint8_t funct6 = extract_funct6(inst);
    uint8_t op = extract_opcode(inst);
    uint8_t rd = extract_rd(inst);
    uint8_t shamt32 = extract_shamt32(inst);
    uint8_t shamt64 = extract_shamt64(inst);
    uint8_t rs1 = extract_rs1(inst);
    uint8_t rs2 = extract_rs2(inst);
    uint8_t rs3 = extract_rs3(inst);
    uint8_t funct2 = extract_funct2(inst);
    uint32_t imm = get_immediate(inst);

    return Fields {
            .OPCode = op,
            .rd = rd,
            .funct3 = funct3,
            .rs1 = rs1,
            .rs2 = rs2,
            .funct7 = funct7,
            .shamt32 = shamt32,
            .shamt64 = shamt64,
            .funct6 = funct6,
            .imm = imm,
            .funct2 = funct2,
            .rs3 = rs3,
    };
}

uint8_t InstructionDecoder::get_opcode(IName name, Instruction::Type t)
{
    switch (t) {
        case Instruction::Type::R:  return r_opcodes.at(name);
        case Instruction::Type::I:  return i_opcodes.at(name);
        case Instruction::Type::S:  return s_opcodes.at(name);
        case Instruction::Type::B:  return b_opcodes.at(name);
        case Instruction::Type::U:  return u_opcodes.at(name);
        case Instruction::Type::J:  return j_opcodes.at(name);
        case Instruction::Type::R4: return r4_opcodes.at(name);
        case Instruction::Type::WRONG: default: return UINT8_MAX;
    }

    return UINT8_MAX;
}

std::string InstructionDecoder::get_string_name(IName ins)
{
    return inst_string_names.at(ins);
}

uint8_t InstructionDecoder::get_i_funct7(IName n)
{
    if (n == IName::XXX) {
        return UINT8_MAX;
    } else {
        return i_funct7.at(n);
    }
}

uint8_t InstructionDecoder::get_i_funct6(IName n)
{
    if (n == IName::XXX) {
        return UINT8_MAX;
    } else {
        return i_funct6.at(n);
    }
}

Instruction InstructionDecoder::decode_r(uint32_t inst)
{
    if (is_fp_instr(inst)) {
        return decode_fp(inst);
    }

    Fields f = get_fields(inst);
    for (auto &[ins, f3] : r_funct3) {
        auto ins_opcode = get_opcode(ins, Instruction::Type::R);
        auto ins_f7 = r_funct7.at(ins);

        if (f.OPCode == ins_opcode &&
            f3 == f.funct3 &&
            ins_f7 == f.funct7) {
            return Instruction(Instruction::Type::R, ins, f, get_string_name(ins));
        } else if (f.OPCode == ins_opcode &&
                f3 == f.funct3 &&
                f.OPCode == AM_OPCODE) {
            auto f5 = f.funct7 & (~0b11);

            if (f5 == ins_f7) {
                return Instruction(Instruction::Type::R, ins, f, get_string_name(ins));
            }
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

bool InstructionDecoder::is_fp_instr(uint32_t inst)
{
    return (extract_opcode(inst) == FP_R_OPCODE);
}

Instruction InstructionDecoder::decode_fp(uint32_t inst)
{
    if (ignore_f3(inst))
        return decode_fp_without_f3(inst);
    else
        return decode_fp_with_f3(inst);
}

Instruction InstructionDecoder::decode_fp_with_f3(uint32_t inst)
{
    Fields f = get_fields(inst);
    
    for (const auto& [ins, f3] : r_funct3) {
        auto op = get_opcode(ins, Instruction::Type::R);
        auto f7 = r_funct7.at(ins);
        if (f.funct3 == f3 && f.OPCode == op && f.funct7 == f7) {
            return Instruction(Instruction::Type::R, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_fp_without_f3(uint32_t inst)
{
    Fields f = get_fields(inst);
    IName ins = IName::XXX;

    if (f.funct7 == 0b1100000) {
        switch(f.rs2) {
            case 0b00000: ins = IName::FCVTWS; break;
            case 0b00001: ins = IName::FCVTWUS; break;
            case 0b00010: ins = IName::FCVTLS; break;
            case 0b00011: ins = IName::FCVTLUS; break;
            default: ins = IName::XXX;
        }
    } else if (f.funct7 == 0b1101000) {
        switch (f.rs2) {
            case 0b00000: ins = IName::FCVTSW; break;
            case 0b00001: ins = IName::FCVTSWU; break;
            case 0b00010: ins = IName::FCVTSL; break;
            case 0b00011: ins = IName::FCVTSLU; break;
            default: ins = IName::XXX;
        }
    } else {
        for (const auto& [iname, f7] : r_funct7) {
            auto opc = get_opcode(iname, Instruction::Type::R);
            if (f7 == f.funct7 && opc == f.OPCode) {
                ins = iname;
                break;
            }
        }
    }
    return Instruction(Instruction::Type::R, ins, f, get_string_name(ins));
}

bool InstructionDecoder::ignore_f3(uint32_t inst)
{
    auto f7 = extract_funct7(inst);
    return (f7 == 0b0000000 ||
            f7 == 0b0000100 ||
            f7 == 0b0001000 ||
            f7 == 0b0001100 ||
            f7 == 0b0101100 ||
            f7 == 0b1100000 ||
            f7 == 0b1101000);
}

bool InstructionDecoder::decode_with_rs2(uint32_t inst)
{
    auto f7 = extract_funct7(inst);
    return (f7 == 0b1100000 || f7 == 0b1101000);
}

Instruction InstructionDecoder::decode_r4(uint32_t inst)
{
    Fields f = get_fields(inst);
    for (auto &[ins, f2] : r4_funct2) {
        auto ins_opcode = get_opcode(ins, Instruction::Type::R4);

        if (f.OPCode == ins_opcode && f2 == f.funct2) {
            return Instruction(Instruction::Type::R4, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_i(uint32_t inst)
{
    Fields f = get_fields(inst);
    for (const auto &[ins, f3] : i_funct3) {
        if (get_opcode(ins, Instruction::Type::I) == f.OPCode && f3 == f.funct3 && !is_shift_imm_32_instruction(ins)
            && !is_shift_imm_64_instruction(ins)) {
            if ((ins == IName::ECALL && f.imm == 0) ||
                (ins == IName::EBREAK && f.imm == 1)||
                (ins != IName::EBREAK && ins != IName::ECALL)) {
                return Instruction(Instruction::Type::I, ins, f, get_string_name(ins));
            }
        } else if (is_shift_imm_32_instruction(ins) && f.OPCode == get_opcode(ins, Instruction::Type::I) &&
                f3 == f.funct3 && get_i_funct7(ins) == f.funct7) {
            return Instruction(Instruction::Type::I, ins, f, get_string_name(ins));
        } else if (is_shift_imm_64_instruction(ins) && f.OPCode == get_opcode(ins, Instruction::Type::I) &&
                f3 == f.funct3 && get_i_funct6(ins) == f.funct6) {
            return Instruction(Instruction::Type::I, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_b(uint32_t inst)
{
    Fields f = get_fields(inst);
    for (const auto &[ins, f3] : b_funct3) {
        if (f.OPCode == get_opcode(ins, Instruction::Type::B) && f3 == f.funct3) {
            return Instruction(Instruction::Type::B, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_s(uint32_t inst)
{
    Fields f = get_fields(inst);
    for (const auto &[ins, f3] : s_funct3) {
        if (f.OPCode == get_opcode(ins, Instruction::Type::S) && f3 == f.funct3) {
            return Instruction(Instruction::Type::S, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_j(uint32_t inst)
{
    Fields f = get_fields(inst);
    for (const auto &[ins, op] : j_opcodes) {
        if (op == f.OPCode) {
            return Instruction(Instruction::Type::J, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_u(uint32_t inst)
{
    Fields f = get_fields(inst);
    for (const auto &[ins, op] : u_opcodes) {
        if (op == f.OPCode) {
            return Instruction(Instruction::Type::U, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

uint8_t InstructionDecoder::extract_opcode(uint32_t inst)
{
    return inst & OPCODE_MASK;
}

uint8_t InstructionDecoder::extract_funct3(uint32_t inst)
{
    return static_cast<uint8_t>((inst & FUNCT3_MASK) >> 12U);
}

uint8_t InstructionDecoder::extract_funct7(uint32_t inst)
{
    return static_cast<uint8_t>((inst & FUNCT7_MASK) >> 25U);
}

uint8_t InstructionDecoder::extract_funct6(uint32_t inst)
{
    return static_cast<uint8_t>((inst & FUNCT6_MASK) >> 26U);
}

uint8_t InstructionDecoder::extract_rd(uint32_t inst)
{
    return (inst & RD_MASK) >> 7U;
}

uint8_t InstructionDecoder::extract_rs1(uint32_t inst)
{
    return (inst & RS1_MASK) >> 15U;
}

uint8_t InstructionDecoder::extract_rs2(uint32_t inst)
{
    return (inst & RS2_MASK) >> 20U;
}

uint8_t InstructionDecoder::extract_rs3(uint32_t inst)
{
    return (inst & RS3_MASK) >> 27U;
}

uint8_t InstructionDecoder::extract_funct2(uint32_t inst)
{
    return (inst & FUNCT2_MASK) >> 25U;
}

uint8_t InstructionDecoder::extract_shamt32(uint32_t inst)
{
    return (inst & SHAMT32_MASK) >> 20U;
}

uint8_t InstructionDecoder::extract_shamt64(uint32_t inst)
{
    return (inst & SHAMT64_MASK) >> 20U;
}

uint32_t InstructionDecoder::get_immediate(uint32_t inst)
{
    auto t = instr_type(inst);
    switch (t) {
        case Instruction::Type::WRONG:
        case Instruction::Type::R:
        case Instruction::Type::R4:
            return 0;
        case Instruction::Type::I:
            return imm_i(inst);
        case Instruction::Type::J:
            return imm_j(inst);
        case Instruction::Type::B:
            return imm_b(inst);
        case Instruction::Type::S:
            return imm_s(inst);
        case Instruction::Type::U:
            return imm_u(inst);
        default:
            exit(EXIT_FAILURE);
    }
    return 0;
}

uint32_t InstructionDecoder::imm_i(uint32_t inst)
{
    int32_t imm = 0;
    imm = inst & IMM12_MASK;

    return static_cast<uint32_t>(imm >> 20);
}

uint32_t InstructionDecoder::imm_s(uint32_t inst)
{
    int32_t imm = 0;
    imm |= extract_rd(inst);
    imm |= (static_cast<int32_t>(inst) & FUNCT7_MASK) >> 20;
    imm <<= 20;
    imm >>= 20;

    return static_cast<uint32_t>(imm);
}

uint32_t InstructionDecoder::imm_b(uint32_t inst)
{
    int32_t imm = 0;
    imm |= ((static_cast<int32_t>(inst) & B_INST_IMM_0) >> 7) & B_IMM_0;
    imm |= ((static_cast<int32_t>(inst) & B_INST_IMM_1) >> 20) & B_IMM_1;
    imm |= ((static_cast<int32_t>(inst) & B_INST_IMM_2) << 4) & B_IMM_2;
    imm |= ((static_cast<int32_t>(inst) & B_INST_IMM_3) >> 19) & B_IMM_3;

    imm = (imm << 19) >> 19;

    return static_cast<uint32_t>(imm);
}

uint32_t InstructionDecoder::imm_u(uint32_t inst)
{
    return static_cast<uint32_t>(inst & IMM20_MASK);
}

uint32_t InstructionDecoder::imm_j(uint32_t inst)
{
    int32_t imm = 0;
    imm |= ((static_cast<int32_t>(inst) & J_INST_IMM_0) >> 20) & J_IMM_0;
    imm |= ((static_cast<int32_t>(inst) & J_INST_IMM_1) >> 20) & J_IMM_1;
    imm |= ((static_cast<int32_t>(inst) & J_INST_IMM_2) >> 9)  & J_IMM_2;
    imm |= ( static_cast<int32_t>(inst) & J_INST_IMM_3)        & J_IMM_3;
    imm |= ((static_cast<int32_t>(inst) & J_INST_IMM_4) >> 11) & J_IMM_4;

    imm = (imm << 11) >> 11;

    return static_cast<uint32_t>(imm);
}

bool InstructionDecoder::is_shift_imm_32_instruction(IName n)
{
    return n == IName::SLLIW || n == IName::SRLIW || 
           n == IName::SRAIW;
}

bool InstructionDecoder::is_shift_imm_64_instruction(IName n)
{
    return n == IName::SLLI || n == IName::SRLI ||
           n == IName::SRAI;
}

