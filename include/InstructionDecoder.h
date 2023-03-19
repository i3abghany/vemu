#pragma once

#include <map>
#include <string>
#include <unordered_map>

#include <Instruction.h>
#include <InstructionDecoder.h>

class InstructionDecoder
{
  public:
    static InstructionDecoder& the();

  private:
    InstructionDecoder();

  public:
    void operator=(const InstructionDecoder&) = delete;
    InstructionDecoder(InstructionDecoder&) = delete;

    Instruction decode(uint32_t);
    uint8_t get_opcode(IName name, Instruction::Type t);

    std::string get_string_name(IName);

    uint8_t get_i_funct7(IName);
    uint8_t get_i_funct6(IName);

  private:
    Instruction::Type instr_type(uint32_t instr);
    Instruction::Type instr_type(uint8_t op);

  private:
    uint32_t get_immediate(uint32_t);
    uint32_t imm_i(uint32_t);
    uint32_t imm_s(uint32_t);
    uint32_t imm_b(uint32_t);
    uint32_t imm_u(uint32_t);
    uint32_t imm_j(uint32_t);

  private:
    const static std::map<IName, std::string> inst_string_names;
    std::unordered_map<uint32_t, Instruction> instr_cache;
    std::unordered_map<uint8_t, Instruction::Type> opcode_to_type_cache;

    void init_fixed_instrs();

    const static std::map<IName, uint8_t> i_opcodes;
    const static std::map<IName, uint8_t> j_opcodes;
    const static std::map<IName, uint8_t> u_opcodes;
    const static std::map<IName, uint8_t> r_opcodes;
    const static std::map<IName, uint8_t> r4_opcodes;
    const static std::map<IName, uint8_t> b_opcodes;
    const static std::map<IName, uint8_t> s_opcodes;

    const static std::map<IName, uint8_t> i_funct3;
    const static std::map<IName, uint8_t> r_funct3;
    const static std::map<IName, uint8_t> b_funct3;
    const static std::map<IName, uint8_t> s_funct3;

    const static std::map<IName, uint8_t> r_funct7;
    const static std::map<IName, uint8_t> i_funct7;
    const static std::map<IName, uint8_t> i_funct6;

    const static std::map<IName, uint8_t> r4_funct2;

    Instruction decode_i(uint32_t);
    Instruction decode_r(uint32_t);
    Instruction decode_r4(uint32_t);
    Instruction decode_b(uint32_t);
    Instruction decode_s(uint32_t);
    Instruction decode_j(uint32_t);
    Instruction decode_u(uint32_t);

    bool is_fp_instr(uint32_t);
    Instruction decode_fp(uint32_t);
    bool ignore_f3(uint32_t inst);
    Instruction decode_fp_without_f3(uint32_t inst);
    Instruction decode_fp_with_f3(uint32_t inst);
    bool decode_with_rs2(uint32_t inst);

    uint8_t extract_opcode(uint32_t);
    uint8_t extract_funct2(uint32_t);
    uint8_t extract_funct3(uint32_t);
    uint8_t extract_funct7(uint32_t);
    uint8_t extract_funct6(uint32_t);
    uint8_t extract_shamt32(uint32_t);
    uint8_t extract_shamt64(uint32_t);
    uint8_t extract_rd(uint32_t);
    uint8_t extract_rs1(uint32_t);
    uint8_t extract_rs2(uint32_t);
    uint8_t extract_rs3(uint32_t);

    Fields get_fields(uint32_t inst);

    bool is_shift_imm_32_instruction(IName);
    bool is_shift_imm_64_instruction(IName);
};
