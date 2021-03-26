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
#include <utility>
#include <cassert>

#include "InstructionDecoder.h"
#include "RegFile.h"
#include "Bus.h"
#include "util.h"

class VEmu {
public:
    VEmu(std::string f_name);

    uint32_t run();
    void dump_regs();

private:
    void init_func_map();
    std::map<IName, std::function<ReturnException(VEmu *)>> inst_funcs;

    ReturnException LB();
    ReturnException LH();
    ReturnException LW();
    ReturnException LBU();
    ReturnException LHU();
    ReturnException LD();
    ReturnException LWU();
    ReturnException ADDI();
    ReturnException ADDIW();
    ReturnException SLTI();
    ReturnException SLTIU();
    ReturnException XORI();
    ReturnException ORI();
    ReturnException ANDI();
    ReturnException SLLI();
    ReturnException SRLI();
    ReturnException SRAI();
    ReturnException SLLIW();
    ReturnException SRLIW();
    ReturnException SRAIW();
    ReturnException FENCE();
    ReturnException FENCEI();
    ReturnException ECALL();
    ReturnException EBREAK();
    ReturnException CSRRW();
    ReturnException CSRRS();
    ReturnException CSRRC();
    ReturnException CSRRWI();
    ReturnException CSRRSI();
    ReturnException CSRRCI();
    ReturnException BEQ();
    ReturnException BNE();
    ReturnException BLT();
    ReturnException BGE();
    ReturnException BLTU();
    ReturnException BGEU();
    ReturnException SB();
    ReturnException SH();
    ReturnException SW();
    ReturnException SD();
    ReturnException ADD();
    ReturnException ADDW();
    ReturnException SUB();
    ReturnException SUBW();
    ReturnException SLL();
    ReturnException SLLW();
    ReturnException SLT();
    ReturnException SLTU();
    ReturnException XOR();
    ReturnException SRL();
    ReturnException SRLW();
    ReturnException SRA();
    ReturnException SRAW();
    ReturnException OR();
    ReturnException AND();
    ReturnException JAL();
    ReturnException JALR();
    ReturnException LUI();
    ReturnException AUIPC();
    ReturnException XXX();

    ReturnException MUL();
    ReturnException MULH();
    ReturnException MULHSU();
    ReturnException MULHU();
    ReturnException DIV();
    ReturnException DIVU();
    ReturnException REM();
    ReturnException REMU();

    ReturnException MULW();
    ReturnException DIVW();
    ReturnException DIVUW();
    ReturnException REMW();
    ReturnException REMUW();

    ReturnException LRW();
    ReturnException LRD();
    ReturnException SCW();
    ReturnException SCD();

    ReturnException AMOSWAPW();
    ReturnException AMOADDW();
    ReturnException AMOANDW();
    ReturnException AMOXORW();
    ReturnException AMOMINW();
    ReturnException AMOMAXW();
    ReturnException AMOMINUW();
    ReturnException AMOMAXUW();
    ReturnException AMOORW();

    ReturnException AMOSWAPD();
    ReturnException AMOADDD();
    ReturnException AMOANDD();
    ReturnException AMOXORD();
    ReturnException AMOMIND();
    ReturnException AMOMAXD();
    ReturnException AMOMINUD();
    ReturnException AMOMAXUD();
    ReturnException AMOORD();

    ReturnException SRET();
    ReturnException MRET();

private:
    void read_file();
    std::string bin_file_name;

    uint32_t hex_instr;
    Instruction curr_instr;

    uint64_t pc;
    uint64_t code_size;

private:
    Mode mode;

private:
    RegFile rf;

private:
    constexpr static size_t CSR_NUM = 4096;
    std::array<uint64_t, CSR_NUM> csrs;

    uint64_t load_csr(uint64_t);
    void store_csr(uint64_t, uint64_t);

    void dump_csrs();

private:
    Bus bus;
    std::pair<uint32_t, ReturnException> get_4byte_aligned_instr(uint64_t);

    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException store(uint64_t, uint64_t, size_t);

private:
    std::unordered_set<uint64_t> reservation_set;

private:
    void trap(ReturnException e);
    bool is_fatal(ReturnException e);
    void exit_fatally(ReturnException e);
    std::string stringify_exception(ReturnException e);
};

