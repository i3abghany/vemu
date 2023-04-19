#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wshadow"
#include <elfio/elfio.hpp>
#pragma GCC diagnostic pop
using namespace ELFIO;

typedef uint8_t BytePermission;

struct MemorySegment {
    BytePermission perms;
    uint64_t start_addr;
    uint64_t mem_size;
    uint64_t file_size;
    const uint8_t* data;
};

struct FileInfo {
    std::vector<MemorySegment> segments;
    uint64_t entry_point;
};

FileInfo read_elf(const std::string& fname, bool exit_fatally = true);