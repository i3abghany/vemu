#pragma once

#include <cstdint>
#include <map>
#include <thread>

#include <VEmu.h>

class Tester {
public:
    static void run();
private:
    const static std::vector<std::string> test_fnames;
};
