#include "../include/InstructionDecoder.h"

InstructionDecoder &InstructionDecoder::the() {
    static InstructionDecoder inst;
    return inst;
}

Instruction InstructionDecoder::decode(const uint32_t inst) {
    auto t = instr_type(inst);
    switch (t) {
        case Instruction::Type::R:
            return decode_r(inst);
        case Instruction::Type::I:
            return decode_i(inst);
        case Instruction::Type::S:
            return decode_s(inst);
        case Instruction::Type::B:
            return decode_b(inst);
        case Instruction::Type::U:
            return decode_u(inst);
        case Instruction::Type::J:
            return decode_j(inst);
        case Instruction::Type::WRONG: default:
            return Instruction(t, IName::XXX, Fields{}, "XXX");
    }
}

const std::map<IName, std::string> InstructionDecoder::inst_string_names = {
		{IName::LB,         "LB"},
		{IName::LH,         "LH"},
		{IName::LW,         "LW"},
		{IName::LBU,       "LBU"},
		{IName::LHU,       "LHU"},
		{IName::LD,         "LD"},
		{IName::LWU,       "LWU"},
		{IName::ADDI,     "ADDI"},
		{IName::ADDIW,   "ADDIW"},
		{IName::SLTI,     "SLTI"},
		{IName::SLTIU,   "SLTIU"},
		{IName::XORI,     "XORI"},
		{IName::ORI,       "ORI"},
		{IName::ANDI,     "ANDI"},
		{IName::SLLI,     "SLLI"},
		{IName::SRLI,     "SRLI"},
		{IName::SRAI,     "SRAI"},
		{IName::SLLIW,   "SLLIW"},
		{IName::SRLIW,   "SRLIW"},
		{IName::SRAIW,   "SRAIW"},
		{IName::FENCE,   "FENCE"},
		{IName::FENCEI, "FENCEI"},
		{IName::ECALL,   "ECALL"},
		{IName::EBREAK, "EBREAK"},
		{IName::CSRRW,   "CSRRW"},
		{IName::CSRRS,   "CSRRS"},
		{IName::CSRRC,   "CSRRC"},
		{IName::CSRRWI, "CSRRWI"},
		{IName::CSRRSI, "CSRRSI"},
		{IName::CSRRCI, "CSRRCI"},
		{IName::BEQ,       "BEQ"},
		{IName::BNE,       "BNE"},
		{IName::BLT,       "BLT"},
		{IName::BGE,       "BGE"},
		{IName::BLTU,     "BLTU"},
		{IName::BGEU,     "BGEU"},
		{IName::SB,         "SB"},
		{IName::SH,         "SH"},
		{IName::SW,         "SW"},
		{IName::SD,         "SD"},
		{IName::ADD,       "ADD"},
		{IName::SUB,       "SUB"},
		{IName::SUBW,     "SUBW"},
		{IName::SLL,       "SLL"},
		{IName::SLLW,     "SLLW"},
		{IName::SLT,       "SLT"},
		{IName::SLTU,     "SLTU"},
		{IName::XOR,       "XOR"},
		{IName::SRL,       "SRL"},
		{IName::SRLW,     "SRLW"},
		{IName::XXX,       "XXX"},
        {IName::LUI,       "LUI"},
        {IName::AUIPC,   "AUIPC"},
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
};

const std::map<IName, uint8_t> InstructionDecoder::r_opcodes = {
        {IName::ADD,  0b0110011},
        {IName::ADDW, 0b0111011},
        {IName::SUB,  0b0110011},
        {IName::SUBW, 0b0111011},
        {IName::SLL,  0b0110011},
        {IName::SLLW, 0b0111011},
        {IName::SLT,  0b0110011},
        {IName::SLTU, 0b0110011},
        {IName::XOR,  0b0110011},
        {IName::SRL,  0b0110011},
        {IName::SRLW, 0b0111011},
        {IName::SRA,  0b0110011},
        {IName::SRAW, 0b0111011},
        {IName::OR,   0b0110011},
        {IName::AND,  0b0110011},
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
        {IName::SB, 0b0100011},
        {IName::SH, 0b0100011},
        {IName::SW, 0b0100011},
        {IName::SD, 0b0100011},
};

const std::map<IName, uint8_t> InstructionDecoder::j_opcodes = {
        {IName::JAL, 0b1101111},
};

const std::map<IName, uint8_t> InstructionDecoder::u_opcodes = {
        {IName::LUI,   0b0110111},
        {IName::AUIPC, 0b0010111},
};

const std::map<IName, uint8_t> InstructionDecoder::i_funct3{
        {IName::LB,     0b000},
        {IName::LH,     0b001},
        {IName::LW,     0b010},
        {IName::LBU,    0b100},
        {IName::LHU,    0b101},
        {IName::LD,     0b011},
        {IName::LWU,    0b110},
        {IName::JALR,   0b000},
        {IName::BEQ,    0b000},
        {IName::BNE,    0b001},
        {IName::BLT,    0b100},
        {IName::BGE,    0b101},
        {IName::BLTU,   0b110},
        {IName::BGEU,   0b111},
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
        {IName::SLLW,   0b001},
        {IName::SRLW,   0b101},
        {IName::SRAW,   0b101},
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
};

const std::map<IName, uint8_t> InstructionDecoder::r_funct3{
        {IName::ADD,  0b000},
        {IName::ADDW, 0b000},
        {IName::SUB,  0b000},
        {IName::SUBW, 0b000},
        {IName::SLL,  0b001},
        {IName::SLT,  0b010},
        {IName::SLTU, 0b011},
        {IName::XOR,  0b100},
        {IName::SRL,  0b101},
        {IName::SRA,  0b101},
        {IName::OR,   0b110},
        {IName::AND,  0b111},
};

const std::map<IName, uint8_t> InstructionDecoder::b_funct3{
        {IName::BEQ,  0b000},
        {IName::BNE,  0b001},
        {IName::BLT,  0b100},
        {IName::BGE,  0b101},
        {IName::BLTU, 0b110},
        {IName::BGEU, 0b111},
};

const std::map<IName, uint8_t> InstructionDecoder::s_funct3{
        {IName::SB, 0b000},
        {IName::SH, 0b001},
        {IName::SW, 0b010},
        {IName::SD, 0b011},
};

const std::map<IName, uint8_t> InstructionDecoder::r_funct7 {
        {IName::ADD,  FUNCT7_PRIMARY},
        {IName::ADDW, FUNCT7_PRIMARY},
        {IName::SUB,  FUNCT7_ALT},
        {IName::SUBW, FUNCT7_ALT},
        {IName::SLL,  FUNCT7_PRIMARY},
        {IName::SLLW, FUNCT7_PRIMARY},
        {IName::SLT,  FUNCT7_PRIMARY},
        {IName::SLTU, FUNCT7_PRIMARY},
        {IName::XOR,  FUNCT7_PRIMARY},
        {IName::SRL,  FUNCT7_PRIMARY},
        {IName::SRLW, FUNCT7_PRIMARY},
        {IName::SRA,  FUNCT7_ALT},
        {IName::SRAW, FUNCT7_ALT},
        {IName::OR,   FUNCT7_PRIMARY},
        {IName::AND,  FUNCT7_PRIMARY},
};

const std::map<IName, uint8_t> InstructionDecoder::i_funct7 {
        {IName::SLLI,  0b0000000},
        {IName::SRLI,  0b0000000},
        {IName::SRAI,  0b0100000},
        {IName::SLLIW, 0b0000000},
        {IName::SRLIW, 0b0000000},
        {IName::SRAIW, 0b0100000},
};

const std::map<IName, uint8_t> InstructionDecoder::i_funct6 {
        {IName::SLLI,  0b000000},
        {IName::SRLI,  0b000000},
        {IName::SRAI,  0b010000},
};

Instruction::Type InstructionDecoder::instr_type(uint8_t op) {
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

    for (const auto& [k, v] : u_opcodes) {
        if (op == v) return Instruction::Type::U;
    }

    return Instruction::Type::WRONG;
}

Instruction::Type InstructionDecoder::instr_type(uint32_t inst) {
    uint8_t op = extract_opcode(inst);
    return instr_type(op);
}

Fields InstructionDecoder::get_fields(uint32_t inst) {
    uint8_t funct3 = extract_funct3(inst);
    uint8_t funct7 = extract_funct7(inst);
    uint8_t funct6 = extract_funct6(inst);
    uint8_t op = extract_opcode(inst);
    uint8_t rd = extract_rd(inst);
    uint8_t shamt32 = extract_shamt32(inst);
    uint8_t shamt64 = extract_shamt64(inst);
    uint8_t rs1 = extract_rs1(inst);
    uint8_t rs2 = extract_rs2(inst);
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
    };
}

uint8_t InstructionDecoder::get_opcode(IName name) {
    if (name == IName::XXX) {
        return UINT8_MAX;
    }
    if (r_opcodes.find(name) != r_opcodes.end())
        return r_opcodes.at(name);
    if (i_opcodes.find(name) != i_opcodes.end())
        return i_opcodes.at(name);
    if (j_opcodes.find(name) != j_opcodes.end())
        return j_opcodes.at(name);
    if (s_opcodes.find(name) != s_opcodes.end())
        return s_opcodes.at(name);
    if (b_opcodes.find(name) != b_opcodes.end())
        return b_opcodes.at(name);
    if (u_opcodes.find(name) != u_opcodes.end())
        return u_opcodes.at(name);

    return UINT8_MAX;
}

IName InstructionDecoder::get_iname(uint32_t inst) {
    auto op = extract_opcode(inst);
    auto funct3 = extract_funct3(inst);
    auto funct7 = extract_funct7(inst);
    Instruction::Type t = instr_type(op);

    if (t == Instruction::Type::WRONG) {
        return IName::XXX;
    }

    if (t == Instruction::Type::I) {
        for (const auto&[k, v] : i_opcodes) {
            if (op == v && i_funct3.at(k) == funct3) return k;
        }
        return IName::XXX;
    }
    if (t == Instruction::Type::J) {
        for (const auto&[k, v] : j_opcodes) {
            if (op == v) return k;
        }
        return IName::XXX;
    }

    if (t == Instruction::Type::S) {
        for (const auto&[k, v] : s_opcodes) {
            if (op == v && s_funct3.at(k) == funct3) return k;
        }
        return IName::XXX;
    }

    if (t == Instruction::Type::B) {
        for (const auto&[k, v] : b_opcodes) {
            if (op == v && b_funct3.at(k) == funct3) return k;
        }
        return IName::XXX;
    }
    if (t == Instruction::Type::R) {
        for (const auto&[k, v] : r_opcodes) {
            if (op == v && r_funct3.at(k) == funct3 && r_funct7.at(k) == funct7)
                return k;
        }
        return IName::XXX;
    }

    if (t == Instruction::Type::U) {
        for (const auto&[k, v] : u_opcodes) {
            if (op == v) return k;
        }
        return IName::XXX;
    }

    return IName::XXX;
}

std::string InstructionDecoder::get_string_name(IName ins) {
	return inst_string_names.at(ins);
}

uint8_t InstructionDecoder::get_i_funct7(IName n) {
    if (n == IName::XXX) {
        return UINT8_MAX;
    } else {
        return i_funct7.at(n);
    }
}

uint8_t InstructionDecoder::get_i_funct6(IName n) {
    if (n == IName::XXX) {
        return UINT8_MAX;
    } else {
        return i_funct6.at(n);
    }
}

uint8_t InstructionDecoder::get_funct3(IName n) {
    auto op = get_opcode(n);

    if (op == UINT8_MAX) {
        return op;
    }
    auto t = instr_type(op);

    if (t == Instruction::Type::WRONG) {
        return UINT8_MAX;
    }

    if (t == Instruction::Type::I &&
        i_funct3.find(n) != i_funct3.end()) {
        return i_funct3.at(n);
    }

    if (t == Instruction::Type::S &&
        s_funct3.find(n) != s_funct3.end()) {
        return s_funct3.at(n);
    }

    if (t == Instruction::Type::B &&
        b_funct3.find(n) != b_funct3.end()) {
        return b_funct3.at(n);
    }

    if (t == Instruction::Type::R &&
        r_funct3.find(n) != b_funct3.end()) {
        return r_funct3.at(n);
    }

    return UINT8_MAX;
}

uint8_t InstructionDecoder::get_funct7(IName n) {
    auto op = get_opcode(n);
    auto t = instr_type(op);

    if (t != Instruction::Type::R)
        return UINT8_MAX;

    if (r_funct7.find(n) != r_funct7.end()) {
        return r_funct7.at(n);
    }

    return UINT8_MAX;
}

Instruction InstructionDecoder::decode_r(uint32_t inst) {
    Fields f = get_fields(inst);
    for (auto &[ins, f3] : r_funct3) {
        if (f.OPCode == get_opcode(ins) && f3 == f.funct3 &&
            r_funct7.at(ins) == f.funct7) {
            return Instruction(Instruction::Type::R, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_i(uint32_t inst) {
    Fields f = get_fields(inst);
    for (auto[ins, f3] : i_funct3) {
        if (get_opcode(ins) == f.OPCode && f3 == f.funct3 && !is_shift_imm_32_instruction(ins)
            && !is_shift_imm_64_instruction(ins)) {
            if (ins == IName::ECALL && f.imm == 0 ||
                ins == IName::EBREAK && f.imm == 1 ||
                ins != IName::EBREAK && ins != IName::ECALL) {
                return Instruction(Instruction::Type::I, ins, f, get_string_name(ins));
            }
        } else if (is_shift_imm_32_instruction(ins) && f.OPCode == get_opcode(ins) &&
                f3 == f.funct3 && get_i_funct7(ins) == f.funct7) {
            return Instruction(Instruction::Type::I, ins, f, get_string_name(ins));
        } else if (is_shift_imm_64_instruction(ins) && f.OPCode == get_opcode(ins) &&
                f3 == f.funct3 && get_i_funct6(ins) == f.funct6) {
            return Instruction(Instruction::Type::I, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_b(uint32_t inst) {
    Fields f = get_fields(inst);
    for (const auto &[ins, f3] : b_funct3) {
        if (f.OPCode == get_opcode(ins) && f3 == f.funct3) {
            return Instruction(Instruction::Type::B, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_s(uint32_t inst) {
    Fields f = get_fields(inst);
    for (const auto &[ins, f3] : s_funct3) {
        if (f.OPCode == get_opcode(ins) && f3 == f.funct3) {
            return Instruction(Instruction::Type::S, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_j(uint32_t inst) {
    Fields f = get_fields(inst);
    for (const auto &[ins, op] : j_opcodes) {
        if (op == f.OPCode) {
            return Instruction(Instruction::Type::J, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

Instruction InstructionDecoder::decode_u(uint32_t inst) {
    Fields f = get_fields(inst);
    for (const auto &[ins, op] : u_opcodes) {
        if (op == f.OPCode) {
            return Instruction(Instruction::Type::U, ins, f, get_string_name(ins));
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f, "XXX");
}

uint8_t InstructionDecoder::extract_opcode(uint32_t inst) {
    return inst & OPCODE_MASK;
}

uint8_t InstructionDecoder::extract_funct3(uint32_t inst) {
    return (inst & FUNCT3_MASK) >> 12U;
}

uint8_t InstructionDecoder::extract_funct7(uint32_t inst) {
    return (inst & FUNCT7_MASK) >> 25U;
}

uint8_t InstructionDecoder::extract_funct6(uint32_t inst) {
    return (inst & FUNCT6_MASK) >> 26U;
}

uint8_t InstructionDecoder::extract_rd(uint32_t inst) {
    return (inst & RD_MASK) >> 7U;
}

uint8_t InstructionDecoder::extract_rs1(uint32_t inst) {
    return (inst & RS1_MASK) >> 15U;
}

uint8_t InstructionDecoder::extract_rs2(uint32_t inst) {
    return (inst & RS2_MASK) >> 20U;
}

uint8_t InstructionDecoder::extract_shamt32(uint32_t inst) {
    return (inst & SHAMT32_MASK) >> 20U;
}

uint8_t InstructionDecoder::extract_shamt64(uint32_t inst) {
    return (inst & SHAMT64_MASK) >> 20U;
}

uint32_t InstructionDecoder::get_immediate(uint32_t inst) {
     auto t = instr_type(inst);

     if (t == Instruction::Type::WRONG) {
         return 0;
     }

     if (t == Instruction::Type::R) {
         return 0;
     }

     if (t == Instruction::Type::I) {
         return imm_i(inst);
     }

     if (t == Instruction::Type::J) {
         return imm_j(inst);
     }

     if (t == Instruction::Type::B) {
         return imm_b(inst);
     }

     if (t == Instruction::Type::S) {
         return imm_s(inst);
     }

     if (t == Instruction::Type::U) {
         return imm_u(inst);
     }

     return 0;
}

uint32_t InstructionDecoder::imm_i(uint32_t inst) {
    int32_t imm = 0;
    imm = inst & IMM12_MASK;

    return static_cast<uint32_t>(imm >> 20);
}

uint32_t InstructionDecoder::imm_s(uint32_t inst) {
    int32_t imm = 0;
    imm |= extract_rd(inst);
    imm |= (static_cast<int32_t>(inst) & FUNCT7_MASK) >> 20;
    imm <<= 20;
	imm >>= 20;

	return static_cast<uint32_t>(imm);
}

uint32_t InstructionDecoder::imm_b(uint32_t inst) {
    int32_t imm = 0;
    imm |= ((static_cast<int32_t>(inst) & B_INST_IMM_0) >> 7) & B_IMM_0;
    imm |= ((static_cast<int32_t>(inst) & B_INST_IMM_1) >> 20) & B_IMM_1;
    imm |= ((static_cast<int32_t>(inst) & B_INST_IMM_2) << 4) & B_IMM_2;
    imm |= ((static_cast<int32_t>(inst) & B_INST_IMM_3) >> 19) & B_IMM_3;

    imm = (imm << 19) >> 19;

	return static_cast<uint32_t>(imm);
}

uint32_t InstructionDecoder::imm_u(uint32_t inst) {
	return static_cast<uint32_t>(inst & IMM20_MASK);
}

uint32_t InstructionDecoder::imm_j(uint32_t inst) {
    int32_t imm = 0;
    imm |= ((static_cast<int32_t>(inst) & J_INST_IMM_0) >> 20) & J_IMM_0;
    imm |= ((static_cast<int32_t>(inst) & J_INST_IMM_1) >> 20) & J_IMM_1;
    imm |= ((static_cast<int32_t>(inst) & J_INST_IMM_2) >> 9)  & J_IMM_2;
    imm |= ( static_cast<int32_t>(inst) & J_INST_IMM_3)        & J_IMM_3;
    imm |= ((static_cast<int32_t>(inst) & J_INST_IMM_4) >> 11) & J_IMM_4;

    imm = (imm << 11) >> 11;

	return static_cast<uint32_t>(imm);
}

bool InstructionDecoder::is_shift_imm_32_instruction(IName n) {
    return n == IName::SLLIW ||
           n == IName::SRLIW || n == IName::SRAIW;
}

bool InstructionDecoder::is_shift_imm_64_instruction(IName n) {
    return n == IName::SLLI || n == IName::SRLI ||
           n == IName::SRAI;
}

