#pragma once
#include <cstdint>

struct Fields {
    uint8_t OPCode;
    uint8_t rd;
    uint8_t funct3;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t funct7;
    uint32_t imm;
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
    Instruction(Type t, Fields vals)
        : t(t), f(vals) {};
private:
    Type t;
    Fields f;
};
