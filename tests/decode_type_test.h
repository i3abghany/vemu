#include "../include/InstructionDecoder.h"

int decode_type_test() {
    // LW instruction.
    uint32_t inst = 0x00812403;
    auto a = InstructionDecoder::the().decode(inst);

    if (a.get_type() != Instruction::Type::I) {
        return 1;
    }

    // LUI instruction
    inst = 0x02bcc5b7;
    a = InstructionDecoder::the().decode(inst);

    if (a.get_type() != Instruction::Type::U) {
        return 2;
    }

    // SW instruction
    inst = 0xfea42a23;
    a = InstructionDecoder::the().decode(inst);

    if (a.get_type() != Instruction::Type::S) {
        return 3;
    }

    // ADD instruction
    inst = 0x00b50533;
    a = InstructionDecoder::the().decode(inst);

    if (a.get_type() != Instruction::Type::R) {
        return 4;
    }

    return 0;
}
