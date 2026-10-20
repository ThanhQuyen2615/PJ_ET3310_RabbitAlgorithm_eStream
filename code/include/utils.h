#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace Utils {

    // Quay trai so 32 bit di n bit (1 <= n <= 31).
    uint32_t rotl32(uint32_t value, int n);

    // Ham g cua Rabbit: binh phuong (u + v) thanh so 64 bit, roi XOR nua cao voi nua thap.
    uint32_t gFunc(uint32_t u, uint32_t v);

    // Chuyen chuoi hex (vd "0A1B") thanh mang byte. Tra ve false neu chuoi khong hop le.
    bool hexToBytes(const std::string& hex, std::vector<uint8_t>& bytes);

    // Chuyen mang byte thanh chuoi hex (vd {0x0A, 0x1B} -> "0A1B").
    std::string bytesToHex(const uint8_t* data, size_t length);

    // In "label: AA BB CC ..." ra man hinh.
    void printHex(const std::string& label, const uint8_t* data, size_t length);

} // namespace Utils
