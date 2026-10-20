#include "utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>

namespace Utils {

    uint32_t rotl32(uint32_t value, unsigned int shift) {
        // Shifting by 32 is undefined in C++, so keep shift in 0..31.
        shift &= 31;
        if (shift == 0) {
            return value;
        }
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
        // u + v wraps mod 2^32; widen to 64 bits so the square does not overflow.
        uint64_t sum = static_cast<uint32_t>(u + v);
        uint64_t square = sum * sum;

        // Low 32 bits XOR high 32 bits.
        return static_cast<uint32_t>(square ^ (square >> 32));
    }

    bool hexToBytes(const std::string& hexStr, std::vector<uint8_t>& outBytes) {
        outBytes.clear();

        // Strip "0x" prefix and separators, keep only hex digits.
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
                return false;
            }
            cleaned.push_back(c);
        }

        // Two hex digits per byte.
        if (cleaned.empty() || cleaned.size() % 2 != 0) {
            return false;
        }

        outBytes.reserve(cleaned.size() / 2);

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
