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

#define FUNCT7_PRIMARY (uint8_t) 0x00000000
#define FUNCT7_ALT     (uint8_t) 0x00100000

enum class IName : uint8_t {
    LB,
    LH,
    LW,
    LBU,
    LHU,
    LD,
    ADDI,
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
