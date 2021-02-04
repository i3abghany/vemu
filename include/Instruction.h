#pragma once
#include <cstdint>
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
    Instruction(Type t, IName i, Fields vals)
        : t(t), name(i), f(vals) {};

    Type get_type();
    Fields get_fields();
    IName get_name();

private:
    Type t;
    IName name;
    Fields f;
};
