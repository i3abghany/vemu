#pragma once 
#include <array>
#include <iostream>
#include <iomanip>

class FRegFile {
public:
    FRegFile();
public:
    void store_reg(size_t idx, double value);
    double load_reg(size_t idx);
    void dump_regs();

private:
    static constexpr size_t REGS_NUM = 32;
    std::array<double, REGS_NUM> data;

    constexpr static std::array<const char *, REGS_NUM> abi_map = {{
        "ft0",  "ft1",  "ft2",  "ft3",   "ft4", "ft5", "ft6",  "ft7",
        "fs0",  "fs1",  "fa0",  "fa1",   "fa2", "fa3", "fa4",  "fa5",
        "fa6",  "fa7",  "fs2",  "fs3",   "fs4", "fs5", "fs6",  "fs7",
        "fs8",  "fs9",  "fs10", "fs11",  "ft8", "ft9", "ft10", "ft11",
    }};

};

