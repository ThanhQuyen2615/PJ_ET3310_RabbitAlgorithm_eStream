
#include "../include/utils.h"

#include <cstdint>
#include <iostream>
#include <iomanip>

void runTest(
    const char* name,
    uint32_t u,
    uint32_t v,
    uint32_t expected,
    int& passed,
    int& failed
) {
    uint32_t actual = Utils::gFunc(u, v);

    if (actual == expected) {
        std::cout << name << ": PASS\n";
        ++passed;
    } else {
        std::cout << name << ": FAIL"
                  << " | Expected: 0x"
                  << std::hex << std::uppercase << expected
                  << " | Actual: 0x" << actual
                  << std::dec << '\n';
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    runTest("Test 1: zero", 0x00000000u, 0x00000000u,
            0x00000000u, passed, failed);

    runTest("Test 2: sum = 1", 0x00000001u, 0x00000000u,
            0x00000001u, passed, failed);

    runTest("Test 3: sum = 2", 0x00000001u, 0x00000001u,
            0x00000004u, passed, failed);

    runTest("Test 4: sum wraps to zero",
            0xFFFFFFFFu, 0x00000001u,
            0x00000000u, passed, failed);

    runTest("Test 5: sum wraps to one",
            0xFFFFFFFFu, 0x00000002u,
            0x00000001u, passed, failed);
    runTest("Test 6: high and low square halves",
            0xFFFFFFFFu, 0x00000000u,
            0xFFFFFFFFu, passed, failed);

    std::cout << "\nSummary: "
              << passed << " PASS, "
              << failed << " FAIL\n";

    return failed == 0 ? 0 : 1;
}
