#include <iostream>
#include "decode_type_test.h"
#include "decode_wrong_test.h"
#include "decode_r_test.h"
#include "decode_several_instructions_test.h"

int main() {
    if (decode_type_test() != 0) {
        std::cout << "ERROR: decode_type_test() returned " << decode_type_test();
        return 1;
    }

    if (decode_wrong_test() != 0) {
        std::cout << "ERROR: decode_wrong_test() returned " << decode_wrong_test();
        return 1;
    }

    if (decode_r_test() != 0) {
        std::cout << "ERROR: decode_r_test() returned " << decode_r_test();
        return 1;
    }

    if (decode_several_instructions_test() != 0) {
        std::cout << "ERROR: decode_several_instructions_test() returned " << decode_several_instructions_test();
        return 1;
    }

    std::cout << "Tests ran successfully\n";
    return 0;
}