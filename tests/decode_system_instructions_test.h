#pragma once
#include "../include/InstructionDecoder.h"

int decode_system_instructions_test() {
    // EBREAK
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

    // FENCE
    uint32_t inst_2 = 0xF;

    a = InstructionDecoder::the().decode(inst_2);

    if (a.get_name() != IName::FENCE) {
        return 4;
    }

    if (a.get_fields().funct3 != InstructionDecoder::the().get_funct3(IName::FENCE)) {
        return 5;
    }

    if (a.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::FENCE)) {
        return 6;
    }

    // CSRRCI x8
    uint32_t inst_3 = 0x00007473;

    a = InstructionDecoder::the().decode(inst_3);

    if (a.get_name() != IName::CSRRCI) {
        return 7;
    }

    if (a.get_fields().funct3 != InstructionDecoder::the().get_funct3(IName::CSRRCI)) {
        return 8;
    }

    if (a.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::CSRRCI)) {
        return 9;
    }

    if (a.get_fields().rd != 8) {
        return 10;
    }

    return 0;
}