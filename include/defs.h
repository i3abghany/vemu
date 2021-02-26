#pragma once

#define OPCODE_MASK (uint32_t) 0x0000007F
#define FUNCT3_MASK (uint32_t) 0x00007000
#define FUNCT7_MASK (uint32_t) 0xFE000000
#define FUNCT6_MASK (uint32_t) 0xFC000000
#define RD_MASK     (uint32_t) 0x00000F80
#define RS1_MASK    (uint32_t) 0x000F8000
#define RS2_MASK    (uint32_t) 0x01F00000
#define IMM12_MASK  (uint32_t) 0xFFF00000
#define IMM20_MASK  (uint32_t) 0xFFFFF000

#define SHAMT64_MASK  (uint32_t) 0x03F00000
#define SHAMT32_MASK  (uint32_t) 0x01F00000

#define J_IMM_0 (uint32_t) 0x0000001E
#define J_IMM_1 (uint32_t) 0x000007E0
#define J_IMM_2 (uint32_t) 0x00000800
#define J_IMM_3 (uint32_t) 0x000FF000
#define J_IMM_4 (uint32_t) 0xFFF00000

#define J_INST_IMM_0 (uint32_t) 0x01E00000
#define J_INST_IMM_1 (uint32_t) 0x7E000000
#define J_INST_IMM_2 (uint32_t) 0x00100000
#define J_INST_IMM_3 (uint32_t) 0x000FF000
#define J_INST_IMM_4 (uint32_t) 0x80000000

#define B_IMM_0 (uint32_t) 0x0000001E
#define B_IMM_1 (uint32_t) 0x000007E0
#define B_IMM_2 (uint32_t) 0x00000800
#define B_IMM_3 (uint32_t) 0xFFFFF000

#define B_INST_IMM_0 (uint32_t) 0x00000F00
#define B_INST_IMM_1 (uint32_t) 0x7E000000
#define B_INST_IMM_2 (uint32_t) 0x00000080
#define B_INST_IMM_3 (uint32_t) 0x80000000

#define FUNCT7_PRIMARY (uint8_t) 0b00000000
#define FUNCT7_ALT     (uint8_t) 0b00100000

#define ADDR_BASE (uint32_t) 0x8000'0000

enum class IName : uint8_t {
    LB,
    LH,
    LW,
    LBU,
    LHU,
    LD,
    LWU,
    ADDI,
    ADDW,
    ADDIW,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    SLLIW,
    SRLIW,
    SRAIW,
    FENCE,
    FENCEI,
    ECALL,
    EBREAK,
    CSRRW,
    CSRRS,
    CSRRC,
    CSRRWI,
    CSRRSI,
    CSRRCI,
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    SB,
    SH,
    SW,
    SD,
    ADD,
    SUB,
    SUBW,
    SLL,
    SLLW,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRLW,
    SRA,
    SRAW,
    OR,
    AND,
    JAL,
    JALR,
    LUI,
    AUIPC,
    XXX, // wrong instruction.
};

#define MSTATUS 0x300
#define SSTATUS 0x100

#define MTVEC 0x305
#define STVEC 0x105

#define MEDELEG 0x302
#define MIDELEG 0x303

#define MIP 0x344
#define MIE 0x304

#define SIP 0x144
#define SIE 0x104

#define MEPC 0x341 
#define SEPC 0x141

#define MCAUSE 0x342
#define SCAUSE 0x142

#define MTVAL 0x343
#define STVAL 0x143

#define SATP 0x180
