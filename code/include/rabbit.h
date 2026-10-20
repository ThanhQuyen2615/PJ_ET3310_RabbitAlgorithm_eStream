#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

constexpr size_t RABBIT_KEY_SIZE   = 16;  // 128-bit key
constexpr size_t RABBIT_IV_SIZE    = 8;   // 64-bit IV
constexpr size_t RABBIT_BLOCK_SIZE = 16;  // 128-bit keystream block

// Byte order used to load key/IV and output keystream.
enum class EndianMode {
    RFC4503,      // RFC 4503, big-endian (default)
    FSE2003_LE    // Original FSE 2003 paper, little-endian
};

// Internal state: 8 state words + 8 counters + 1 carry bit = 513 bits.
struct RabbitState {
    uint32_t x[8];    // State variables X0..X7
    uint32_t c[8];    // Counter variables C0..C7
    uint32_t carry;   // Counter carry bit (0 or 1)
};

// Rabbit stream cipher. Encryption and decryption are the same operation.
class RabbitCipher {
public:
    explicit RabbitCipher(EndianMode mode = EndianMode::RFC4503);
    ~RabbitCipher() = default;

    void setEndianMode(EndianMode mode);
    EndianMode getEndianMode() const;

    // Load a 16-byte key. Must be called first.
    void setupKey(const uint8_t key[RABBIT_KEY_SIZE]);
    void setupKey(const std::vector<uint8_t>& key);

    // Mix an 8-byte IV into the key state (optional).
    void setupIV(const uint8_t iv[RABBIT_IV_SIZE]);
    void setupIV(const std::vector<uint8_t>& iv);

    // Return to the state right after setupKey().
    void resetToMaster();

    // Produce the next 16-byte keystream block.
    void generateBlock(uint8_t output[RABBIT_BLOCK_SIZE]);

    // XOR data with the keystream. input and output may be the same buffer.
    void process(const uint8_t* input, uint8_t* output, size_t length);
    void process(std::vector<uint8_t>& data);

    bool isKeySet() const;
    bool isIVSet() const;

private:
    RabbitState state_;       // Current state
    RabbitState masterState_; // State saved after setupKey()
    bool hasKey_;
    bool hasIV_;
    EndianMode endianMode_;

    uint8_t buffer_[RABBIT_BLOCK_SIZE]; // Current keystream block
    size_t bufferPos_;                  // Next unused byte in buffer_

    // Advance counters and state by one round.
    void nextState();
};
