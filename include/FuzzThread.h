#pragma once

#include "VEmu.h"
#include "util.h"
#include <cstdint>

struct stats {
    uint64_t runs;
    uint64_t crashes;
};

class FuzzThread {
public:
    FuzzThread(Corpus* _corpus, FileInfo* _target, const char** _fuzz_opts, int _n_opts);

    void dispatch(uint64_t runs);

private:
    VEmu emulator;
    Corpus* corpus;
    FileInfo* target;
    const char** fuzz_opts;
    int n_opts;
};
