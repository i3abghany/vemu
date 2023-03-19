#include <FRegFile.h>

FRegFile::FRegFile()
{
    data.fill(0.0);
}

void FRegFile::store_reg(size_t idx, double value)
{
    data[idx] = value;
}

double FRegFile::load_reg(size_t idx)
{
    return data[idx];
}

std::array<double, 32> FRegFile::get_regs()
{
    return data;
}

void FRegFile::dump_regs()
{
    for (int i = 0; i < 32; i++) {
        std::cout << "{" << std::left << std::setfill(' ') << std::setw(3)
                  << abi_map[i] << "} "
                  << "regs[" << std::left << std::setw(2) << i
                  << "] = " << std::hex << data[i] << std::dec << '\n';
    }
}
