#include <iostream>
#include <string>

#include <util.h>
#include <FuzzThread.h>
#ifndef TEST_ENV
#include <VEmu.h>
#else
#include <Tester.h>
#endif

int main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;
#ifdef TEST_ENV
    Tester::run();
#elif defined(FUZZ_ENV)
    if (argc < 4) {
        std::cout << "Usage: " << argv[0]
                  << " path/to/corpus/directory TARGET [OPTIONS...]";
        exit(EXIT_FAILURE);
    }

    auto corpus = Corpus(argv[1]);
    auto* fuzz_info = read_elf(argv[2]);
    const char** fuzzed_cmd_args = &argv[3];
    FuzzThread th1 { &corpus, fuzz_info, fuzzed_cmd_args, argc - 3 };
    th1.dispatch(100);
#else
#error "General emulation still WIP."
#endif
}
