#include "../include/InstructionDecoder.h"

InstructionDecoder &InstructionDecoder::the() {
    static InstructionDecoder inst;
    return inst;
}

Instruction InstructionDecoder::decode(const uint32_t inst) {
    return Instruction(Instruction::Type::S, Fields());
}

std::set<uint8_t> InstructionDecoder::i_opcodes = {
    // TODO: complete OPCodes.
};

std::set<uint8_t> InstructionDecoder::r_opcodes = {
    // TODO: complete OPCodes.
};

std::set<uint8_t> InstructionDecoder::b_opcodes = {
    // TODO: complete OPCodes.
};

std::set<uint8_t> InstructionDecoder::s_opcodes = {
    // TODO: complete OPCodes.
};

std::set<uint8_t> InstructionDecoder::j_opcodes = {
    // TODO: complete OPCodes.
};

std::set<uint8_t> InstructionDecoder::u_opcodes = {
    // TODO: complete OPCodes.
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
