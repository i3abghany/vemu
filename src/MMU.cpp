#include <MMU.h>
#include <cassert>
#include <execution>

MMU::MMU(uint64_t mem_size)
  : ram_size(mem_size)
{
    ram.reserve(ram_size);
    byte_permission.reserve(ram_size);
}

void MMU::set_perms(uint64_t addr, uint64_t size, BytePermission perm)
{
    assert(addr + size < ram_size);
    std::fill(std::execution::par,
              byte_permission.begin() + addr,
              byte_permission.begin() + addr + size,
              perm);
}

uint64_t MMU::allocate(uint64_t size)
{
    uint64_t base = alloc_ptr;
    assert(size + base < ram_size);
    alloc_ptr += size;
    set_perms(base, size, PERM_RAW | PERM_WRITE);
    return base;
}

void MMU::reset_to(const MMU& other)
{
    for (auto blk : dirty_blocks) {
        uint64_t start_addr = blk * BLOCK_SIZE;
        for (uint64_t i = start_addr; i < start_addr + BLOCK_SIZE; i++) {
            ram[i] = other.ram[i];
        }
    }
    dirty_blocks.clear();
}

void MMU::load_file(FileInfo info)
{
    for (auto seg : info.segments) {
        set_perms(seg.start_addr, seg.mem_size, PERM_WRITE);
        std::vector<uint8_t> data;
        data.assign((uint8_t*)seg.data, (uint8_t*)seg.data + seg.file_size);
        if (data.size() < seg.mem_size) {
            data.resize(seg.mem_size, 0);
        }
        if (data.size() > seg.mem_size)
            data =
              std::vector<uint8_t>{ data.begin(), data.begin() + seg.mem_size };
        write_from(data, seg.start_addr);
        set_perms(seg.start_addr, seg.mem_size, seg.perms);
        alloc_ptr = std::max(
          alloc_ptr, ((seg.start_addr + seg.mem_size) + 0xFFFF) & ~0xFFFF);
    }
}

void MMU::write_from(const std::vector<uint8_t>& buf, uint64_t start_addr)
{
    assert(start_addr + buf.size() < ram_size);
    bool can_write_all = true;
    for (uint64_t i = start_addr; i < start_addr + buf.size(); i++) {
        can_write_all &= ((byte_permission[i] & PERM_WRITE) != 0);
    }

    // TODO: Handle memory write errors gracefully.
    assert(can_write_all);

    for (uint64_t i = start_addr; i < start_addr + buf.size(); i++) {
        ram[i] = buf[i - start_addr];
        if ((byte_permission[i] & PERM_RAW) != 0) {
            byte_permission[i] |= PERM_READ;
            byte_permission[i] &= ~PERM_RAW;
        }
    }
}

std::vector<uint8_t> MMU::read_to(uint64_t start_addr, uint64_t len) const
{
    assert(start_addr + len < ram_size);
    std::vector<uint8_t> ret(len);

    bool can_read_all = true;
    bool has_raw = false;
    for (uint64_t i = start_addr; i < start_addr + len; i++) {
        can_read_all &= ((byte_permission[i] & PERM_READ) != 0);
        has_raw |= (byte_permission[i] & PERM_RAW) != 0;
    }

    // TODO: Handle read permission errors gracefully.
    // assert(can_read_all);
    // assert(!has_raw);
    if (!can_read_all) {
        std::cout << "WARNING: Reading memory without read permission @ 0x"
                  << std::hex << start_addr << ", len: 0x" << len << std::endl;
        std::cout << std::dec;
    }

    if (has_raw) {
        std::cout << "WARNING: Reading uninitialized memory @ 0x" << std::hex
                  << start_addr << ", len: 0x" << len << std::endl;
        std::cout << std::dec;
    }

    for (uint64_t i = start_addr; i < start_addr + len; i++) {
        ret[i - start_addr] = ram[i];
    }
    return ret;
}

std::pair<uint64_t, ReturnException> MMU::load(uint64_t addr, size_t sz)
{
    std::pair<uint64_t, ReturnException> res{
        0x00000000, ReturnException::NormalExecutionReturn
    };

    switch (sz) {
        case 8:
            res.first = load_byte(addr);
            break;
        case 16:
            res.first = load_hword(addr);
            break;
        case 32:
            res.first = load_word(addr);
            break;
        case 64:
            res.first = load_dword(addr);
            break;
        default:
            assert(false);
    }

    return res;
}

ReturnException MMU::store(uint64_t addr, uint64_t data, size_t sz)
{
    switch (sz) {
        case 8:
            store_byte(addr, data);
            break;
        case 16:
            store_hword(addr, data);
            break;
        case 32:
            store_word(addr, data);
            break;
        case 64:
            store_dword(addr, data);
            break;
        default:
            assert(false);
    }

    uint64_t block_idx = addr / BLOCK_SIZE;
    dirty_blocks.insert(block_idx);

    return ReturnException::NormalExecutionReturn;
}

uint64_t MMU::load_byte(uint64_t addr) const
{
    uint64_t res = 0x00000000;

    std::vector<uint8_t> read_data = read_to(addr, 1);
    res |= static_cast<uint64_t>(read_data[0]);

    return res;
}

uint64_t MMU::load_hword(uint64_t addr) const
{
    uint64_t res = 0x00000000;

    std::vector<uint8_t> read_data = read_to(addr, 2);
    res |= static_cast<uint64_t>(read_data[0]);
    res |= static_cast<uint64_t>(read_data[1]) << 8;

    return res;
}

uint64_t MMU::load_word(uint64_t addr) const
{
    uint64_t res = 0x00000000;

    std::vector<uint8_t> read_data = read_to(addr, 4);
    res |= static_cast<uint64_t>(read_data[0]);
    res |= static_cast<uint64_t>(read_data[1]) << 8;
    res |= static_cast<uint64_t>(read_data[2]) << 16;
    res |= static_cast<uint64_t>(read_data[3]) << 24;
    return res;
}

uint64_t MMU::load_dword(uint64_t addr) const
{
    uint64_t res = 0x00000000;

    std::vector<uint8_t> read_data = read_to(addr, 8);
    res |= static_cast<uint64_t>(read_data[0]);
    res |= static_cast<uint64_t>(read_data[1]) << 8;
    res |= static_cast<uint64_t>(read_data[2]) << 16;
    res |= static_cast<uint64_t>(read_data[3]) << 24;
    res |= static_cast<uint64_t>(read_data[4]) << 32;
    res |= static_cast<uint64_t>(read_data[5]) << 40;
    res |= static_cast<uint64_t>(read_data[6]) << 48;
    res |= static_cast<uint64_t>(read_data[7]) << 56;

    return res;
}

[[nodiscard]] std::string MMU::_read_null_terminated_string(uint64_t addr) const
{
    std::string str;
    uint8_t read_char = ' ';
    while (read_char != 0) {
        read_char = read_to(addr++, 1)[0];
        str += read_char;
    }
    return str;
}

void MMU::store_byte(uint64_t addr, uint64_t data)
{
    std::vector<uint8_t> data_to_store(1);
    data_to_store[0] = static_cast<uint8_t>(data);
    write_from(data_to_store, addr);
}

void MMU::store_hword(uint64_t addr, uint64_t data)
{
    std::vector<uint8_t> data_to_store(2);
    data_to_store[0] = static_cast<uint8_t>(data) & 0xFF;
    data_to_store[1] = static_cast<uint8_t>((data >> 8) & 0xFF);
    write_from(data_to_store, addr);
}

void MMU::store_word(uint64_t addr, uint64_t data)
{
    std::vector<uint8_t> data_to_store(4);
    data_to_store[0] = static_cast<uint8_t>(data) & 0xFF;
    data_to_store[1] = static_cast<uint8_t>((data >> 8) & 0xFF);
    data_to_store[2] = static_cast<uint8_t>((data >> 16) & 0xFF);
    data_to_store[3] = static_cast<uint8_t>((data >> 24) & 0xFF);
    write_from(data_to_store, addr);
}

void MMU::store_dword(uint64_t addr, uint64_t data)
{
    std::vector<uint8_t> data_to_store(8);
    data_to_store[0] = static_cast<uint8_t>(data) & 0xFF;
    data_to_store[1] = static_cast<uint8_t>((data >> 8) & 0xFF);
    data_to_store[2] = static_cast<uint8_t>((data >> 16) & 0xFF);
    data_to_store[3] = static_cast<uint8_t>((data >> 24) & 0xFF);
    data_to_store[4] = static_cast<uint8_t>((data >> 32) & 0xFF);
    data_to_store[5] = static_cast<uint8_t>((data >> 40) & 0xFF);
    data_to_store[6] = static_cast<uint8_t>((data >> 48) & 0xFF);
    data_to_store[7] = static_cast<uint8_t>((data >> 56) & 0xFF);
    write_from(data_to_store, addr);
}
