#include <filesystem>
#include <map>
#include <util.h>

static elfio reader;

FileInfo* read_elf(const std::string& fname, bool exit_fatally)
{
    if (!reader.load(fname.c_str())) {
        std::cout << "Can't find or process ELF file: " << fname << std::endl;
        if (exit_fatally)
            exit(EXIT_FAILURE);
        else
            return new FileInfo { "", {}, (uint64_t)~0 };
    }
    std::vector<MemorySegment> segments;
    for (const auto& pseg : reader.segments) {
        if (pseg->get_type() != PT_LOAD)
            continue;
        auto* p = (uint8_t*)pseg->get_data();
        uint8_t* dat = new uint8_t[pseg->get_file_size()];
        memcpy(dat, p, pseg->get_file_size());
        auto seg_perms = (BytePermission)pseg->get_flags();
        segments.push_back(MemorySegment { seg_perms, pseg->get_virtual_address(),
                                           pseg->get_memory_size(), pseg->get_file_size(),
                                           dat });
    }
    return new FileInfo { fname, segments, reader.get_entry() };
}

static uint64_t seed = 123456789;
uint64_t gen_rand()
{
    uint64_t x = seed;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    seed = x;
    return x * 0x2545F4914F6CDD1Dull;
}

Corpus::Corpus(const std::string& dir_path)
{
    for (const auto& entry : std::filesystem::directory_iterator(dir_path))
        corpus_files.push_back(read_elf(entry.path(), true));
}

FileInfo* Corpus::get_random_free_file()
{
    while (true) {
        auto idx = gen_rand() % corpus_files.size();
        auto& f = corpus_files[idx];
        if (f->lock.try_lock())
            return f;
    }
}

using MutationHistory = std::map<std::pair<uint64_t, uint64_t>, uint8_t>;

void push_mutate(FileInfo* info, uint64_t n_bytes, MutationHistory& history)
{
    while (n_bytes--) {
        uint64_t seg_idx = gen_rand() % info->segments.size();
        uint64_t byte_idx = gen_rand() % info->segments[seg_idx].file_size;
        if (history.find({ seg_idx, byte_idx }) == std::end(history)) {
            history[{ seg_idx, byte_idx }] = info->segments[seg_idx].data[byte_idx];
        }

        info->segments[seg_idx].data[byte_idx] = (uint8_t)(gen_rand() % 255);
    }
}

void pop_mutate(FileInfo* info, const MutationHistory& history)
{
    for (auto it = std::begin(history); it != std::end(history); it++) {
        auto pos = (*it).first;
        auto byte_val = (*it).second;
        info->segments[pos.first].data[pos.second] = byte_val;
    }
}
