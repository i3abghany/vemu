#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include <Device.h>
#include <defs.h>

class PLIC : public Device
{
  public:
    PLIC();

    [[nodiscard]] std::pair<uint64_t, ReturnException> load(uint64_t addr,
                                                            size_t sz) override;
    ReturnException store(uint64_t addr, uint64_t value, size_t sz) override;

    [[nodiscard]] uint64_t get_base() const override { return PLIC_BASE; }

    [[nodiscard]] uint64_t get_size() const override { return PLIC_SIZE; }

    [[nodiscard]] bool is_interrupting() override { return false; }

  private:
    [[nodiscard]] uint64_t load64(uint64_t) const;
    void store64(uint64_t, uint64_t);

    uint64_t pending;
    uint64_t senable;
    uint64_t spriority;
    uint64_t sclaim;
};
