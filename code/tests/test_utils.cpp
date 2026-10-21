#include "../include/utils.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// In ket qua kiem thu va cap nhat so luong PASS/FAIL.
void checkResult(
    bool condition,
    const std::string& name,
    int& passed,
    int& failed
) {
    if (condition) {
        ++passed;
        std::cout << name << ": PASS\n";
    } else {
        ++failed;
        std::cout << name << ": FAIL\n";
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    std::cout << "========== UTILS TEST ==========\n";

    std::cout << "\n========== SUMMARY ==========\n";
    std::cout << "Total : " << passed + failed << '\n';
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
