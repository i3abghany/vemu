#pragma once

#include "Instruction.h"

#include <string>
#include <set>
#include <map>

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

    const static std::map<IName, uint8_t> i_funct3;
    const static std::map<IName, uint8_t> r_funct3;
    const static std::map<IName, uint8_t> b_funct3;
    const static std::map<IName, uint8_t> s_funct3;

    const static std::map<IName, uint8_t> r_funct7;

    Instruction decode_i(uint32_t);
    Instruction decode_r(uint32_t);
    Instruction decode_b(uint32_t);
    Instruction decode_s(uint32_t);
    Instruction decode_j(uint32_t);
    Instruction decode_u(uint32_t);

    uint8_t extract_opcode(uint32_t);
    uint8_t extract_funct3(uint32_t);
    uint8_t extract_funct7(uint32_t);
    uint8_t extract_rd(uint32_t);
    uint8_t extract_rs1(uint32_t);
    uint8_t extract_rs2(uint32_t);

    Fields get_fields(uint32_t inst);
};

