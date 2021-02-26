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

#include "InstructionDecoder.h"
#include "Bus.h"
#include "util.h"

class VEmu {
public:
	VEmu(std::string f_name);

	uint32_t run();


private:
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
    void ADD();    void ADDW();   void SUB();   void SUBW();  
	void SLL();    void SLLW();   void SLT();   void SLTU();  
	void XOR();    void SRL();    void SRLW();  void SRA();   
	void SRAW();   void OR();     void AND();   void JAL();   
	void JALR();   void LUI();    void AUIPC(); void XXX(); 
	void LRW();    void LRD();    void SCW();   void SCD(); 

private:
	void read_file();
	std::string bin_file_name;

	Instruction curr_instr;

	uint64_t pc;
	uint64_t code_size;

private:
	Mode mode;

	constexpr static size_t REGS_NUM = 32;
	std::array<int64_t, REGS_NUM> regs;


private:
	constexpr static size_t CSR_NUM = 4096;
	std::array<uint64_t, CSR_NUM> csrs;

	uint64_t load_csr(uint64_t);
	void store_csr(uint64_t, uint64_t);

private:
	Bus bus;
	uint32_t get_4byte_aligned_instr(uint32_t);

	uint64_t load(uint64_t, size_t);
	void store(uint64_t, uint64_t, size_t);

private:
	std::unordered_set<uint64_t> reservation_set;
};

