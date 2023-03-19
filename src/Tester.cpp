#include <algorithm>

#include <Tester.h>

void Tester::run()
{
    for (const auto& tcase : test_cases) {
        if (tcase.skip) {
#ifdef _WIN32
            HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hStdout,
                                    FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif
            std::cout << "Skipped test: " << tcase.bin_file_name << std::endl;
            continue;
        }
        std::cout << "Starting test: " << tcase.bin_file_name << "... ";
        VEmu em = VEmu{ tcase.bin_file_name, 0x80000000, 64 * 1024 * 1024 };
        em.run();
    }
}

const std::vector<TestCase> Tester::test_cases = {
    { "../tests/bins/rv64ua_p_amoadd_d" },
    { "../tests/bins/rv64ua_p_amoadd_w" },
    { "../tests/bins/rv64ua_p_amoand_d" },
    { "../tests/bins/rv64ua_p_amoand_w" },
    { "../tests/bins/rv64ua_p_amomax_d" },
    { "../tests/bins/rv64ua_p_amomaxu_d" },
    { "../tests/bins/rv64ua_p_amomaxu_w" },
    { "../tests/bins/rv64ua_p_amomax_w" },
    { "../tests/bins/rv64ua_p_amomin_d" },
    { "../tests/bins/rv64ua_p_amominu_d" },
    { "../tests/bins/rv64ua_p_amominu_w" },
    { "../tests/bins/rv64ua_p_amomin_w" },
    { "../tests/bins/rv64ua_p_amoor_d" },
    { "../tests/bins/rv64ua_p_amoor_w" },
    { "../tests/bins/rv64ua_p_amoswap_d" },
    { "../tests/bins/rv64ua_p_amoswap_w" },
    { "../tests/bins/rv64ua_p_amoxor_d" },
    { "../tests/bins/rv64ua_p_amoxor_w" },
    { "../tests/bins/rv64ui_p_add" },
    { "../tests/bins/rv64ui_p_addi" },
    { "../tests/bins/rv64ui_p_addiw" },
    { "../tests/bins/rv64ui_p_addw" },
    { "../tests/bins/rv64ui_p_auipc" },
    { "../tests/bins/rv64ui_p_bgeu" },
    { "../tests/bins/rv64ui_p_blt" },
    { "../tests/bins/rv64ui_p_bltu" },
    { "../tests/bins/rv64ui_p_bne" },
    { "../tests/bins/rv64ui_p_fence_i" },
    { "../tests/bins/rv64ui_p_jal" },
    { "../tests/bins/rv64ui_p_jalr" },
    { "../tests/bins/rv64ui_p_lb" },
    { "../tests/bins/rv64ui_p_lbu" },
    { "../tests/bins/rv64ui_p_ld" },
    { "../tests/bins/rv64ui_p_lh" },
    { "../tests/bins/rv64ui_p_lui" },
    { "../tests/bins/rv64ui_p_lw" },
    { "../tests/bins/rv64ui_p_lwu" },
    { "../tests/bins/rv64ui_p_sb" },
    { "../tests/bins/rv64ui_p_sd" },
    { "../tests/bins/rv64ui_p_sh" },
    { "../tests/bins/rv64ui_p_sll" },
    { "../tests/bins/rv64ui_p_slli" },
    { "../tests/bins/rv64ui_p_slliw" },
    { "../tests/bins/rv64ui_p_sllw" },
    { "../tests/bins/rv64ui_p_slt" },
    { "../tests/bins/rv64ui_p_slti" },
    { "../tests/bins/rv64ui_p_sltiu" },
    { "../tests/bins/rv64ui_p_sltu" },
    { "../tests/bins/rv64ui_p_sraiw" },
    { "../tests/bins/rv64ui_p_sraw" },
    { "../tests/bins/rv64ui_p_srli" },
    { "../tests/bins/rv64ui_p_srliw" },
    { "../tests/bins/rv64ui_p_srlw" },
    { "../tests/bins/rv64ui_p_subw" },
    { "../tests/bins/rv64ui_p_sw" },
    { "../tests/bins/rv64ui_p_xor" },
    { "../tests/bins/rv64ui_p_xori" },
    { "../tests/bins/rv64um_p_div" },
    { "../tests/bins/rv64um_p_divu" },
    { "../tests/bins/rv64um_p_divuw" },
    { "../tests/bins/rv64um_p_divw" },
    { "../tests/bins/rv64um_p_mul" },
    { "../tests/bins/rv64um_p_mulh" },
    { "../tests/bins/rv64um_p_mulhsu" },
    { "../tests/bins/rv64um_p_mulhu" },
    { "../tests/bins/rv64um_p_mulw" },
    { "../tests/bins/rv64um_p_rem" },
    { "../tests/bins/rv64um_p_remu" },
    { "../tests/bins/rv64um_p_remuw" },
    { "../tests/bins/rv64um_p_remw" },
    { "../tests/bins/rv64ui_p_and" },
    { "../tests/bins/rv64ui_p_andi" },
    { "../tests/bins/rv64ui_p_beq" },
    { "../tests/bins/rv64ui_p_bge" },
    { "../tests/bins/rv64ui_p_lhu" },
    { "../tests/bins/rv64ui_p_or" },
    { "../tests/bins/rv64ui_p_ori" },
    { "../tests/bins/rv64ui_p_sra" },
    { "../tests/bins/rv64ui_p_srai" },
    { "../tests/bins/rv64ui_p_srl" },
    { "../tests/bins/rv64ui_p_sub" },
    { "../tests/bins/rv64uf_p_fadd", true },
    { "../tests/bins/rv64uf_p_fdiv", true }
};
