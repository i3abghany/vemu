#include "../include/RegFile.h"
RegFile::RegFile()
{
    data.fill(0);
}

void RegFile::store_reg(size_t idx, int64_t value)
{
    if (idx == 0) return;
    data[idx] = value;
}

int64_t RegFile::load_reg(size_t idx)
{
    return data[idx];
}

void RegFile::dump_regs()
{
    for (int i = 0; i < 32; i++) {
        std::cout << "{"
            << std::left
            << std::setfill(' ')
            << std::setw(3)
            << abi_map[i]
            << "} " << "regs["
            << std::left
            << std::setw(2)
            << i << "] = "
            << std::hex
            << data[i] <<
            std::dec << '\n';
    }
}
