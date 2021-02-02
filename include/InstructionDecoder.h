#pragma once

#include "Instruction.h"
#include "defs.h"

#include <string>
#include <set>

class InstructionDecoder {
public:
    static InstructionDecoder& the();

private:
    InstructionDecoder() = default;

public:
    void operator=(const InstructionDecoder&) = delete;
    InstructionDecoder(InstructionDecoder&) = delete;

    Instruction decode(uint32_t);
private:
    Instruction::Type instr_type(uint32_t);

    int32_t get_immediate(uint32_t);
    int32_t imm_i(uint32_t);
    int32_t imm_s(uint32_t);
    int32_t imm_b(uint32_t);
    int32_t imm_u(uint32_t);
    int32_t imm_j(uint32_t);

    static std::set<uint8_t> i_opcodes;
    static std::set<uint8_t> j_opcodes;
    static std::set<uint8_t> u_opcodes;
    static std::set<uint8_t> r_opcodes;
    static std::set<uint8_t> b_opcodes;
    static std::set<uint8_t> s_opcodes;
};

