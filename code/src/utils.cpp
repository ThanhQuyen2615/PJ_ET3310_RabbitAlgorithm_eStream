#include "utils.h"
#include <iostream>

namespace Utils {

    uint32_t rotl32(uint32_t value, int n) {
        // Bit bi day ra ben trai quay vong ve ben phai.
        return (value << n) | (value >> (32 - n));
    }

    uint32_t gFunc(uint32_t u, uint32_t v) {
        uint32_t sum = u + v;             // Tu dong lay mod 2^32
        uint64_t sum64 = sum;             // Doi sang 64 bit de binh phuong khong bi tran
        uint64_t square = sum64 * sum64;

        uint32_t low  = square;           // 32 bit thap
        uint32_t high = square >> 32;     // 32 bit cao
        return low ^ high;
    }

    // Gia tri cua mot ky tu hex ('0'..'9', 'a'..'f', 'A'..'F'), hoac -1 neu khong phai hex.
    static int hexValue(char ch) {
        if (ch >= '0' && ch <= '9') return ch - '0';
        if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
        if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
        return -1;
    }

    bool hexToBytes(const std::string& hex, std::vector<uint8_t>& bytes) {
        bytes.clear();

        // Moi byte gom 2 chu so hex.
        if (hex.empty() || hex.size() % 2 != 0) {
            return false;
        }

        for (size_t i = 0; i < hex.size(); i += 2) {
            int high = hexValue(hex[i]);
            int low  = hexValue(hex[i + 1]);
            if (high < 0 || low < 0) {
                return false;
            }
            bytes.push_back(high * 16 + low);
        }
        return true;
    }

    std::string bytesToHex(const uint8_t* data, size_t length) {
        const char* digits = "0123456789ABCDEF";
        std::string hex;
        for (size_t i = 0; i < length; ++i) {
            hex += digits[data[i] / 16];  // 4 bit cao
            hex += digits[data[i] % 16];  // 4 bit thap
        }
        return hex;
    }

    void printHex(const std::string& label, const uint8_t* data, size_t length) {
        std::cout << label << ": ";
        for (size_t i = 0; i < length; ++i) {
            std::cout << bytesToHex(data + i, 1) << " ";
        }
        std::cout << "\n";
    }

} // namespace Utils
