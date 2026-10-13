#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

// Algorithm parameter constants defined in RFC 4503 and FSE 2003
constexpr size_t RABBIT_KEY_SIZE   = 16;  // 128-bit secret key = 16 bytes
constexpr size_t RABBIT_IV_SIZE    = 8;   // 64-bit initialization vector = 8 bytes
constexpr size_t RABBIT_BLOCK_SIZE = 16;  // 128-bit keystream output block = 16 bytes

/**
 * @brief Endianness format conventions supported by the Rabbit cipher.
 * 
 * - RFC4503: Standardized Internet RFC format (Big-Endian OS2IP / I2OSP octet ordering).
 * - FSE2003_LE: Original FSE 2003 paper format (978-3-540-39887-5_23.pdf Appendix B,
 *               Little-Endian word and byte ordering used on x86 architectures).
 */
enum class EndianMode {
    RFC4503,      // RFC 4503 Standard (Default)
    FSE2003_LE    // FSE 2003 Original Paper (PDF Appendix B)
};

/**
 * @brief Rabbit cipher internal 513-bit state structure.
 * 
 * [INTERNAL STATE SPECIFICATION (RFC 4503 Section 2.2 / FSE 2003 Section 2.1)]
 * The internal state consists of:
 *  - 8 state words X0..X7 (32-bit each = 256 bits)
 *  - 8 counter words C0..C7 (32-bit each = 256 bits)
 *  - 1 counter carry bit 'carry' (1 bit)
 * Total internal state: 256 + 256 + 1 = 513 bits.
 */
struct RabbitState {
    uint32_t x[8];    // State variables X0..X7
    uint32_t c[8];    // Counter variables C0..C7
    uint32_t carry;   // Counter carry bit b (0 or 1)
};

/**
 * @brief High-performance C++ class implementing the Rabbit Stream Cipher.
 */
class RabbitCipher {
public:
    explicit RabbitCipher(EndianMode mode = EndianMode::RFC4503);
    ~RabbitCipher() = default;

    /**
     * @brief Configure byte-ordering mode (RFC 4503 Big-Endian vs FSE 2003 Little-Endian).
     */
    void setEndianMode(EndianMode mode);
    EndianMode getEndianMode() const;

    /**
     * @brief Initializes cipher state using a 128-bit key.
     * @param key 16-byte raw key array.
     */
    void setupKey(const uint8_t key[RABBIT_KEY_SIZE]);
    void setupKey(const std::vector<uint8_t>& key);

    /**
     * @brief Mixes a 64-bit Initialization Vector (IV) into counter state.
     * @param iv 8-byte raw IV array.
     */
    void setupIV(const uint8_t iv[RABBIT_IV_SIZE]);
    void setupIV(const std::vector<uint8_t>& iv);

    /**
     * @brief Resets the working state back to the Master State (post-key-setup).
     */
    void resetToMaster();

    /**
     * @brief Generates one 128-bit (16-byte) keystream block.
     * @param output 16-byte destination buffer.
     */
    void generateBlock(uint8_t output[RABBIT_BLOCK_SIZE]);

    /**
     * @brief Encrypts or decrypts arbitrary-length data stream.
     * @param input Pointer to source data buffer.
     * @param output Pointer to destination data buffer (can be identical to input for in-place).
     * @param length Number of bytes to process.
     */
    void process(const uint8_t* input, uint8_t* output, size_t length);
    void process(std::vector<uint8_t>& data);

    // Context status inspectors
    bool isKeySet() const;
    bool isIVSet() const;

private:
    RabbitState state_;       // Working internal state
    RabbitState masterState_; // Cached master state after Key Setup
    bool hasKey_;             // Key initialization flag
    bool hasIV_;              // IV initialization flag
    EndianMode endianMode_;   // Endianness convention (RFC 4503 vs FSE 2003)

    uint8_t buffer_[RABBIT_BLOCK_SIZE]; // 16-byte keystream cache buffer
    size_t bufferPos_;                  // Read pointer in keystream cache

    /**
     * @brief Executes one iteration of the Counter System & Next-State function.
     */
    void nextState();
};
