#pragma once

#include <utility>

#include <DRAM.h>
#include <PLIC.h>
#include <CLINT.h>
#include <UART.h>

class Bus {
public:
    Bus() = default;
    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException  store(uint64_t, uint64_t, size_t);
    bool uart_is_interrupting() { return uart.is_interrupting(); }

private:
    DRAM dr {};
    CLINT clint {};
    PLIC plic {};
    UART uart {};
};
