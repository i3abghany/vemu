#include <iostream>
#include <cstdint>

int main() {
    int64_t a1 = 0x8000000000000000;
    int64_t a2 = 0xffffffffffffffff;
//    int64_t a3 = a1 / a2;
    int64_t a4 = a2 / a1;
}
