#pragma once

#include <cstdint>
#include <utility>

#include <defs.h>

class Device
{
  public:
    virtual std::pair<uint64_t, ReturnException> load(uint64_t, size_t) = 0;
    virtual ReturnException store(uint64_t, uint64_t, size_t) = 0;
    [[nodiscard]] virtual uint64_t get_base() const = 0;
    [[nodiscard]] virtual uint64_t get_size() const = 0;

    virtual bool is_interrupting() = 0;
    virtual ~Device() = default;
};
