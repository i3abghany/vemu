#define OPCODE_MASK (uint32_t) 0x0000007F
#define FUNCT3_MASK (uint32_t) 0x00007000
#define FUNCT7_MASK (uint32_t) 0xF7000000
#define RD_MASK     (uint32_t) 0x00000F80
#define RS1_MASK    (uint32_t) 0x000F8000
#define RS2_MASK    (uint32_t) 0x01F00000

#define FUNCT7_PRIMARY (uint8_t) 0x00000000
#define FUNCT7_ALT (uint8_t) 0x00100000

enum class IName : uint8_t {
    LB,
    LH,
    LW,
    LBU,
    LHU,
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    SB,
    SH,
    SW,
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    JAL,
    JALR,
    LUI,
    AUIPC,
    XXX, // wrong instruction.
};
