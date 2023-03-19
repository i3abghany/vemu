#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include <Device.h>
#include <defs.h>

class CLINT : public Device
{
  public:
    CLINT();

    [[nodiscard]] std::pair<uint64_t, ReturnException> load(uint64_t,
                                                            size_t) override;
    ReturnException store(uint64_t, uint64_t, size_t) override;

    [[nodiscard]] uint64_t get_base() const override { return CLINT_BASE; }

    [[nodiscard]] uint64_t get_size() const override { return CLINT_SIZE; }

    [[nodiscard]] bool is_interrupting() override { return false; }

  private:
    [[nodiscard]] uint64_t load64(uint64_t addr) const;
    void store64(uint64_t addr, uint64_t data);

    uint64_t mtime;
    uint64_t mtimecmp;
};
