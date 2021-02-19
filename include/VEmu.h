#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <functional>

#include "InstructionDecoder.h"

class VEmu {
public:
	VEmu(std::string f_name);

	uint32_t run();

	void read_file();

private:
	Instruction curr_instr;
	constexpr static size_t REGS_NUM = 32;
	std::array<uint64_t, REGS_NUM> regs;
	
	uint32_t get_4byte_aligned_instr(uint32_t);
	std::vector<uint8_t> code;

	std::map<IName, std::function<void(VEmu *)>> inst_funcs;

    void LB();     void LH();     void LW();    void LBU();
    void LHU();    void LD();     void LWU();   void ADDI();
    void ADDIW();  void SLTI();   void SLTIU(); void XORI();
    void ORI();    void ANDI();   void SLLI();  void SRLI();
    void SRAI();   void SLLIW();  void SRLIW(); void SRAIW();
    void FENCE();  void FENCEI(); void ECALL(); void EBREAK();
    void CSRRW();  void CSRRS();  void CSRRC(); void CSRRWI();
    void CSRRSI(); void CSRRCI(); void BEQ();   void BNE();
    void BLT();    void BGE();    void BLTU();  void BGEU();
    void SB();     void SH();     void SW();    void SD();
    void ADD();    void SUB();    void SUBW();  void SLL();
    void SLLW();   void SLT();    void SLTU();  void XOR();
    void SRL();    void SRLW();   void SRA();   void SRAW();
    void OR();     void AND();    void JAL();   void JALR();
    void LUI();    void AUIPC();  void XXX(); 

	std::string bin_file_name;
};

