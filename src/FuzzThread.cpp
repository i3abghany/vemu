#include "FuzzThread.h"


FuzzThread::FuzzThread(Corpus* _corpus, FileInfo* _target, const char** _fuzz_opts, int _n_opts)
        : corpus(_corpus)
        , target(_target)
        , fuzz_opts(_fuzz_opts)
        , n_opts(_n_opts)
    {
    }



void FuzzThread::dispatch(uint64_t runs)
{
    while (runs--) {
        MutationHistory hist;
        auto input_info = corpus->get_random_free_file();
        push_mutate(target, gen_rand() % 8, hist);
        VEmu em = VEmu { target,
                         substitute_input(fuzz_opts, n_opts,
                                          input_info->file_name.c_str()) };
        pop_mutate(input_info, hist);
        auto exit_status = em.run();
        (void)exit_status;
    }
}
