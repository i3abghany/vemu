#pragma once

#include <utility>
#include <algorithm>

#include <MMU.h>
#include <PLIC.h>
#include <CLINT.h>
#include <UART.h>

class Bus {
public:
    Bus(uint64_t mem_size) : ram_size(mem_size)
    {
        devices = std::vector<Device *>{ new CLINT(), new PLIC(), new UART(), new MMU(ram_size) };
    }

    ~Bus()
    {
        for (Device *device : devices)
            delete device;
    }

    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException  store(uint64_t, uint64_t, size_t);
    bool uart_is_interrupting() { return get_uart()->is_interrupting(); }

private:
    std::vector<Device *> devices;
    Device *get_uart()
    {
        /* We have to ensure that we have only one UART device... */
        return *std::find_if(std::begin(devices), std::end(devices), [](Device *device) {
            return dynamic_cast<UART *>(device) != nullptr;
        });
    }

    Device *get_mmu()
    {
        return *std::find_if(std::begin(devices), std::end(devices), [](Device *device) {
            return dynamic_cast<MMU *>(device) != nullptr;
        });
    }

    uint64_t ram_size;
};
