#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include "defs.h"

struct Fields {
    uint8_t OPCode;
    uint8_t rd;
    uint8_t funct3;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t funct7;
    uint8_t shamt32;
    uint8_t shamt64;
    uint8_t funct6;
    int32_t imm;
};

class Instruction {
public:
    enum class Type {
        R,
        I,
        S,
        B,
        U,
        J,
        WRONG
    };

public:
    Instruction() = default;
    Instruction(Type t, IName i, Fields vals, std::string n)
        : t(t), name(i), f(vals), str_name(n) {};

    Type get_type();
    Fields get_fields();
    IName get_name();

	friend std::ostream& operator << (std::ostream &os, const Instruction &ins) {
		os << std::hex;

		if (ins.t == Type::WRONG) {
			os << "Faulty Instruction.\n";
			return os;
		}

		os << "Instruction: " << ins.str_name << " ";
		os << "OPCode: " << static_cast<int>(ins.f.OPCode);

		if (ins.t == Type::R) {
			os << " rd: " << static_cast<int>(ins.f.rd);
			os << " rs1: " << static_cast<int>(ins.f.rs1);
			os << " rs2: " << static_cast<int>(ins.f.rs2);
		} else if (ins.t == Type::I) {
			os << " rd: " << static_cast<int>(ins.f.rd);
			os << " rs1: " << static_cast<int>(ins.f.rs1);
			os << " imm: 0x" << static_cast<int>(ins.f.imm);
		} else if (ins.t == Type::S) {
			os << " rs1: " << static_cast<int>(ins.f.rs1);
			os << " rs2: " << static_cast<int>(ins.f.rs2);
			os << " imm: 0x" << static_cast<int>(ins.f.imm);
		} else if (ins.t == Type::B) {
			os << " rs1: " << static_cast<int>(ins.f.rs1);
			os << " rs2: " << static_cast<int>(ins.f.rs2);
			os << " imm: 0x" << static_cast<int>(ins.f.imm);
		} else if (ins.t == Type::U) {
			os << " rd: " << static_cast<int>(ins.f.rd);
			os << " imm: 0x" << static_cast<int>(ins.f.imm);
		} else if (ins.t == Type::J) {
			os << " rd: " << static_cast<int>(ins.f.rd);
			os << " imm: 0x" << static_cast<int>(ins.f.imm);
		} 
		
		return os << "\n";	
	}

private:
    Type t;
    IName name;
    Fields f;
	std::string str_name;
};
