#include <Bus.h>

std::pair<uint64_t, ReturnException> Bus::load(uint64_t addr, size_t sz)
{
    for (Device* device : devices) {
        auto base = device->get_base();
        auto size = device->get_size();

        if (addr >= base && addr < base + size) {
            return device->load(addr, sz);
        }
    }
    return get_mmu()->load(addr, sz);
}

ReturnException Bus::store(uint64_t addr, uint64_t data, size_t sz)
{
    for (Device* device : devices) {
        auto base = device->get_base();
        auto size = device->get_size();

        if (addr >= base && addr < base + size) {
            return device->store(addr, data, sz);
        }
    }

    get_mmu()->store(addr, data, sz);
    return ReturnException::NormalExecutionReturn;
}
