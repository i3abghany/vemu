#pragma once

#include <cstdint>
#include <utility>
#include <atomic>
#include <thread>
#include <mutex>
#include <array>
#include <iostream>

#include <defs.h>

class UART {
public:
    UART();
    
    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException store(uint64_t, uint64_t, size_t);

private:
    uint64_t load8(uint64_t);
    void store8(uint64_t, uint64_t);

    std::array<uint8_t, UART_SIZE> uart_mem;
    std::mutex mu;
};
