#pragma once

#include<cstdint>
#include <cstddef>
#include <utility>

#include <defs.h>
#include <Device.h>

class CLINT : public Device {
public:
    CLINT();

    std::pair<uint64_t, ReturnException> load(uint64_t, size_t) override;
    ReturnException store(uint64_t, uint64_t, size_t) override;

private:
    uint64_t load64(uint64_t addr);
    void store64(uint64_t addr, uint64_t data);

    uint64_t mtime;
    uint64_t mtimecmp;
};
