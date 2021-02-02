#include "../include/InstructionDecoder.h"

InstructionDecoder &InstructionDecoder::the() {
    static InstructionDecoder inst;
    return inst;
}

Instruction InstructionDecoder::decode(const uint32_t inst) {
    auto t = instr_type(inst);
    return Instruction(t, Fields());
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

Instruction::Type InstructionDecoder::instr_type(uint32_t inst) {
    uint8_t op = inst & OPCODE_MASK;

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
