#include "../include/Bus.h"

Bus::Bus() {
    dr = DRAM{};
}

uint64_t Bus::load(uint64_t addr, size_t sz) {
    return dr.load(addr, sz);
}

void Bus::store(uint64_t addr, uint64_t data, size_t sz) {
    dr.store(addr, data, sz);
}
