#pragma once

#include <utility>

#include "DRAM.h"
#include "PLIC.h"
#include "CLIC.h"

class Bus {
public:
    Bus();
    std::pair<uint64_t, ReturnException> load(uint64_t, size_t);
    ReturnException  store(uint64_t, uint64_t, size_t);
private:
    DRAM dr;
    CLIC clic;
    PLIC plic;
};
