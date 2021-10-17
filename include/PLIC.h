#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include <defs.h>
#include <Device.h>

class PLIC : public Device {
public:   
    PLIC();

    std::pair<uint64_t, ReturnException> load(uint64_t addr, size_t sz) override;
    ReturnException store(uint64_t addr, uint64_t value, size_t sz) override;

private:
    uint64_t load64(uint64_t);
    void store64(uint64_t, uint64_t);

    uint64_t pending;
    uint64_t senable;
    uint64_t spriority;
    uint64_t sclaim;
};
