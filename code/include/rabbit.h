#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

// Algorithm parameter constants defined in RFC 4503
constexpr size_t RABBIT_KEY_SIZE   = 16;  // 128-bit secret key = 16 bytes
constexpr size_t RABBIT_IV_SIZE    = 8;   // 64-bit initialization vector = 8 bytes
constexpr size_t RABBIT_BLOCK_SIZE = 16;  // 128-bit keystream output block = 16 bytes

/**
 * @brief Rabbit cipher internal 513-bit state structure.
 * 
 * [INTERNAL STATE SPECIFICATION (RFC 4503 Section 2.2)]
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
 * @brief High-performance C++ class implementing the Rabbit Stream Cipher (RFC 4503).
 */
class RabbitCipher {
public:
    RabbitCipher();
    ~RabbitCipher() = default;

    /**
     * @brief Initializes cipher state using a 128-bit key (RFC 4503 Section 2.3).
     * @param key 16-byte raw key array.
     */
    void setupKey(const uint8_t key[RABBIT_KEY_SIZE]);
    void setupKey(const std::vector<uint8_t>& key);

    /**
     * @brief Mixes a 64-bit Initialization Vector (IV) into counter state (RFC 4503 Section 2.4).
     * @param iv 8-byte raw IV array.
     */
    void setupIV(const uint8_t iv[RABBIT_IV_SIZE]);
    void setupIV(const std::vector<uint8_t>& iv);

    /**
     * @brief Resets the working state back to the Master State (post-key-setup).
     * 
     * [OPTIMIZATION DETAIL]
     * RFC 4503 specifies that after Key Setup, the state is saved as Master State.
     * When re-initializing under a new IV, resetToMaster() restores this state directly,
     * avoiding the overhead of re-computing the 4 key-expansion warmup rounds.
     */
    void resetToMaster();

    /**
     * @brief Generates one 128-bit (16-byte) keystream block (RFC 4503 Section 2.7).
     * @param output 16-byte destination buffer.
     */
    void generateBlock(uint8_t output[RABBIT_BLOCK_SIZE]);

    /**
     * @brief Encrypts or decrypts arbitrary-length data stream (RFC 4503 Section 2.8).
     * @param input Pointer to source data buffer.
     * @param output Pointer to destination data buffer (can be identical to input for in-place).
     * @param length Number of bytes to process.
     * 
     * [OPTIMIZATION DETAIL]
     * An internal 16-byte keystream buffer is maintained (`buffer_`). Data of any arbitrary
     * size (e.g. 1 byte, 15 bytes, 1 MB) can be processed continuously across multiple calls
     * without wasting unaligned keystream bytes.
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

    uint8_t buffer_[RABBIT_BLOCK_SIZE]; // 16-byte keystream cache buffer
    size_t bufferPos_;                  // Read pointer in keystream cache

    /**
     * @brief Executes one iteration of the Counter System & Next-State function.
     * 
     * [OPTIMIZATION DETAIL]
     * Combines counter ripple updates and non-linear G-function updates into a tightly
     * coupled pipelined routine. All modulo-8 array index operations are replaced with
     * bitwise masking `& 7` for maximum instruction-level throughput.
     */
    void nextState();
};
