#include "rabbit.h"
#include "utils.h"
#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    // RFC 4503 Appendix A.2 test vector
    const std::vector<uint8_t> key(16, 0x00);
    const std::vector<uint8_t> iv = {0xC3, 0x73, 0xF5, 0x75, 0xC1, 0x26, 0x7E, 0x59};
    const std::vector<uint8_t> expected = {
        0x1F,0xCD,0x4E,0xB9,0x58,0x00,0x12,0xE2,0xE0,0xDC,0xCC,0x92,0x22,0x01,0x7D,0x6D,
        0xA7,0x5F,0x4E,0x10,0xD1,0x21,0x25,0x01,0x7B,0x24,0x99,0xFF,0xED,0x93,0x6F,0x2E,
        0xEB,0xC1,0x12,0xC3,0x93,0xE7,0x38,0x39,0x23,0x56,0xBD,0xD0,0x12,0x02,0x9B,0xA7
    };

    RabbitCipher cipher(EndianMode::RFC4503);
    cipher.setupKey(key);
    cipher.setupIV(iv);

    std::vector<uint8_t> plaintext(expected.size(), 0x00);
    std::vector<uint8_t> output(expected.size(), 0x00);
    cipher.process(plaintext.data(), output.data(), plaintext.size());

    Utils::printHex("Expected", expected);
    Utils::printHex("Actual  ", output);

    if (output == expected) {
        std::cout << "PASS: Rabbit key + IV setup matches RFC 4503 test vector.\n";
        return 0;
    }
    std::cout << "FAIL: Output does not match RFC 4503 test vector.\n";
    return 1;
}
