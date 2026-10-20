#include <iostream>
#include <iomanip>
#include <cassert>
#include "rabbit.h"

void run_key_setup_test(const uint8_t key[16], const uint8_t expected_output[16], int test_num) {
    RabbitCipher cipher(ByteOrder::RFC);
    cipher.setupKey(key);

    uint8_t output[16];
    cipher.generateBlock(output);

    bool pass = true;
    for (int i = 0; i < 16; ++i) {
        if (output[i] != expected_output[i]) {
            pass = false;
            break;
        }
    }

    if (!pass) {
        std::cerr << "[FAIL] Test Key Setup " << test_num << " failed!\n";
        std::cerr << "Expected : ";
        for (int i = 0; i < 16; ++i) std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)expected_output[i] << " ";
        std::cerr << "\nGot      : ";
        for (int i = 0; i < 16; ++i) std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)output[i] << " ";
        std::cerr << std::dec << std::endl;
        assert(false);
    } else {
        std::cout << "  - Test case " << test_num << " passed.\n";
    }
}

void test_key_setup() {
    std::cout << "Running test_key_setup (RFC 4503 Appendix A.1)...\n";

    // Test vector 1: Khóa toàn 0
    uint8_t key1[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t expected1[16] = {0xB1, 0x57, 0x54, 0xF0, 0x36, 0xA5, 0xD6, 0xEC,
                             0xF5, 0x6B, 0x45, 0x26, 0x1C, 0x4A, 0xF7, 0x02};
    run_key_setup_test(key1, expected1, 1);

    // Test vector 2: Khóa chuẩn và kết quả chuẩn khớp với đầu ra của chương trình
    uint8_t key2[16] = {0x91, 0x28, 0x13, 0x29, 0x2E, 0x3D, 0x36, 0xFE,
                        0x3B, 0xFC, 0x62, 0xF1, 0xDC, 0x51, 0xC3, 0xAC};
    uint8_t expected2[16] = {0x3D, 0x2D, 0xF3, 0xC8, 0x3E, 0xF6, 0x27, 0xA1,
                             0xE9, 0x7F, 0xC3, 0x84, 0x87, 0xE2, 0x51, 0x9C};
    run_key_setup_test(key2, expected2, 2);

    std::cout << "[PASS] Key setup testing completed successfully!\n";
}

int main() {
    test_key_setup();
    return 0;
}