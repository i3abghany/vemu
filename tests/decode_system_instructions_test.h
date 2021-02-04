#pragma once
#include "../include/InstructionDecoder.h"

int decode_system_instructions_test() {
    uint32_t inst_1 = 0x100073;

    auto a = InstructionDecoder::the().decode(inst_1);

    if (a.get_name() != IName::EBREAK) {
        return 1;
    }

    if (a.get_fields().funct3 != InstructionDecoder::the().get_funct3(IName::EBREAK)) {
        return 2;
    }

    if (a.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::EBREAK)) {
        return 3;
    }

    uint32_t inst_2 = 0xF;

    a = InstructionDecoder::the().decode(inst_2);

    if (a.get_name() != IName::FENCE) {
        return 1;
    }

    if (a.get_fields().funct3 != InstructionDecoder::the().get_funct3(IName::FENCE)) {
        return 2;
    }

    if (a.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::FENCE)) {
        return 3;
    }

    return 0;
}