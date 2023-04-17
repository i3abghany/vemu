#include <iostream>
#include <string>

#include <util.h>
#ifndef TEST_ENV
#include <VEmu.h>
#else
#include <Tester.h>
#endif

int main(int argc, char* argv[])
{
#ifndef TEST_ENV
    if (argc < 3) {
        std::cout << "Usage: " << argv[0]
                  << " </path/to/objdump> </path/to/fuzz/input>\n";
        exit(EXIT_FAILURE);
    }
    auto info = read_elf(argv[1]);
    VEmu em = VEmu{ argv[1], info, argv[2] };
    em.run();
    em.dump_regs();
#else
    (void)argc;
    (void)argv;
    Tester::run();

#endif
}
