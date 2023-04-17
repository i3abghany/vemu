#include <util.h>

static elfio reader;

FileInfo read_elf(const std::string& fname, bool exit_fatally)
{
    if (!reader.load(fname.c_str())) {
        std::cout << "Can't find or process ELF file: " << fname << std::endl;
        if (exit_fatally)
            exit(EXIT_FAILURE);
        else
            return { {}, (uint64_t)~0 };
    }
    std::vector<MemorySegment> segments;
    for (int i = 0; i < reader.segments.size(); i++) {
        const segment* pseg = reader.segments[i];
        if (pseg->get_type() != PT_LOAD)
            continue;
        const uint8_t* p = (const uint8_t*)reader.segments[i]->get_data();
        auto seg_perms = (BytePermission)pseg->get_flags();
        segments.push_back({ seg_perms,
                             pseg->get_virtual_address(),
                             pseg->get_memory_size(),
                             pseg->get_file_size(),
                             p });
    }
    return { segments, reader.get_entry() };
}