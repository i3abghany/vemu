#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wshadow"
#include <elfio/elfio.hpp>
#pragma GCC diagnostic pop
using namespace ELFIO;

#include <map>
#include <mutex>

typedef uint8_t BytePermission;
using MutationHistory = std::map<std::pair<uint64_t, uint64_t>, uint8_t>;

struct MemorySegment {
    BytePermission perms;
    uint64_t start_addr;
    uint64_t mem_size;
    uint64_t file_size;
    uint8_t* data;
};

struct FileInfo {
    FileInfo(std::string fname, std::vector<MemorySegment> segs, uint64_t e)
        : file_name(std::move(fname))
        , segments(std::move(segs))
        , entry_point(e)
    {
    }
    std::string file_name;
    std::vector<MemorySegment> segments;
    uint64_t entry_point;
    std::mutex lock;
};

std::vector<char*> substitute_input(const char** args, size_t len, const char* input_name);
FileInfo* read_elf(const std::string& fname, bool exit_fatally = true);
uint64_t gen_rand();

class Corpus {
public:
    explicit Corpus(const std::string& dir_path);
    FileInfo* get_random_free_file();

private:
    std::vector<FileInfo*> corpus_files;
};

void push_mutate(FileInfo* info, uint64_t n_bytes, MutationHistory& history);
void pop_mutate(FileInfo* info, const MutationHistory& history);
