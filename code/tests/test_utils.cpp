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

    checkResult(
        Utils::rotl32(0x80000000u, 1) == 0x00000001u,
        "rotl32: quay bit cao nhat sang bit thap nhat",
        passed,
        failed
    );

    checkResult(
        Utils::rotl32(0x00000001u, 31) == 0x80000000u,
        "rotl32: quay 31 bit",
        passed,
        failed
    );

    checkResult(
        Utils::rotl32(0x12345678u, 8) == 0x34567812u,
        "rotl32: quay 8 bit",
        passed,
        failed
    );

    checkResult(
        Utils::rotl32(0x12345678u, 16) == 0x56781234u,
        "rotl32: quay 16 bit",
        passed,
        failed
    );

    std::cout << "\n========== SUMMARY ==========\n";
    std::cout << "Total : " << passed + failed << '\n';
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
