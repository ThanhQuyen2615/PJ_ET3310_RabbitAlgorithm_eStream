#include "rabbit.h"
#include "utils.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// So test da chay va so test that bai, dung chung cho moi ham test.
static int totalTests = 0;
static int failedTests = 0;

// In PASS/FAIL cho mot truong hop kiem tra.
static void check(const std::string& name, bool ok) {
    totalTests++;
    if (!ok) failedTests++;
    std::cout << "  " << name << ": " << (ok ? "[PASS]" : "[FAIL]") << "\n";
}

// Khoa va IV co dinh dung chung cho cac test.
static const uint8_t KEY[16] = {
    0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
    0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF
};
static const uint8_t IV[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};

// Test 1: ma hoa chuoi roi giai ma bang doi tuong moi phai ra lai chuoi goc.
static void testString() {
    const std::string text = "Xin chao ET3310!";
    const std::vector<uint8_t> plain(text.begin(), text.end());
    std::vector<uint8_t> cipher(plain.size());
    std::vector<uint8_t> decrypted(plain.size());

    RabbitCipher enc;
    enc.setupKey(KEY);
    enc.setupIV(IV);
    enc.process(plain.data(), cipher.data(), plain.size());

    RabbitCipher dec;
    dec.setupKey(KEY);
    dec.setupIV(IV);
    dec.process(cipher.data(), decrypted.data(), cipher.size());

    check("chuoi: giai ma ra lai ban goc", decrypted == plain);
    check("chuoi: ban ma khac ban ro", cipher != plain);
}


int main() {
    std::cout << "=== TEST MA HOA / GIAI MA ===\n";

    testString();

    std::cout << "\nTong: " << totalTests - failedTests << "/" << totalTests << " PASS\n";
    return failedTests == 0 ? 0 : 1;
}
