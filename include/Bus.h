#pragma once

#include <algorithm>
#include <utility>

#include <CLINT.h>
#include <MMU.h>
#include <PLIC.h>
#include <UART.h>

class Bus {
public:
    Bus(uint64_t mem_size)
        : ram_size(mem_size)
    {
#if !defined(FUZZ_ENV) && !defined(TEST_ENV)
        devices = std::vector<Device*> { new CLINT(), new PLIC(), new UART() };
#endif
        mmu = new MMU(ram_size);
    }

    Bus(const Bus& other)
    {
#if !defined(FUZZ_ENV) && !defined(TEST_ENV)
        devices
            = std::vector<Device*> { new CLINT(*dynamic_cast<CLINT*>(other.devices[0])),
                                     new PLIC(*dynamic_cast<PLIC*>(other.devices[1])),
                                     new UART(*dynamic_cast<UART*>(other.devices[2])) };
#endif
        mmu = new MMU(*other.mmu);
    }

    ~Bus()
    {
#if !defined(FUZZ_ENV) && !defined(TEST_ENV)
        for (Device* device : devices)
            delete device;
#endif
        delete mmu;
    }

    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException store(uint64_t, uint64_t, size_t);
    bool uart_is_interrupting()
    {
#if !defined(FUZZ_ENV) && !defined(TEST_ENV)
        return get_uart()->is_interrupting();
#else
        return false;
#endif
    }

    MMU* get_mmu() const { return mmu; }

private:
    std::vector<Device*> devices;
    Device* get_uart() const
    {
#if !defined(FUZZ_ENV) && !defined(TEST_ENV)
        return *std::find_if(std::begin(devices), std::end(devices), [](Device* device) {
            return dynamic_cast<UART*>(device) != nullptr;
        });
#else
        return nullptr;
#endif
    }

    uint64_t ram_size;
    MMU* mmu;
};
