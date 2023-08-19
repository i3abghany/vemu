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
    auto* input_info = corpus.get_random_free_file();
    MutationHistory hist;
    push_mutate(input_info, gen_rand() % 16, hist);
    char** fuzzed_cmd_args = &argv[3];
    VEmu em = VEmu { fuzz_info,
                     substitute_input(fuzzed_cmd_args, argc - 3,
                                      input_info->file_name.c_str()) };
    pop_mutate(input_info, hist);
    auto exit_status = em.run();
    std::cout << exit_status;
#else
#error "General emulation still WIP."
#endif
}
