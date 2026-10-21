#include "../include/utils.h"

#include <cstdint>
#include <cstdlib>
#include <iomanip>
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

// Tao so gia ngau nhien 32 bit bang rand().
uint32_t random32() {
    uint32_t result = 0;

    for (int i = 0; i < 4; ++i) {
        result = (result << 8)
               | static_cast<uint32_t>(rand() & 0xFF);
    }

    return result;
}

// Quay 8 bit roi 24 bit phai tra ve gia tri ban dau.
void testRotationProperty(int& passed, int& failed) {
    int failedBefore = failed;

    for (int i = 0; i < 1000; ++i) {
        uint32_t expected = random32();
        uint32_t actual = Utils::rotl32(
            Utils::rotl32(expected, 8),
            24
        );

        if (actual == expected) {
            ++passed;
        } else {
            ++failed;
            std::cout << "rotl32 property case " << i + 1
                      << ": FAIL | Expected: 0x"
                      << std::hex << std::uppercase
                      << std::setfill('0') << std::setw(8)
                      << expected
                      << " | Actual: 0x" << std::setw(8) << actual
                      << std::dec << '\n';
        }
    }

    std::cout << "rotl32: quay 8 roi 24 bit (1000 cases): "
              << (failed == failedBefore ? "PASS" : "FAIL")
              << '\n';
}

// Kiem tra chuyen chuoi hex hop le thanh dung mang byte mong doi.
void testValidHex(
    const std::string& name,
    const std::string& hex,
    const std::vector<uint8_t>& expected,
    int& passed,
    int& failed
) {
    std::vector<uint8_t> actual;
    bool converted = Utils::hexToBytes(hex, actual);

    if (converted && actual == expected) {
        ++passed;
        std::cout << name << ": PASS\n";
        return;
    }

    ++failed;
    std::cout << name << ": FAIL"
              << " | Expected: "
              << Utils::bytesToHex(expected.data(), expected.size())
              << " | Actual: "
              << (converted
                      ? Utils::bytesToHex(actual.data(), actual.size())
                      : "conversion failed")
              << '\n';
}

// Kiem tra chuoi hex khong hop le phai bi tu choi.
void testInvalidHex(
    const std::string& name,
    const std::string& hex,
    int& passed,
    int& failed
) {
    std::vector<uint8_t> actual;
    bool converted = Utils::hexToBytes(hex, actual);

    if (!converted) {
        ++passed;
        std::cout << name << ": PASS\n";
    } else {
        ++failed;
        std::cout << name << ": FAIL"
                  << " | Expected return: false"
                  << " | Actual return: true"
                  << " | Actual bytes: "
                  << Utils::bytesToHex(actual.data(), actual.size())
                  << '\n';
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    // Seed co dinh de co the tai lap ket qua kiem thu.
    srand(12345);

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

    testRotationProperty(passed, failed);

    testValidHex(
        "hexToBytes: chuoi 0A1B",
        "0A1B",
        std::vector<uint8_t>{0x0A, 0x1B},
        passed,
        failed
    );

    testValidHex(
        "hexToBytes: chu hoa va chu thuong",
        "ff00Aa",
        std::vector<uint8_t>{0xFF, 0x00, 0xAA},
        passed,
        failed
    );

    testInvalidHex(
        "hexToBytes: chuoi rong",
        "",
        passed,
        failed
    );

    testInvalidHex(
        "hexToBytes: so ky tu le",
        "ABC",
        passed,
        failed
    );

    std::cout << "\n========== SUMMARY ==========\n";
    std::cout << "Total : " << passed + failed << '\n';
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
