#include <iostream>
#include <string>

#ifndef TEST_ENV
#include <VEmu.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wshadow"
#include <elfio/elfio.hpp>
#pragma GCC diagnostic pop
using namespace ELFIO;
#else
#include <Tester.h>
#endif

int main(int argc, char* argv[])
{
#ifndef TEST_ENV
    if (argc < 2) {
        std::cout << "Provide a program.\n";
        return 1;
    }
    elfio reader;
    if (!reader.load(argv[1])) {
        std::cout << "Can't find or process ELF file " << argv[1] << std::endl;
        exit(1);
    }
    std::vector<MemorySegment> segments;
    auto seg_num = reader.segments.size();
    for (int i = 0; i < seg_num; ++i) {
        const segment* pseg = reader.segments[i];
        BytePermission seg_perms = (uint8_t)pseg->get_flags();
        const uint8_t* p = (const uint8_t*)reader.segments[i]->get_data();
        segments.push_back({ seg_perms,
                             pseg->get_virtual_address(),
                             pseg->get_memory_size(),
                             pseg->get_file_size(),
                             p });
    }

    FileInfo info{ segments, reader.get_entry() };
    VEmu em = VEmu{ argv[1], info, argv[2] };
    em.run();
    em.dump_regs();
#else
    (void)argc;
    (void)argv;
    Tester::run();

#endif
    return 0;
}
