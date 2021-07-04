#pragma once

#include <iostream>
#include <vector>

#include <defs.h>

class DRAM {
public:
    DRAM();
    static constexpr uint64_t RAM_SIZE = 1024 * 1024 * 128;

    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException store(uint64_t, uint64_t, size_t);

private:

    uint64_t load_byte(uint64_t);
    uint64_t load_hword(uint64_t);
    uint64_t load_word(uint64_t);
    uint64_t load_dword(uint64_t);

    void store_byte(uint64_t, uint64_t);
    void store_hword(uint64_t, uint64_t);
    void store_word(uint64_t, uint64_t);
    void store_dword(uint64_t, uint64_t);

private:
    std::vector<uint8_t> ram;
};
