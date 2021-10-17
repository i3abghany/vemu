#pragma once

#include <cstdint>
#include <utility>
#include <atomic>
#include <thread>
#include <mutex>
#include <array>
#include <iostream>

#include <defs.h>
#include <Device.h>

class UART : public Device {
public:
    UART();
    
    [[nodiscard]] std::pair<uint64_t, ReturnException> load(uint64_t, size_t) override;
    ReturnException store(uint64_t, uint64_t, size_t) override;

    [[nodiscard]] uint64_t get_base() const override
    {
        return UART_BASE;
    }

    [[nodiscard]] uint64_t get_size() const override
    {
        return UART_SIZE;
    }

    [[nodiscard]] bool is_interrupting() override
    {
        return interrupting.exchange(false, std::memory_order_acquire);
    }

private:
    uint64_t load8(uint64_t);
    void store8(uint64_t, uint64_t);

    std::array<uint8_t, UART_SIZE> uart_mem;
    std::mutex mu;
    std::atomic<bool> interrupting;
};
