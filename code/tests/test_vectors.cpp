#include "rabbit.h"
#include "utils.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>

// So sanh khoi 16 byte voi gia tri mong doi va in PASS/FAIL.
static bool verifyBlock(const std::string& testName,
                        const uint8_t actual[16],
                        const uint8_t expected[16]) {
    bool match = (std::memcmp(actual, expected, 16) == 0);
    std::cout << "  " << std::left << std::setw(32) << testName << ": ";
    if (match) {
        std::cout << "[PASS]\n";
    } else {
        std::cout << "[FAIL]\n";
        std::cout << "    Expected: " << Utils::bytesToHex(expected, 16) << "\n";
        std::cout << "    Actual  : " << Utils::bytesToHex(actual, 16) << "\n";
    }
    return match;
}

int main() {
    std::cout << "====================================================================\n";
    std::cout << "      RABBIT STREAM CIPHER - OFFICIAL TEST VECTORS VERIFICATION     \n";
    std::cout << "====================================================================\n\n";

    int totalTests = 0;
    int passedTests = 0;

    // =========================================================================
    // BO TEST 1: Test vector FSE 2003 (PDF 978-3-540-39887-5_23.pdf, Phu luc B)
    // =========================================================================
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << " SUITE 1: FSE 2003 Original Paper Vectors (Little-Endian / PDF App B)\n";
    std::cout << "--------------------------------------------------------------------\n";

    // --- Vector 1 (Key = 0) ---
    {
        uint8_t key[16] = {0};
        RabbitCipher cipher(EndianMode::FSE2003_LE);
        cipher.setupKey(key);

        uint8_t exp_s0[16]  = {0x02, 0xF7, 0x4A, 0x1C, 0x26, 0x45, 0x6B, 0xF5, 0xEC, 0xD6, 0xA5, 0x36, 0xF0, 0x54, 0x57, 0xB1};
        uint8_t exp_s1[16]  = {0xA7, 0x8A, 0xC6, 0x89, 0x47, 0x6C, 0x69, 0x7B, 0x39, 0x0C, 0x9C, 0xC5, 0x15, 0xD8, 0xE8, 0x88};
        uint8_t exp_s31[16] = {0xEF, 0x9A, 0x69, 0x71, 0x8B, 0x82, 0x49, 0xA1, 0xA7, 0x3C, 0x5A, 0x6E, 0x5B, 0x90, 0x45, 0x95};

        uint8_t block[16];
        for (int r = 0; r <= 31; ++r) {
            cipher.generateBlock(block);
            if (r == 0) {
                totalTests++;
                if (verifyBlock("PDF Vector 1 (Key=0) s[0]", block, exp_s0)) passedTests++;
            } else if (r == 1) {
                totalTests++;
                if (verifyBlock("PDF Vector 1 (Key=0) s[1]", block, exp_s1)) passedTests++;
            } else if (r == 31) {
                totalTests++;
                if (verifyBlock("PDF Vector 1 (Key=0) s[31]", block, exp_s31)) passedTests++;
            }
        }
    }

    // --- Vector 2 (Key = C2 1F CF 38 ...) ---
    {
        uint8_t key[16] = {0xC2, 0x1F, 0xCF, 0x38, 0x81, 0xCD, 0x5E, 0xE8, 0x62, 0x8A, 0xCC, 0xB0, 0xA9, 0x89, 0x0D, 0xF8};
        RabbitCipher cipher(EndianMode::FSE2003_LE);
        cipher.setupKey(key);

        uint8_t exp_s0[16]  = {0x3D, 0x02, 0xE0, 0xC7, 0x30, 0x55, 0x91, 0x12, 0xB4, 0x73, 0xB7, 0x90, 0xDE, 0xE0, 0x18, 0xDF};
        uint8_t exp_s1[16]  = {0xCD, 0x6D, 0x73, 0x0C, 0xE5, 0x4E, 0x19, 0xF0, 0xC3, 0x5E, 0xC4, 0x79, 0x0E, 0xB6, 0xC7, 0x4A};
        uint8_t exp_s31[16] = {0x9F, 0xB4, 0x92, 0xE1, 0xB5, 0x40, 0x36, 0x3A, 0xE3, 0x83, 0xC0, 0x1F, 0x9F, 0xA2, 0x26, 0x1A};

        uint8_t block[16];
        for (int r = 0; r <= 31; ++r) {
            cipher.generateBlock(block);
            if (r == 0) {
                totalTests++;
                if (verifyBlock("PDF Vector 2 (Key=C2..) s[0]", block, exp_s0)) passedTests++;
            } else if (r == 1) {
                totalTests++;
                if (verifyBlock("PDF Vector 2 (Key=C2..) s[1]", block, exp_s1)) passedTests++;
            } else if (r == 31) {
                totalTests++;
                if (verifyBlock("PDF Vector 2 (Key=C2..) s[31]", block, exp_s31)) passedTests++;
            }
        }
    }

    // --- Vector 3 (Key = 1D 27 2C 6A ...) ---
    {
        uint8_t key[16] = {0x1D, 0x27, 0x2C, 0x6A, 0x2D, 0x8E, 0x3D, 0xFC, 0xAC, 0x14, 0x05, 0x6B, 0x78, 0xD6, 0x33, 0xA0};
        RabbitCipher cipher(EndianMode::FSE2003_LE);
        cipher.setupKey(key);

        uint8_t exp_s0[16]  = {0xA3, 0xA9, 0x7A, 0xBB, 0x80, 0x39, 0x38, 0x20, 0xB7, 0xE5, 0x0C, 0x4A, 0xBB, 0x53, 0x82, 0x3D};
        uint8_t exp_s1[16]  = {0xC4, 0x42, 0x37, 0x99, 0xC2, 0xEF, 0xC9, 0xFF, 0xB3, 0xA4, 0x12, 0x5F, 0x1F, 0x4C, 0x99, 0xA8};
        uint8_t exp_s31[16] = {0x97, 0xC0, 0x73, 0x3F, 0xF1, 0xF1, 0x8D, 0x25, 0x6A, 0x59, 0xE2, 0xBA, 0xAB, 0xC1, 0xF4, 0xF1};

        uint8_t block[16];
        for (int r = 0; r <= 31; ++r) {
            cipher.generateBlock(block);
            if (r == 0) {
                totalTests++;
                if (verifyBlock("PDF Vector 3 (Key=1D..) s[0]", block, exp_s0)) passedTests++;
            } else if (r == 1) {
                totalTests++;
                if (verifyBlock("PDF Vector 3 (Key=1D..) s[1]", block, exp_s1)) passedTests++;
            } else if (r == 31) {
                totalTests++;
                if (verifyBlock("PDF Vector 3 (Key=1D..) s[31]", block, exp_s31)) passedTests++;
            }
        }
    }

    // =========================================================================
    // BO TEST 2: Test vector RFC 4503 (Phu luc A)
    // =========================================================================
    std::cout << "\n--------------------------------------------------------------------\n";
    std::cout << " SUITE 2: RFC 4503 Standard Vectors (Big-Endian OS2IP / Appendix A)\n";
    std::cout << "--------------------------------------------------------------------\n";

    // --- Vector A.1.1 (Key = 0, khong IV) ---
    {
        uint8_t key[16] = {0};
        RabbitCipher cipher(EndianMode::RFC4503);
        cipher.setupKey(key);

        uint8_t exp_s0[16] = {0xB1, 0x57, 0x54, 0xF0, 0x36, 0xA5, 0xD6, 0xEC, 0xF5, 0x6B, 0x45, 0x26, 0x1C, 0x4A, 0xF7, 0x02};
        uint8_t exp_s1[16] = {0x88, 0xE8, 0xD8, 0x15, 0xC5, 0x9C, 0x0C, 0x39, 0x7B, 0x69, 0x6C, 0x47, 0x89, 0xC6, 0x8A, 0xA7};
        uint8_t exp_s2[16] = {0xF4, 0x16, 0xA1, 0xC3, 0x70, 0x0C, 0xD4, 0x51, 0xDA, 0x68, 0xD1, 0x88, 0x16, 0x73, 0xD6, 0x96};

        uint8_t block[16];
        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.1.1 (Key=0) S[0]", block, exp_s0)) passedTests++;

        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.1.1 (Key=0) S[1]", block, exp_s1)) passedTests++;

        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.1.1 (Key=0) S[2]", block, exp_s2)) passedTests++;
    }

    // --- Vector A.1.2 (Key = 91 28 13 29..., khong IV) ---
    {
        uint8_t key[16] = {0x91, 0x28, 0x13, 0x29, 0x2E, 0x3D, 0x36, 0xFE, 0x3B, 0xFC, 0x62, 0xF1, 0xDC, 0x51, 0xC3, 0xAC};
        RabbitCipher cipher(EndianMode::RFC4503);
        cipher.setupKey(key);

        uint8_t exp_s0[16] = {0x3D, 0x2D, 0xF3, 0xC8, 0x3E, 0xF6, 0x27, 0xA1, 0xE9, 0x7F, 0xC3, 0x84, 0x87, 0xE2, 0x51, 0x9C};
        uint8_t exp_s1[16] = {0xF5, 0x76, 0xCD, 0x61, 0xF4, 0x40, 0x5B, 0x88, 0x96, 0xBF, 0x53, 0xAA, 0x85, 0x54, 0xFC, 0x19};
        uint8_t exp_s2[16] = {0xE5, 0x54, 0x74, 0x73, 0xFB, 0xDB, 0x43, 0x50, 0x8A, 0xE5, 0x3B, 0x20, 0x20, 0x4D, 0x4C, 0x5E};

        uint8_t block[16];
        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.1.2 (Key=91..) S[0]", block, exp_s0)) passedTests++;

        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.1.2 (Key=91..) S[1]", block, exp_s1)) passedTests++;

        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.1.2 (Key=91..) S[2]", block, exp_s2)) passedTests++;
    }

    // --- Vector A.2.1 (Key = 0, IV = 0) ---
    {
        uint8_t key[16] = {0};
        uint8_t iv[8]  = {0};
        RabbitCipher cipher(EndianMode::RFC4503);
        cipher.setupKey(key);
        cipher.setupIV(iv);

        uint8_t exp_s0[16] = {0xC6, 0xA7, 0x27, 0x5E, 0xF8, 0x54, 0x95, 0xD8, 0x7C, 0xCD, 0x5D, 0x37, 0x67, 0x05, 0xB7, 0xED};
        uint8_t exp_s1[16] = {0x5F, 0x29, 0xA6, 0xAC, 0x04, 0xF5, 0xEF, 0xD4, 0x7B, 0x8F, 0x29, 0x32, 0x70, 0xDC, 0x4A, 0x8D};
        uint8_t exp_s2[16] = {0x2A, 0xDE, 0x82, 0x2B, 0x29, 0xDE, 0x6C, 0x1E, 0xE5, 0x2B, 0xDB, 0x8A, 0x47, 0xBF, 0x8F, 0x66};

        uint8_t block[16];
        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.2.1 (IV=0) S[0]", block, exp_s0)) passedTests++;

        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.2.1 (IV=0) S[1]", block, exp_s1)) passedTests++;

        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.2.1 (IV=0) S[2]", block, exp_s2)) passedTests++;
    }

    // --- Vector A.2.2 (Key = 0, IV = C3 73 F5 75 C1 26 7E 59) ---
    {
        uint8_t key[16] = {0};
        uint8_t iv[8]  = {0xC3, 0x73, 0xF5, 0x75, 0xC1, 0x26, 0x7E, 0x59};
        RabbitCipher cipher(EndianMode::RFC4503);
        cipher.setupKey(key);
        cipher.setupIV(iv);

        uint8_t exp_s0[16] = {0x1F, 0xCD, 0x4E, 0xB9, 0x58, 0x00, 0x12, 0xE2, 0xE0, 0xDC, 0xCC, 0x92, 0x22, 0x01, 0x7D, 0x6D};
        uint8_t exp_s1[16] = {0xA7, 0x5F, 0x4E, 0x10, 0xD1, 0x21, 0x25, 0x01, 0x7B, 0x24, 0x99, 0xFF, 0xED, 0x93, 0x6F, 0x2E};
        uint8_t exp_s2[16] = {0xEB, 0xC1, 0x12, 0xC3, 0x93, 0xE7, 0x38, 0x39, 0x23, 0x56, 0xBD, 0xD0, 0x12, 0x02, 0x9B, 0xA7};

        uint8_t block[16];
        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.2.2 (IV=C3..) S[0]", block, exp_s0)) passedTests++;

        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.2.2 (IV=C3..) S[1]", block, exp_s1)) passedTests++;

        cipher.generateBlock(block);
        totalTests++;
        if (verifyBlock("RFC 4503 A.2.2 (IV=C3..) S[2]", block, exp_s2)) passedTests++;
    }

    // =========================================================================
    // TONG KET
    // =========================================================================
    std::cout << "\n====================================================================\n";
    std::cout << " TEST SUMMARY: " << passedTests << " / " << totalTests << " TESTS PASSED (";
    std::cout << std::fixed << std::setprecision(1) << (passedTests * 100.0 / totalTests) << "%)\n";
    std::cout << "====================================================================\n";

    return (passedTests == totalTests) ? 0 : 1;
}
