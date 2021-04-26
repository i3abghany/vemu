#include "../include/Bus.h"

Bus::Bus() {
    dr = DRAM{};
    clic = CLIC{};
    plic = PLIC{};
}

std::pair<uint64_t, ReturnException> Bus::load(uint64_t addr, size_t sz) {
    if (addr >= CLIC_BASE && addr < CLIC_BASE + CLIC_SIZE) {
        return clic.load(addr, sz);
    } else if (addr >= PLIC_BASE && addr < PLIC_BASE + PLIC_SIZE) {
        return plic.load(addr, sz);
    } else if (addr >= ADDR_BASE) {
        return dr.load(addr, sz);
    }

    return {0, ReturnException::LoadAccessFault};
}

ReturnException Bus::store(uint64_t addr, uint64_t data, size_t sz) {
    if (addr >= CLIC_BASE && addr < CLIC_BASE + CLIC_SIZE) {
        return clic.store(addr, data, sz);
    } else if (addr >= PLIC_BASE && addr < PLIC_BASE + PLIC_SIZE) {
        return plic.store(addr, data, sz);
    } else if (addr >= ADDR_BASE) {
        return dr.store(addr, data, sz);
    }

    return ReturnException::LoadAccessFault;
}
