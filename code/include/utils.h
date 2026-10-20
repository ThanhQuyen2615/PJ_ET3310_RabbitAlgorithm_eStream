#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace Utils {

    // Quay trai / phai so 32 bit di `shift` bit.
    uint32_t rotl32(uint32_t value, unsigned int shift);
    uint32_t rotr32(uint32_t value, unsigned int shift);

    // Ham g cua Rabbit: binh phuong (u + v) thanh so 64 bit, roi XOR nua cao voi nua thap.
    uint32_t gFunc(uint32_t u, uint32_t v);

    // Chuyen chuoi hex thanh mang byte. Chap nhan tien to "0x" va dau cach, ':', '-', ','.
    // Tra ve false neu co ky tu khong phai hex hoac so chu so la so le.
    bool hexToBytes(const std::string& hexStr, std::vector<uint8_t>& outBytes);

    // Chuyen mang byte thanh chuoi hex.
    std::string bytesToHex(const uint8_t* data, size_t length, bool uppercase = true);
    std::string bytesToHex(const std::vector<uint8_t>& bytes, bool uppercase = true);

    // In "label: AA BB CC ..." ra man hinh.
    void printHex(const std::string& label, const uint8_t* data, size_t length);
    void printHex(const std::string& label, const std::vector<uint8_t>& bytes);

} // namespace Utils
