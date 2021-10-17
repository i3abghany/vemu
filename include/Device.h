#pragma once

#include <cstdint>
#include <utility>

#include <defs.h>

class Device {
public:
    virtual std::pair<uint64_t, ReturnException> load(uint64_t, size_t) = 0;
    virtual ReturnException store(uint64_t, uint64_t, size_t) = 0;
};

