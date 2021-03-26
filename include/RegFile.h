#pragma once

#include <array>
#include <iostream>
#include <iomanip>

class RegFile {
public:
    RegFile();
public:
    void store_reg(size_t idx, uint64_t value);
    int64_t load_reg(size_t idx);
    void dump_regs();

private:
    static constexpr size_t REGS_NUM = 32;
    std::array<int64_t, REGS_NUM> data;

    constexpr static std::array<const char *, REGS_NUM> abi_map = {{
        "zero", "ra","sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    }};

};
