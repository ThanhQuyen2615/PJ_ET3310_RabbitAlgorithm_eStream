#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace Utils {

    /**
     * @brief 32-bit Circular Left Shift (Rotate Left - ROL).
     * @param value 32-bit unsigned integer to rotate.
     * @param shift Number of bits to rotate (0-31).
     * @return Rotated 32-bit unsigned integer.
     * 
     * [OPTIMIZATION DETAIL]
     * The pattern `(value << shift) | (value >> (32 - shift))` is an idiomatic bitwise rotation.
     * Modern optimizing compilers (GCC, Clang, MSVC) recognize this exact pattern and map it
     * directly to a single native assembly instruction:
     *   - x86/x86_64: `rol eax, cl`
     *   - ARM / AArch64: `ror` (with inverted shift)
     * This avoids any conditional branch instructions or multi-cycle emulations.
     */
    uint32_t rotl32(uint32_t value, unsigned int shift);

    /**
     * @brief 32-bit Circular Right Shift (Rotate Right - ROR).
     * @param value 32-bit unsigned integer to rotate.
     * @param shift Number of bits to rotate (0-31).
     * @return Rotated 32-bit unsigned integer.
     */
    uint32_t rotr32(uint32_t value, unsigned int shift);

    /**
     * @brief Non-linear Rabbit G-function defined in RFC 4503 Section 2.6.
     * @param u 32-bit state variable X[j].
     * @param v 32-bit counter variable C[j].
     * @return 32-bit non-linear output: LSW(square) ^ MSW(square).
     * 
     * [OPTIMIZATION & MATHEMATICAL DETAIL]
     * Formula: g(u, v) = LSW((u + v mod 2^32)^2) ^ MSW((u + v mod 2^32)^2)
     * 1. `(u + v)` is added in 32-bit unsigned arithmetic, naturally wrapping modulo 2^32
     *    without expensive modulo `%` operations.
     * 2. The 32-bit sum is cast to 64-bit `uint64_t` before squaring to prevent overflow.
     * 3. XORing the lower 32 bits (LSW) and upper 32 bits (MSW) collapses 64 bits of
     *    cryptographic diffusion back into a 32-bit word in just two CPU instructions:
     *    `shr` and `xor`.
     */
    uint32_t gFunc(uint32_t u, uint32_t v);

    /**
     * @brief Converts a Hexadecimal ASCII string into a vector of bytes (uint8_t).
     * @param hexStr Input hex string (e.g. "912813292E3D...", supports "0x" prefix and spaces).
     * @param outBytes Output byte vector.
     * @return true if string is valid hex with even length, false otherwise.
     */
    bool hexToBytes(const std::string& hexStr, std::vector<uint8_t>& outBytes);

    /**
     * @brief Converts raw byte array into an uppercase/lowercase Hexadecimal string.
     * @param data Pointer to byte buffer.
     * @param length Number of bytes.
     * @param uppercase Set to true for uppercase hex output (default: true).
     * @return Formatted Hex string.
     */
    std::string bytesToHex(const uint8_t* data, size_t length, bool uppercase = true);
    std::string bytesToHex(const std::vector<uint8_t>& bytes, bool uppercase = true);

    /**
     * @brief Prints a byte array in formatted Hexadecimal representation to stdout.
     * @param label Title label printed before hex values.
     * @param data Pointer to byte buffer.
     * @param length Number of bytes.
     */
    void printHex(const std::string& label, const uint8_t* data, size_t length);
    void printHex(const std::string& label, const std::vector<uint8_t>& bytes);

} // namespace Utils
