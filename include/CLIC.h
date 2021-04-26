#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>

#include "defs.h"

class CLIC {
public:
    CLIC();

    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException store(uint64_t, uint64_t, size_t);

    uint64_t load64(uint64_t addr);
    void store64(uint64_t addr, uint64_t data);
private:
    uint64_t mtime;
    uint64_t mtimecmp;
};
