
#include "../include/utils.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iomanip>

// In ket qua kiem thu va cap nhat so luong PASS/FAIL.
void checkResult(
    const char* name,
    uint32_t expected,
    uint32_t actual,
    int& passed,
    int& failed,
    bool verbose = true
) {
    if (actual == expected) {
        ++passed;

        if (verbose) {
            std::cout << name << ": PASS\n";
        }
    } else {
        ++failed;

        std::cout << name << ": FAIL"
                  << " | Expected: 0x"
                  << std::hex << std::uppercase
                  << std::setfill('0') << std::setw(8)
                  << expected
                  << " | Actual: 0x"
                  << std::setw(8) << actual
                  << std::dec << '\n';
    }
}

// Kiem tra ham g voi gia tri mong doi da tinh truoc.
void runTest(
    const char* name,
    uint32_t u,
    uint32_t v,
    uint32_t expected,
    int& passed,
    int& failed
) {
    uint32_t actual = Utils::gFunc(u, v);

    checkResult(name, expected, actual, passed, failed);
}

// Tao so gia ngau nhien 32 bit bang rand().
uint32_t random32() {
    uint32_t result = 0;

    for (int i = 0; i < 4; ++i) {
        result = (result << 8) |
                 static_cast<uint32_t>(rand() & 0xFF);
    }

    return result;
}

// Kiem tra tinh giao hoan: g(u, v) = g(v, u).
void testCommutative(int& passed, int& failed) {
    int failedBefore = failed;

    for (int i = 0; i < 1000; ++i) {
        uint32_t u = random32();
        uint32_t v = random32();

        uint32_t expected = Utils::gFunc(u, v);
        uint32_t actual = Utils::gFunc(v, u);

        if (actual != expected) {
            std::cout << "Lan thu " << i + 1
                      << " | u = 0x"
                      << std::hex << std::uppercase
                      << std::setfill('0') << std::setw(8)
                      << u
                      << " | v = 0x"
                      << std::setw(8) << v
                      << std::dec << '\n';
        }

        checkResult("Commutative property",
                    expected, actual, passed, failed, false);
    }

    std::cout << "Commutative property (1000 cases): "
              << (failed == failedBefore ? "PASS" : "FAIL")
              << '\n';
}

// Kiem tra tinh bat bien tong: g(u+k, v-k) = g(u, v).
void testSumInvariant(int& passed, int& failed) {
    int failedBefore = failed;

    for (int i = 0; i < 1000; ++i) {
        uint32_t u = random32();
        uint32_t v = random32();
        uint32_t k = random32();

        uint32_t expected = Utils::gFunc(u, v);

        uint32_t actual = Utils::gFunc(
            static_cast<uint32_t>(u + k),
            static_cast<uint32_t>(v - k)
        );

        if (actual != expected) {
            std::cout << "Lan thu " << i + 1
                      << " | u = 0x"
                      << std::hex << std::uppercase
                      << std::setfill('0') << std::setw(8)
                      << u
                      << " | v = 0x"
                      << std::setw(8) << v
                      << " | k = 0x"
                      << std::setw(8) << k
                      << std::dec << '\n';
        }

        checkResult("Sum invariant property",
                    expected, actual, passed, failed, false);
    }

    std::cout << "Sum invariant property (1000 cases): "
              << (failed == failedBefore ? "PASS" : "FAIL")
              << '\n';
}

int main() {
    int passed = 0;
    int failed = 0;

    // Seed co dinh de co the tai lap ket qua kiem thu.
    srand(12345);

    std::cout << "========== G FUNCTION TEST ==========\n";

    // Test 1: g(0, 0) = 0.
    runTest("Test 1: g(0, 0)",
            0x00000000u, 0x00000000u,
            0x00000000u, passed, failed);

    // Test 2: g(1, 0) = 1.
    runTest("Test 2: g(1, 0)",
            0x00000001u, 0x00000000u,
            0x00000001u, passed, failed);

    // Test 3: 0xFFFF^2 = 0xFFFE0001.
    runTest("Test 3: g(0xFFFF, 0)",
            0x0000FFFFu, 0x00000000u,
            0xFFFE0001u, passed, failed);

    // Test 4: 0x10000^2 = 0x0000000100000000.
    // XOR 32 bit cao va thap cho ket qua 1.
    runTest("Test 4: g(0x10000, 0)",
            0x00010000u, 0x00000000u,
            0x00000001u, passed, failed);

    // Test 5: 0xFFFFFFFF^2 = 0xFFFFFFFE00000001.
    // XOR 0xFFFFFFFE voi 0x00000001 = 0xFFFFFFFF.
    runTest("Test 5: g(0xFFFFFFFF, 0)",
            0xFFFFFFFFu, 0x00000000u,
            0xFFFFFFFFu, passed, failed);

    // Test 6: Phep cong 32 bit tran ve 0.
    runTest("Test 6: g(0xFFFFFFFF, 1)",
            0xFFFFFFFFu, 0x00000001u,
            0x00000000u, passed, failed);
    // Test 7: g(1, 1) = 4.
    runTest("Test 7: g(1, 1)",
        0x00000001u, 0x00000001u,
        0x00000004u, passed, failed);

    // Test 8: Phep cong 32 bit tran ve 1.
    runTest("Test 8: g(0xFFFFFFFF, 2)",
        0xFFFFFFFFu, 0x00000002u,
        0x00000001u, passed, failed);
    std::cout << "\n========== PROPERTY TESTS ==========\n";

    testCommutative(passed, failed);
    testSumInvariant(passed, failed);

    std::cout << "\n========== SUMMARY ==========\n";
    std::cout << "Total : " << passed + failed << '\n';
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
