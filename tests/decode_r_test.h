#pragma once
#include "../include/InstructionDecoder.h"

int decode_r_test() {
    uint32_t inst = 0x003100b3;
    auto a = InstructionDecoder::the().decode(inst);

    if (a.get_type() != Instruction::Type::R) {
        return 1;
    }

    if (a.get_name() != InstructionDecoder::the().get_iname(inst)) {
        return 1;
    }

    if (a.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::ADD)) {
        return 2;
    }

    if (a.get_fields().funct3 != InstructionDecoder::the().get_funct3(IName::ADD)) {
        return 2;
    }

    return 0;
}