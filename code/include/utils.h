#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace Utils {

    // Rotate a 32-bit value left / right by `shift` bits.
    uint32_t rotl32(uint32_t value, unsigned int shift);
    uint32_t rotr32(uint32_t value, unsigned int shift);

    // Rabbit g-function: square (u + v) to 64 bits, then XOR its high and low halves.
    uint32_t gFunc(uint32_t u, uint32_t v);

    // Parse a hex string into bytes. Accepts "0x" prefix and spaces, ':', '-', ','.
    // Returns false if a character is not hex or the digit count is odd.
    bool hexToBytes(const std::string& hexStr, std::vector<uint8_t>& outBytes);

    // Convert bytes to a hex string.
    std::string bytesToHex(const uint8_t* data, size_t length, bool uppercase = true);
    std::string bytesToHex(const std::vector<uint8_t>& bytes, bool uppercase = true);

    // Print "label: AA BB CC ..." to stdout.
    void printHex(const std::string& label, const uint8_t* data, size_t length);
    void printHex(const std::string& label, const std::vector<uint8_t>& bytes);

} // namespace Utils
