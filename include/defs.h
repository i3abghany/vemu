#pragma once

#define OPCODE_MASK (uint32_t)0x0000007F
#define FUNCT3_MASK (uint32_t)0x00007000
#define FUNCT2_MASK (uint32_t)0x06000000
#define FUNCT7_MASK (uint32_t)0xFE000000
#define FUNCT6_MASK (uint32_t)0xFC000000
#define RD_MASK (uint32_t)0x00000F80
#define RS1_MASK (uint32_t)0x000F8000
#define RS2_MASK (uint32_t)0x01F00000
#define RS3_MASK (uint32_t)0xF1000000
#define IMM12_MASK (uint32_t)0xFFF00000
#define IMM20_MASK (uint32_t)0xFFFFF000

#define SHAMT64_MASK (uint32_t)0x03F00000
#define SHAMT32_MASK (uint32_t)0x01F00000

#define J_IMM_0 (uint32_t)0x0000001E
#define J_IMM_1 (uint32_t)0x000007E0
#define J_IMM_2 (uint32_t)0x00000800
#define J_IMM_3 (uint32_t)0x000FF000
#define J_IMM_4 (uint32_t)0xFFF00000

#define J_INST_IMM_0 (uint32_t)0x01E00000
#define J_INST_IMM_1 (uint32_t)0x7E000000
#define J_INST_IMM_2 (uint32_t)0x00100000
#define J_INST_IMM_3 (uint32_t)0x000FF000
#define J_INST_IMM_4 (uint32_t)0x80000000

#define B_IMM_0 (uint32_t)0x0000001E
#define B_IMM_1 (uint32_t)0x000007E0
#define B_IMM_2 (uint32_t)0x00000800
#define B_IMM_3 (uint32_t)0xFFFFF000

#define B_INST_IMM_0 (uint32_t)0x00000F00
#define B_INST_IMM_1 (uint32_t)0x7E000000
#define B_INST_IMM_2 (uint32_t)0x00000080
#define B_INST_IMM_3 (uint32_t)0x80000000

#define ADDR_BASE (uint64_t)0x8000'0000

#define CLINT_BASE (uint64_t)0x200'0000
#define CLINT_SIZE (uint64_t)0x10000
#define CLINT_MTIMECMP (uint64_t) CLINT_BASE + 0x4000
#define CLINT_MTIME (uint64_t) CLINT_BASE + 0xbff8

#define PLIC_BASE (uint64_t)0xc00'0000
#define PLIC_SIZE (uint64_t)0x400'0000
#define PLIC_PENDING (uint64_t) PLIC_BASE + 0x1000
#define PLIC_SENABLE (uint64_t) PLIC_BASE + 0x2080
#define PLIC_SPRIORITY (uint64_t) PLIC_BASE + 0x201000
#define PLIC_SCLAIM (uint64_t) PLIC_BASE + 0x201004

#define UART_BASE (uint64_t)0x1000'0000
#define UART_SIZE (uint64_t)0x100
#define UART_RHR (uint64_t) UART_BASE + 0
#define UART_THR (uint64_t) UART_BASE + 0
#define UART_LCR (uint64_t) UART_BASE + 3
#define UART_LSR (uint64_t) UART_BASE + 5

/*
 * The 0th bit if set, data is received and is stored in either
 * receive holding register or in the UART FIFO.
 */
#define UART_LSR_RX (uint8_t)1

/*
 * The bit at the 5th bit if reset, the transmit holding register is full and
 * will not receive more data.
 */
#define UART_LSR_TX (uint8_t)(1 << 5)

#define AM_OPCODE (uint8_t)0b0101111

#define FP_R_OPCODE (uint8_t)0b1010011

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
    // RV32/64M
    MUL,
    MULH,
    MULHSU,
    MULHU,
    DIV,
    DIVU,
    REM,
    REMU,
    MULW,
    DIVW,
    DIVUW,
    REMW,
    REMUW,
    // A32-extension instructions
    LRW,
    SCW,
    AMOSWAPW,
    AMOADDW,
    AMOXORW,
    AMOANDW,
    AMOORW,
    AMOMINW,
    AMOMAXW,
    AMOMINUW,
    AMOMAXUW,
    // A64-extension instructions
    LRD,
    SCD,
    AMOSWAPD,
    AMOADDD,
    AMOXORD,
    AMOANDD,
    AMOORD,
    AMOMIND,
    AMOMAXD,
    AMOMINUD,
    AMOMAXUD,

    SRET,
    MRET,

    FLW,
    FSW,
    FMADDS,
    FMSUBS,
    FNMSUBS,
    FNMADDS,
    FADDS,
    FSUBS,
    FMULS,
    FDIVS,
    FSQRTS,
    FSGNJS,
    FSGNJNS,
    FSGNJXS,
    FMINS,
    FMAXS,
    FCVTWS,
    FCVTWUS,
    FMVXW,
    FEQS,
    FLTS,
    FLES,
    FCLASSS,
    FCVTSW,
    FCVTSWU,
    FMVWX,

    FCVTLS,
    FCVTLUS,
    FCVTSL,
    FCVTSLU,

    XXX,
};

#define MHARTID 0xf14
#define MSTATUS 0x300
#define MISA 0x301
#define MTVEC 0x305
#define MEDELEG 0x302
#define MIDELEG 0x303
#define MIP 0x344
#define MIE 0x304
#define MEPC 0x341
#define MCAUSE 0x342
#define MTVAL 0x343
#define MCOUNTEREN 0x306
#define MSCRATCH 0x340

#define SSTATUS 0x100
#define SSCRATCH 0x140
#define STVEC 0x105
#define SIP 0x144
#define SIE 0x104
#define SEPC 0x141
#define SCAUSE 0x142
#define STVAL 0x143
#define SATP 0x180

#define FFLAGS 0x001

#define SSTATUS_SIE_POS 1U
#define SSTATUS_SPIE_POS 5U
#define SSTATUS_SPP_POS 8U

#define MSTATUS_MIE_POS 3U
#define MSTATUS_MPIE_POS 7U

#define MIP_SSIP_POS 1U
#define MIP_MSIP_POS 3U
#define MIP_STIP_POS 5U
#define MIP_MTIP_POS 7U
#define MIP_SEIP_POS 9U
#define MIP_MEIP_POS 11U

#define REG_ZERO 0
#define REG_RA 1
#define REG_SP 2
#define REG_GP 3
#define REG_TP 4
#define REG_T0 5
#define REG_T1 6
#define REG_T2 7
#define REG_S0 8
#define REG_S1 9
#define REG_A0 10
#define REG_A1 11
#define REG_A2 12
#define REG_A3 13
#define REG_A4 14
#define REG_A5 15
#define REG_A6 16
#define REG_A7 17
#define REG_S2 18
#define REG_S3 19
#define REG_S4 20
#define REG_S5 21
#define REG_S6 22
#define REG_S7 23
#define REG_S8 24
#define REG_S9 25
#define REG_S10 26
#define REG_S11 27
#define REG_T3 28
#define REG_T4 29
#define REG_T5 30
#define REG_T6 31

#define SYSCALL_NR_EXIT 57
#define SYSCALL_NR_WRITE 64
#define SYSCALL_NR_FSTAT 80
#define SYSCALL_NR_BRK 214
#define SYSCALL_NR_OPEN 1024

enum class Mode : uint8_t { User = 0b00, Supervisor = 0b01, Machine = 0b11 };

enum class ReturnException : uint8_t {
    InstructionAddressMisaligned = 0,
    InstructionAccessFault = 1,
    IllegalInstruction = 2,
    InstructionAddressBreakpoint = 3,
    LoadAddressMisaligned = 4,
    LoadAccessFault = 5,
    StoreAMOAddressMisaligned = 6,
    StoreAMOAccessFault = 7,
    EnvironmentCallFromUserMode = 8,
    EnvironmentCallFromSupervisorMode = 9,
    EnvironmentCallFromMachineMode = 11,
    InstructionPageFault = 12,
    LoadPageFault = 13,
    StoreAMOPageFault = 15,
    ReadMemoryWithNoPermission = 253,
    UninitializedMemoryAccess = 254,
    NormalExecutionReturn = 255,
};

enum class Interrupt : uint8_t {
    UserSoftwareInterrupt = 0,
    SupervisorSoftwareInterrupt = 1,
    MachineSoftwareInterrupt = 3,
    UserTimerInterrupt = 4,
    SupervisorTimerInterrupt = 5,
    MachineTimerInterrupt = 7,
    UserExternalInterrupt = 8,
    SupervisorExternalInterrupt = 9,
    MachineExternalInterrupt = 11,
    NoInterrupt = 255,
};
