#pragma once
#include "../include/InstructionDecoder.h"

int decode_wrong_test() {
    uint32_t inst = 0x00000000;
    auto a = InstructionDecoder::the().decode(inst);

    if (a.get_type() != Instruction::Type::WRONG) {
        return 1;
    }

    return 0;
}
