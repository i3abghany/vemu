#include <iostream>
#include <string>

#ifndef TEST_ENV
#include <VEmu.h>
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
    VEmu em{ std::string{ argv[1] } };
    em.run();
    em.dump_regs();
#else
    (void)argc;
    (void)argv;
    Tester::run();

#endif
    return 0;
}
