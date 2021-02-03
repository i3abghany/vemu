#include <iostream>
#include "decode_type_test.h"
#include "decode_wrong_test.h"

int main() {
    if (decode_type_test() != 0) {
        std::cout << "ERROR: decode_type_test() returned " << decode_type_test();
        return 1;
    }

    if (decode_wrong_test() != 0) {
        std::cout << "ERROR: decode_wrong_test() returned " << decode_wrong_test();
        return 1;
    }

    std::cout << "Tests ran successfully\n";
    return 0;
}