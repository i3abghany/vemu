#include <algorithm>

#include <Tester.h>
#include <util.h>

void Tester::run()
{
    for (const auto& tcase : test_cases) {
        if (tcase.skip) {
            std::cout << "Skipped test: " << tcase.bin_file_name << std::endl;
            continue;
        }
        std::cout << "Starting test: " << tcase.bin_file_name << "... ";
        FileInfo* info = read_elf(tcase.bin_file_name, false);
        if (info->entry_point == (uint64_t)~0)
            continue;
        VEmu em = VEmu { info, std::vector<char*> {}, 2 * 1024 * 1024 };
        em.run();
    }
}

const std::vector<TestCase> Tester::test_cases
    = { { "../tests/elf/rv64ui-p-add" },   { "../tests/elf/rv64ui-p-addi" },
        { "../tests/elf/rv64ui-p-addiw" }, { "../tests/elf/rv64ui-p-addw" },
        { "../tests/elf/rv64ui-p-and" },   { "../tests/elf/rv64ui-p-andi" },
        { "../tests/elf/rv64ui-p-auipc" }, { "../tests/elf/rv64ui-p-beq" },
        { "../tests/elf/rv64ui-p-bge" },   { "../tests/elf/rv64ui-p-bgeu" },
        { "../tests/elf/rv64ui-p-blt" },   { "../tests/elf/rv64ui-p-bltu" },
        { "../tests/elf/rv64ui-p-bne" },   { "../tests/elf/rv64ui-p-fence_i" },
        { "../tests/elf/rv64ui-p-jal" },   { "../tests/elf/rv64ui-p-jalr" },
        { "../tests/elf/rv64ui-p-lb" },    { "../tests/elf/rv64ui-p-lbu" },
        { "../tests/elf/rv64ui-p-ld" },    { "../tests/elf/rv64ui-p-lh" },
        { "../tests/elf/rv64ui-p-lhu" },   { "../tests/elf/rv64ui-p-lui" },
        { "../tests/elf/rv64ui-p-lw" },    { "../tests/elf/rv64ui-p-lwu" },
        { "../tests/elf/rv64ui-p-or" },    { "../tests/elf/rv64ui-p-ori" },
        { "../tests/elf/rv64ui-p-sb" },    { "../tests/elf/rv64ui-p-sd" },
        { "../tests/elf/rv64ui-p-sh" },    { "../tests/elf/rv64ui-p-sll" },
        { "../tests/elf/rv64ui-p-slli" },  { "../tests/elf/rv64ui-p-slliw" },
        { "../tests/elf/rv64ui-p-sllw" },  { "../tests/elf/rv64ui-p-slt" },
        { "../tests/elf/rv64ui-p-slti" },  { "../tests/elf/rv64ui-p-sltiu" },
        { "../tests/elf/rv64ui-p-sltu" },  { "../tests/elf/rv64ui-p-sra" },
        { "../tests/elf/rv64ui-p-srai" },  { "../tests/elf/rv64ui-p-sraiw" },
        { "../tests/elf/rv64ui-p-sraw" },  { "../tests/elf/rv64ui-p-srl" },
        { "../tests/elf/rv64ui-p-srli" },  { "../tests/elf/rv64ui-p-srliw" },
        { "../tests/elf/rv64ui-p-srlw" },  { "../tests/elf/rv64ui-p-sub" },
        { "../tests/elf/rv64ui-p-subw" },  { "../tests/elf/rv64ui-p-sw" },
        { "../tests/elf/rv64ui-p-xor" },   { "../tests/elf/rv64ui-p-xori" } };
