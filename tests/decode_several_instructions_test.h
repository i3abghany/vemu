#pragma once

#include <vector>
#include <iostream>
#include "../include/InstructionDecoder.h"

int decode_several_instructions_test() {
    std::vector<uint32_t> instructions {
            0x004000ef, // jal x1, 4
            0x003160b3, // or x1, x2, x3
            0xfe41eee3, // bltu x3, x4, -4
            0x0000a523, // sw x0, 10(x1)
    };

    auto i = InstructionDecoder::the().decode(instructions[0]);

    if (i.get_name() != IName::JAL) {
        return 1;
    }

    if (i.get_type() != Instruction::Type::J) {
        return 2;
    }

    if (i.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::JAL)) {
        return 3;
    }

    if (i.get_fields().imm != 4) {
        return 4;
    }

    i = InstructionDecoder::the().decode(instructions[1]);

    if (i.get_type() != Instruction::Type::R) {
        return 5;
    }

    if (i.get_name() != IName::OR) {
        return 6;
    }

    if (i.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::OR)) {
        return 7;
    }

    if (i.get_fields().rd != 1) {
        return 8;
    }

    if (i.get_fields().rs1 != 2) {
        return 9;
    }

    if (i.get_fields().rs2 != 3) {
        return 10;
    }

    i = InstructionDecoder::the().decode(instructions[2]);

    if (i.get_name() != IName::BLTU) {
        return 11;
    }

    if (i.get_type() != Instruction::Type::B) {
        return 12;
    }

    if (i.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::BLTU)) {
        return 13;
    }

    if (i.get_fields().rs1 != 3) {
        return 14;
    }

    if (i.get_fields().rs2 != 4) {
        return 15;
    }

    if (i.get_fields().imm != -4) {
        return 16;
    }

    i = InstructionDecoder::the().decode(instructions[3]);

    if (i.get_name() != IName::SW) {
        return 17;
    }

    if (i.get_fields().OPCode != InstructionDecoder::the().get_opcode(IName::SW)) {
        return 18;
    }

    if (i.get_fields().funct3 != InstructionDecoder::the().get_funct3(IName::SW)) {
        return 19;
    }

    if (i.get_fields().rs1 != 1) {
        return 20;
    }

    if (i.get_fields().rs2 != 0) {
        return 21;
    }
    return 0;
}