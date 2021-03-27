#pragma once

#include <cstdint>
#include <map>

#include "VEmu.h"

class Tester {
public:
    static void run();
private:
    const static std::map<std::string, uint64_t> fname_passing_pc;
};
