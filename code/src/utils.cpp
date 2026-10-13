#include "utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>

namespace Utils {

    uint32_t rotl32(uint32_t value, unsigned int shift) {
        // [OPTIMIZATION: Mask shift amount with 31]
        // In x86/ARM assembly, rotation counts modulo 32 are standard.
        // `shift &= 31` prevents undefined behavior in C++ when shift >= 32.
        shift &= 31;
        if (shift == 0) {
            return value;
        }

        // [OPTIMIZATION: Compiler Idiom Recognition]
        // This exact expression `(value << shift) | (value >> (32 - shift))` is recognized
        // by GCC, Clang, and MSVC optimizers (-O2 / -O3) and translated into a single
        // hardware rotation instruction (`rol` on x86, `ror` on ARM), executing in 1 cycle.
        return (value << shift) | (value >> (32 - shift));
    }

    uint32_t rotr32(uint32_t value, unsigned int shift) {
        shift &= 31;
        if (shift == 0) {
            return value;
        }
        return (value >> shift) | (value << (32 - shift));
    }

    uint32_t gFunc(uint32_t u, uint32_t v) {
        // [OPTIMIZATION: Natural Modulo 2^32 Arithmetic]
        // In RFC 4503: square(u + v) = ((u + v mod WORDSIZE) * (u + v mod WORDSIZE)).
        // Since `u` and `v` are `uint32_t`, `(u + v)` wraps modulo 2^32 natively in hardware
        // at register level, requiring zero additional modulo or branch instructions.
        uint64_t sum = static_cast<uint32_t>(u + v);

        // [OPTIMIZATION: 64-bit Multiplication]
        // Squaring a 32-bit unsigned integer produces at most a 64-bit integer (2^64 - 2^33 + 1).
        // By casting to uint64_t before multiplying, we compute the full exact square without
        // 64-bit overflow or precision loss.
        uint64_t square = sum * sum;

        // [OPTIMIZATION: LSW XOR MSW Extraction]
        // LSW is `square & 0xFFFFFFFF` (implicit in cast to uint32_t).
        // MSW is `square >> 32`.
        // XORing them collapses 64 bits of non-linear diffusion down to 32 bits using 2 opcodes:
        // `shr` (shift right 32) and `xor` (bitwise exclusive OR).
        return static_cast<uint32_t>(square ^ (square >> 32));
    }

    bool hexToBytes(const std::string& hexStr, std::vector<uint8_t>& outBytes) {
        outBytes.clear();

        // Sanitize string: skip optional '0x' prefix, spaces, colons, hyphens
        std::string cleaned;
        cleaned.reserve(hexStr.size());

        size_t start = 0;
        if (hexStr.size() >= 2 && hexStr[0] == '0' && (hexStr[1] == 'x' || hexStr[1] == 'X')) {
            start = 2;
        }

        for (size_t i = start; i < hexStr.size(); ++i) {
            char c = hexStr[i];
            if (std::isspace(static_cast<unsigned char>(c)) || c == ':' || c == '-' || c == ',') {
                continue;
            }
            if (!std::isxdigit(static_cast<unsigned char>(c))) {
                return false; // Invalid hex character encountered
            }
            cleaned.push_back(c);
        }

        // Hex string must consist of pairs of nibbles (even number of characters)
        if (cleaned.empty() || cleaned.size() % 2 != 0) {
            return false;
        }

        // Pre-allocate destination memory to prevent repeated dynamic reallocations
        outBytes.reserve(cleaned.size() / 2);

        // [OPTIMIZATION: Fast branchless ASCII nibble to integer conversion]
        auto hexCharToVal = [](char ch) -> uint8_t {
            if (ch >= '0' && ch <= '9') return static_cast<uint8_t>(ch - '0');
            if (ch >= 'a' && ch <= 'f') return static_cast<uint8_t>(ch - 'a' + 10);
            if (ch >= 'A' && ch <= 'F') return static_cast<uint8_t>(ch - 'A' + 10);
            return 0;
        };

        for (size_t i = 0; i < cleaned.size(); i += 2) {
            uint8_t high = hexCharToVal(cleaned[i]);
            uint8_t low  = hexCharToVal(cleaned[i + 1]);
            outBytes.push_back(static_cast<uint8_t>((high << 4) | low));
        }

        return true;
    }

    std::string bytesToHex(const uint8_t* data, size_t length, bool uppercase) {
        if (!data || length == 0) {
            return "";
        }

        // [OPTIMIZATION: Pre-calculate string size and format directly]
        std::string hexStr;
        hexStr.resize(length * 2);

        const char* hexDigits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

        for (size_t i = 0; i < length; ++i) {
            hexStr[2 * i]     = hexDigits[(data[i] >> 4) & 0x0F];
            hexStr[2 * i + 1] = hexDigits[data[i] & 0x0F];
        }

        return hexStr;
    }

    std::string bytesToHex(const std::vector<uint8_t>& bytes, bool uppercase) {
        return bytesToHex(bytes.data(), bytes.size(), uppercase);
    }

    void printHex(const std::string& label, const uint8_t* data, size_t length) {
        std::cout << label << ": ";
        if (!data || length == 0) {
            std::cout << "(empty)\n";
            return;
        }

        std::cout << std::hex << std::setfill('0') << std::uppercase;
        for (size_t i = 0; i < length; ++i) {
            std::cout << std::setw(2) << static_cast<int>(data[i]) << " ";
        }
        std::cout << std::dec << "\n";
    }

    void printHex(const std::string& label, const std::vector<uint8_t>& bytes) {
        printHex(label, bytes.data(), bytes.size());
    }

} // namespace Utils
