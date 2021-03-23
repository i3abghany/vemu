#include "../include/DRAM.h"

DRAM::DRAM() {
    ram.resize(RAM_SIZE, 0x00);
}

uint64_t DRAM::load(uint64_t addr, size_t sz) {
    uint64_t res = 0x00000000;
    switch (sz) {
        case  8: res = load_byte(addr); break;
        case 16: res = load_hword(addr); break;
        case 32: res = load_word(addr); break;
        case 64: res = load_dword(addr); break;
        default: break;
    }
    return res;
}

void DRAM::store(uint64_t addr, uint64_t data, size_t sz) {
    switch (sz) {
        case  8: store_byte(addr, data); break;
        case 16: store_hword(addr, data); break;
        case 32: store_word(addr, data); break;
        case 64: store_dword(addr, data); break;
        default: break;
    }
}

uint64_t DRAM::load_byte(uint64_t addr) {
    uint64_t res = 0x00000000;
    uint64_t idx = addr - ADDR_BASE;

    res |= static_cast<uint64_t>(ram[idx]);

    return res;
}

uint64_t DRAM::load_hword(uint64_t addr) {
    uint64_t res = 0x00000000;
    uint64_t idx = addr - ADDR_BASE;

    res |= static_cast<uint64_t>(ram[idx]);
    res |= static_cast<uint64_t>(ram[idx + 1]) << 8;

    return res;
}

uint64_t DRAM::load_word(uint64_t addr) {
    uint64_t res = 0x00000000;
    uint64_t idx = addr - ADDR_BASE;

    res |= static_cast<uint64_t>(ram[idx]);
    res |= static_cast<uint64_t>(ram[idx + 1]) << 8;
    res |= static_cast<uint64_t>(ram[idx + 2]) << 16;
    res |= static_cast<uint64_t>(ram[idx + 3]) << 24;
    return res;
}

uint64_t DRAM::load_dword(uint64_t addr) {
    uint64_t res = 0x00000000;
    uint64_t idx = addr - ADDR_BASE;

    res |= static_cast<uint64_t>(ram[idx]);
    res |= static_cast<uint64_t>(ram[idx + 1]) << 8;
    res |= static_cast<uint64_t>(ram[idx + 2]) << 16;
    res |= static_cast<uint64_t>(ram[idx + 3]) << 24;
    res |= static_cast<uint64_t>(ram[idx + 4]) << 32;
    res |= static_cast<uint64_t>(ram[idx + 5]) << 40;
    res |= static_cast<uint64_t>(ram[idx + 6]) << 48;
    res |= static_cast<uint64_t>(ram[idx + 7]) << 56;

    return res;
}

void DRAM::store_byte(uint64_t addr, uint64_t data) {
    uint64_t idx = addr - ADDR_BASE;

    ram[idx] = static_cast<uint8_t>(data);
}

void DRAM::store_hword(uint64_t addr, uint64_t data) {
    uint64_t idx = addr - ADDR_BASE;

    ram[idx] = static_cast<uint8_t>(data) & 0xFF;
    ram[idx + 1] = static_cast<uint8_t>((data >> 8) & 0xFF);
}

void DRAM::store_word(uint64_t addr, uint64_t data) {
    uint64_t idx = addr - ADDR_BASE;

    ram[idx] = static_cast<uint8_t>(data) & 0xFF;
    ram[idx + 1] = static_cast<uint8_t>((data >> 8) & 0xFF);
    ram[idx + 2] = static_cast<uint8_t>((data >> 16) & 0xFF);
    ram[idx + 3] = static_cast<uint8_t>((data >> 24) & 0xFF);
}

void DRAM::store_dword(uint64_t addr, uint64_t data) {
    uint64_t idx = addr - ADDR_BASE;

    ram[idx] = static_cast<uint8_t>(data) & 0xFF;
    ram[idx + 1] = static_cast<uint8_t>((data >> 8) & 0xFF);
    ram[idx + 2] = static_cast<uint8_t>((data >> 16) & 0xFF);
    ram[idx + 3] = static_cast<uint8_t>((data >> 24) & 0xFF);
    ram[idx + 4] = static_cast<uint8_t>((data >> 32) & 0xFF);
    ram[idx + 5] = static_cast<uint8_t>((data >> 40) & 0xFF);
    ram[idx + 6] = static_cast<uint8_t>((data >> 48) & 0xFF);
    ram[idx + 7] = static_cast<uint8_t>((data >> 56) & 0xFF);
}
