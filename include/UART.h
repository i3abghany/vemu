#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

#include <Device.h>
#include <defs.h>

class UART : public Device
{
  public:
    UART();
    UART(const UART& other)
    {
        uart_mem = other.uart_mem;
        interrupting = false;
    }

    [[nodiscard]] std::pair<uint64_t, ReturnException> load(uint64_t,
                                                            size_t) override;
    ReturnException store(uint64_t, uint64_t, size_t) override;

    [[nodiscard]] uint64_t get_base() const override { return UART_BASE; }

    [[nodiscard]] uint64_t get_size() const override { return UART_SIZE; }

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
