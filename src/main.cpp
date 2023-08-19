#include <iostream>
#include <string>

#include <util.h>
#ifndef TEST_ENV
#include <VEmu.h>
#else
#include <Tester.h>
#endif

std::vector<char*> substitute_input(char** args, size_t len, const char* input_name)
{
    std::vector<char*> new_args(len);
    for (size_t i = 0; i < len; i++) {
        if (strcmp(args[i], "{}") == 0) {
            new_args[i] = new char[strlen(input_name) + 1];
            strcpy(new_args[i], input_name);
        } else
            new_args[i] = args[i];
    }

    return new_args;
}

int main(int argc, char* argv[])
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
    mutate(input_info, gen_rand() % 16);
    char** fuzzed_cmd_args = &argv[3];
    VEmu em = VEmu { fuzz_info,
                     substitute_input(fuzzed_cmd_args, argc - 3,
                                      input_info->file_name.c_str()) };
    auto exit_status = em.run();
    std::cout << exit_status;
#else
#error "General emulation still WIP."
#endif
}
