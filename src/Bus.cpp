#include <Bus.h>

std::pair<uint64_t, ReturnException> Bus::load(uint64_t addr, size_t sz)
{
    for (Device *device : devices) {
        auto base = device->get_base();
        auto size = device->get_size();

        if (addr >= base && addr < base + size) {
            return device->load(addr, sz);
        }
    }

    return {0, ReturnException::LoadAccessFault};
}

ReturnException Bus::store(uint64_t addr, uint64_t data, size_t sz)
{
    for (Device *device : devices) {
        auto base = device->get_base();
        auto size = device->get_size();

        if (addr >= base && addr < base + size) {
            return device->store(addr, data, sz);
        }
    }

    return ReturnException::LoadAccessFault;
}
