#include <Bus.h>

std::pair<uint64_t, ReturnException> Bus::load(uint64_t addr, size_t sz) {
    if (addr >= UART_BASE && addr < UART_BASE + UART_SIZE) {
        return uart.load(addr, sz);
    } else if (addr >= CLINT_BASE && addr < CLINT_BASE + CLINT_SIZE) {
        return clint.load(addr, sz);
    } else if (addr >= PLIC_BASE && addr < PLIC_BASE + PLIC_SIZE) {
        return plic.load(addr, sz);
    } else if (addr >= ADDR_BASE) {
        return dr.load(addr, sz);
    }

    return {0, ReturnException::LoadAccessFault};
}

ReturnException Bus::store(uint64_t addr, uint64_t data, size_t sz) {
    if (addr >= UART_BASE && addr < UART_BASE + UART_SIZE) {
        return uart.store(addr, data, sz);
    } else if (addr >= CLINT_BASE && addr < CLINT_BASE + CLINT_SIZE) {
        return clint.store(addr, data, sz);
    } else if (addr >= PLIC_BASE && addr < PLIC_BASE + PLIC_SIZE) {
        return plic.store(addr, data, sz);
    } else if (addr >= ADDR_BASE) {
        return dr.store(addr, data, sz);
    }

    return ReturnException::LoadAccessFault;
}
