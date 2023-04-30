# vemu

A toy emulator for the RISC-V ISA.  
Currently supports: RV64IMA full user-space emulation.
Partially supports: The F extension, the previliged ISA specification, and the
Newlib Linux syscall subset.

The emulator uses the Berkeley SoftFloat (release 3e) to carry out floating
point operations. The source of the library is included in the `vendor`
directory. The static library is prebuilt for only Windows for now.

## Building

vemu uses CMake, I prefer to use Ninja as a generator but a Makefile can be
generated by omitting the `-G Ninja` part from invoking CMake.

The fuzz target expects a corpus path along with the command line of the program
to fuzz.

```
mkdir build && cd build
cmake ../ -G Ninja
ninja fuzz_emu
./fuzz_emu /path/to/binary/corpus TARGET [OPTIONS...]
```

The fuzzing input can be indicated by the `{}` option. As an example, to fuzz
objdump, a possible command line for vemu is as follows.

```
./fuzz_emu ./input_corpus/ objdump -x {}
```

## Testing

A subset of the RISC-V test suite is used to test vemu, a testing target can be
built and run against the available tests using the following commands, after
generating the ninja build files:

```
ninja test_emu
./test_emu
```

The test suite is organized in a manner that in each test, an `ECALL` 
instruction is invoked whether the test passed or not. A pass/fail value is set
in a register that we could check to see whether all tests have passed or not.
The testing target interposes `ECALL` instructions to do so.
