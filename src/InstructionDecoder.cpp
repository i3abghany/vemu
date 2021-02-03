#include <cstring>
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
            return Instruction(t, IName::XXX, Fields{});
    }
}

std::set<uint8_t> InstructionDecoder::i_opcodes = {
        0b0000011, // L*
        0b0010011, // ADDI, SLTI, SLTIU, XORI, ANDI, SLLI, SRLI, SRAI
        0b1100111, // JALR
};

std::set<uint8_t> InstructionDecoder::r_opcodes = {
    0b0110011, // ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND

};

std::set<uint8_t> InstructionDecoder::b_opcodes = {
    0b1100011, // B*

};

std::set<uint8_t> InstructionDecoder::s_opcodes = {
    0b0100011, // S*
};

std::set<uint8_t> InstructionDecoder::j_opcodes = {
    0b1101111, // JAL
};

std::set<uint8_t> InstructionDecoder::u_opcodes = {
    0b0110111, // LUI
    0b0010111, // AUIPC
};

const std::map<IName, uint8_t> InstructionDecoder::i_funct3{
        {IName::LB,    0b000},
        {IName::LH,    0b001},
        {IName::LW,    0b010},
        {IName::LBU,   0b100},
        {IName::LHU,   0b101},
        {IName::ADDI,  0b000},
        {IName::SLTI,  0b010},
        {IName::SLTIU, 0b011},
        {IName::XORI,  0b100},
        {IName::ORI,   0b110},
        {IName::ANDI,  0b111},
        {IName::SLLI,  0b001},
        {IName::SRLI,  0b101},
        {IName::SRAI,  0b101},
};

const std::map<IName, uint8_t> InstructionDecoder::r_funct3{
        {IName::ADD,  0b000},
        {IName::SUB,  0b000},
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
        {IName::BEQ, 0b000},
        {IName::BNE, 0b001},
        {IName::BLT, 0b100},
        {IName::BGE, 0b101},
        {IName::BLTU, 0b110},
        {IName::BGEU, 0b111},
};

const std::map<IName, uint8_t> InstructionDecoder::s_funct3{
        {IName::SB, 0b000},
        {IName::SH, 0b001},
        {IName::SW, 0b010},
};

const std::map<IName, uint8_t> InstructionDecoder::r_funct7{
        {IName::ADD,  FUNCT7_PRIMARY},
        {IName::SUB,  FUNCT7_ALT},
        {IName::SLL,  FUNCT7_PRIMARY},
        {IName::SLT,  FUNCT7_PRIMARY},
        {IName::SLTU, FUNCT7_PRIMARY},
        {IName::XOR,  FUNCT7_PRIMARY},
        {IName::SRL,  FUNCT7_PRIMARY},
        {IName::SRA,  FUNCT7_ALT},
        {IName::OR,   FUNCT7_PRIMARY},
        {IName::AND,  FUNCT7_PRIMARY},
};

Instruction::Type InstructionDecoder::instr_type(uint32_t inst) {
    uint8_t op = extract_opcode(inst);

    if (i_opcodes.find(op) != i_opcodes.end()) {
        return Instruction::Type::I;
    } else if (r_opcodes.find(op) != r_opcodes.end()) {
        return Instruction::Type::R;
    } else if (b_opcodes.find(op) != b_opcodes.end()) {
        return Instruction::Type::B;
    } else if (u_opcodes.find(op) != u_opcodes.end()) {
        return Instruction::Type::U;
    } else if (j_opcodes.find(op) != j_opcodes.end()) {
        return Instruction::Type::J;
    } else if (s_opcodes.find(op) != s_opcodes.end()) {
        return Instruction::Type::S;
    }

    return Instruction::Type::WRONG;
}

Fields InstructionDecoder::get_fields(uint32_t inst) {
    uint8_t funct3 = extract_funct3(inst);
    uint8_t funct7 = extract_funct7(inst);
    uint8_t op = extract_opcode(inst);
    uint8_t rd = extract_rd(inst);
    uint8_t rs1 = extract_rs1(inst);
    uint8_t rs2 = extract_rs2(inst);

    return Fields {
            .OPCode = op,
            .rd = rd,
            .funct3 = funct3,
            .rs1 = rs1,
            .rs2 = rs2,
            .funct7 = funct7,
            .imm = 0,
    };
}

Instruction InstructionDecoder::decode_r(uint32_t inst) {
    Fields f = get_fields(inst);
    for (auto [ins, f3] : r_funct3) {
        if (f3 == f.funct3 && r_funct7.at(ins) == f.funct7) {
            return Instruction(Instruction::Type::R, ins, f);
        }
    }
    return Instruction(Instruction::Type::WRONG, IName::XXX, f);
}

Instruction InstructionDecoder::decode_i(uint32_t) {
    return Instruction();
}

Instruction InstructionDecoder::decode_b(uint32_t) {
    return Instruction();
}

Instruction InstructionDecoder::decode_s(uint32_t) {
    return Instruction();
}

Instruction InstructionDecoder::decode_j(uint32_t) {
    return Instruction();
}

Instruction InstructionDecoder::decode_u(uint32_t) {
    return Instruction();
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

uint8_t InstructionDecoder::extract_rd(uint32_t inst) {
    return (inst & RD_MASK) >> 7U;
}

uint8_t InstructionDecoder::extract_rs1(uint32_t inst) {
    return (inst & RS1_MASK) >> 15U;
}

uint8_t InstructionDecoder::extract_rs2(uint32_t inst) {
    return (inst & RS2_MASK) >> 20U;
}
