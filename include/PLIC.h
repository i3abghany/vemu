#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include <defs.h>

class PLIC {
public:   
    PLIC();

    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException store(uint64_t, uint64_t, size_t);

private:
    uint64_t load64(uint64_t);
    void store64(uint64_t, uint64_t);

    uint64_t pending;
    uint64_t senable;
    uint64_t spriority;
    uint64_t sclaim;
};
