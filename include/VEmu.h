#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <functional>
#include <unordered_set>
#include <cassert>

#include "InstructionDecoder.h"
#include "Bus.h"
#include "util.h"

class VEmu {
public:
    VEmu(std::string f_name);

    uint32_t run();
    void dump_regs();

private:
    void init_func_map();
    std::map<IName, std::function<void(VEmu *)>> inst_funcs;

    void LB();       void LH();      void LW();       void LBU();
    void LHU();      void LD();      void LWU();      void ADDI();
    void ADDIW();    void SLTI();    void SLTIU();    void XORI();
    void ORI();      void ANDI();    void SLLI();     void SRLI();
    void SRAI();     void SLLIW();   void SRLIW();    void SRAIW();
    void FENCE();    void FENCEI();  void ECALL();    void EBREAK();
    void CSRRW();    void CSRRS();   void CSRRC();    void CSRRWI();
    void CSRRSI();   void CSRRCI();  void BEQ();      void BNE();
    void BLT();      void BGE();     void BLTU();     void BGEU();
    void SB();       void SH();      void SW();       void SD();
    void ADD();      void ADDW();    void SUB();      void SUBW();
    void SLL();      void SLLW();    void SLT();      void SLTU();
    void XOR();      void SRL();     void SRLW();     void SRA();
    void SRAW();     void OR();      void AND();      void JAL();
    void JALR();     void LUI();     void AUIPC();    void XXX();

    void MUL();      void MULH();    void MULHSU();   void MULHU();
    void DIV();      void DIVU();    void REM();      void REMU();

    void MULW();     void DIVW();    void DIVUW();    void REMW();
    void REMUW();

    void LRW();      void LRD();     void SCW();      void SCD();

    void AMOSWAPW(); void AMOADDW(); void AMOANDW();  void AMOXORW();
    void AMOMINW();  void AMOMAXW(); void AMOMINUW(); void AMOMAXUW();
    void AMOORW();

    void AMOSWAPD(); void AMOADDD(); void AMOANDD();  void AMOXORD();
    void AMOMIND();  void AMOMAXD(); void AMOMINUD(); void AMOMAXUD();
    void AMOORD();

    void SRET(); void MRET();

private:
    void read_file();
    std::string bin_file_name;

    uint32_t hex_instr;
    Instruction curr_instr;

    uint64_t pc;
    uint64_t code_size;

private:
    Mode mode;

    constexpr static size_t REGS_NUM = 32;
    std::array<int64_t, REGS_NUM> regs;

    constexpr static std::array<const char *, REGS_NUM> abi_map = {{
        "zero", "ra","sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    }};

private:
    constexpr static size_t CSR_NUM = 4096;
    std::array<uint64_t, CSR_NUM> csrs;

    uint64_t load_csr(uint64_t);
    void store_csr(uint64_t, uint64_t);

private:
    Bus bus;
    uint32_t get_4byte_aligned_instr(uint64_t);

    uint64_t load(uint64_t, size_t);
    void store(uint64_t, uint64_t, size_t);

private:
    std::unordered_set<uint64_t> reservation_set;
};

