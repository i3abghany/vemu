#pragma once

#include <algorithm>
#include <utility>

#include <CLINT.h>
#include <MMU.h>
#include <PLIC.h>
#include <UART.h>

class Bus
{
  public:
    Bus(uint64_t mem_size)
      : ram_size(mem_size)
    {
        devices = std::vector<Device*>{ new CLINT(), new PLIC(), new UART() };
        mmu = new MMU(ram_size);
    }

    Bus(const Bus& other)
    {
        devices = std::vector<Device*>{
            new CLINT(*dynamic_cast<CLINT*>(other.devices[0])),
            new PLIC(*dynamic_cast<PLIC*>(other.devices[1])),
            new UART(*dynamic_cast<UART*>(other.devices[2]))
        };
        mmu = new MMU(*other.mmu);
    }

    ~Bus()
    {
        for (Device* device : devices)
            delete device;
        delete mmu;
    }

    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException store(uint64_t, uint64_t, size_t);
    bool uart_is_interrupting() { return get_uart()->is_interrupting(); }

    MMU* get_mmu() const { return mmu; }

  private:
    std::vector<Device*> devices;
    Device* get_uart() const
    {
        return *std::find_if(
          std::begin(devices), std::end(devices), [](Device* device) {
              return dynamic_cast<UART*>(device) != nullptr;
          });
    }

    uint64_t ram_size;
    MMU* mmu;
};
