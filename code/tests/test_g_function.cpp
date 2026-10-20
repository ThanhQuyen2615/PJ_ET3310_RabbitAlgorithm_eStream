
#include "../include/utils.h"

#include <cstdint>
#include <iostream>

int main() {
    int passed = 0;
    int failed = 0;

    // Test 1: u = 0, v = 0 -> ket qua 0
    {
        uint32_t actual = Utils::gFunc(0, 0);
        uint32_t expected = 0;

        if (actual == expected) {
            std::cout << "Test 1 (0, 0): PASS\n";
            ++passed;
        } else {
            std::cout << "Test 1 (0, 0): FAIL"
                      << " | Expected: " << expected
                      << " | Actual: " << actual << '\n';
            ++failed;
        }
    }

    std::cout << "\nSummary: "
              << passed << " PASS, "
              << failed << " FAIL\n";

    return failed == 0 ? 0 : 1;
}
