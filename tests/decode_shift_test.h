#pragma once
#include "../include/InstructionDecoder.h"

int decode_shift_test() {
    // SLL x1, x2, x4
    uint32_t inst_1 = 0x004110b3;

    auto a = InstructionDecoder::the().decode(inst_1);

    if (a.get_name() != IName::SLL) {
        return 1;
    }

    if (a.get_type() != Instruction::Type::R) {
        return 2;
    }

    if (a.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::SLL)) {
        return 3;
    }

    if (a.get_fields().rd != 1) {
        return 4;
    }

    if (a.get_fields().rs1 != 2) {
        return 5;
    }

    if (a.get_fields().rs2 != 4) {
        return 6;
    }

    // SRAI x10, x10, 0x3F
    uint32_t inst_2 = 0x43F55513;

    a = InstructionDecoder::the().decode(inst_2);

    if (a.get_name() != IName::SRAI) {
        return 7;
    }

    if (a.get_fields().shamt64 != 0x3F) {
        return 8;
    }

    if (a.get_fields().rd != 10) {
        return 9;
    }

    if (a.get_fields().rs1 != 10) {
        return 10;
    }

    // srliw x17, x17, 0x18
    uint32_t inst_3 = 0x0188d89b;

    a = InstructionDecoder::the().
            decode(inst_3);

    if (a.get_name() != IName::SRLIW) {
        return 11;
    }

    if (a.get_fields().shamt32 != 0x18) {
        return 12;
    }

    return 0;
}