#pragma once

#include <iostream>
#include <unordered_set>
#include <vector>

#include <Device.h>
#include <defs.h>
#include <util.h>

static constexpr BytePermission PERM_EXEC = (1 << 0);
static constexpr BytePermission PERM_WRITE = (1 << 1);
static constexpr BytePermission PERM_READ = (1 << 2);
static constexpr BytePermission PERM_RAW = (1 << 3);

static constexpr uint64_t BLOCK_SIZE = 4096;

class MMU : public Device {
public:
    MMU(const MMU& other)
    {
        ram = other.ram;
        byte_permission = other.byte_permission;
        dirty_blocks = other.dirty_blocks;
        alloc_ptr = other.alloc_ptr;
        ram_size = other.ram_size;
    }

    MMU(uint64_t ram_size);
    [[nodiscard]] std::pair<uint64_t, ReturnException> load(uint64_t, size_t) override;
    ReturnException store(uint64_t, uint64_t, size_t) override;
    [[nodiscard]] uint64_t get_base() const override { return 0; }
    [[nodiscard]] uint64_t get_size() const override { return ram_size; }
    [[nodiscard]] bool is_interrupting() override { return false; }
    void write_from(const std::vector<uint8_t>&, uint64_t);
    [[nodiscard]] std::pair<std::vector<uint8_t>, ReturnException>
        read_to(uint64_t, uint64_t) const;
    [[nodiscard]] uint64_t cur_alloc_ptr() const { return alloc_ptr; }
    [[nodiscard]] std::string _read_null_terminated_string(uint64_t) const;
    [[nodiscard]] std::pair<uint32_t, ReturnException> load_insn(uint64_t addr) const;

    void reset_to(const MMU& other);

    uint64_t allocate(uint64_t);
    void set_perms(uint64_t, uint64_t, BytePermission);

    void load_file(FileInfo*);

private:
    [[nodiscard]] std::pair<uint64_t, ReturnException> load_byte(uint64_t) const;
    [[nodiscard]] std::pair<uint64_t, ReturnException> load_hword(uint64_t) const;
    [[nodiscard]] std::pair<uint64_t, ReturnException> load_word(uint64_t) const;
    [[nodiscard]] std::pair<uint64_t, ReturnException> load_dword(uint64_t) const;

    void store_byte(uint64_t, uint64_t);
    void store_hword(uint64_t, uint64_t);
    void store_word(uint64_t, uint64_t);
    void store_dword(uint64_t, uint64_t);

private:
    std::vector<uint8_t> ram;
    std::vector<uint8_t> byte_permission;
    std::unordered_set<uint64_t> dirty_blocks;
    uint64_t ram_size;
    uint64_t alloc_ptr = 0x10000;
};
