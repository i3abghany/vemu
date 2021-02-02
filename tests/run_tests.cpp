#include <iostream>
#include "decode_type_test.h"

int main() {
    if (decode_type_test() != 0) {
        std::cout << "ERROR: decode_type_test() returned " << decode_type_test();
        return 1;
    }

    std::cout << "Tests ran successfully\n";
    return 0;
}