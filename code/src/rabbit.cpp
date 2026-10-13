#include "rabbit.h"
#include "utils.h"
#include <cstring>
#include <stdexcept>

// [ALGORITHM SPECIFICATION: RFC 4503 Section 2.5]
// Fixed counter constants A[0..7]. These asymmetric constants guarantee that the counter
// periods do not synchronize and prevent slide attacks or related-key attacks.
static const uint32_t A[8] = {
    0x4D34D34D, 0xD34D34D3, 0x34D34D34, 0x4D34D34D,
    0xD34D34D3, 0x34D34D34, 0x4D34D34D, 0xD34D34D3
};

RabbitCipher::RabbitCipher()
    : hasKey_(false), hasIV_(false), bufferPos_(RABBIT_BLOCK_SIZE) {
    std::memset(&state_, 0, sizeof(state_));
    std::memset(&masterState_, 0, sizeof(masterState_));
    std::memset(buffer_, 0, sizeof(buffer_));
}

void RabbitCipher::nextState() {
    // -------------------------------------------------------------------------
    // STEP 1: Counter System Update (RFC 4503 Section 2.5)
    // -------------------------------------------------------------------------
    // [OPTIMIZATION: Ripple Carry Propagation via 64-bit Addition]
    // Naive reference code uses `temp / WORDSIZE` and `temp % WORDSIZE` with 64-bit division.
    // In our implementation:
    //  - We compute `temp = C[j] + A[j] + carry` in a 64-bit register (`uint64_t`).
    //  - The carry bit `b` is extracted using a fast right-shift: `temp >> 32`.
    //  - The lower 32-bit result is truncated into `state_.c[j]` naturally.
    // This entirely avoids expensive hardware division and modulo instructions!
    for (int j = 0; j < 8; ++j) {
        uint64_t temp = static_cast<uint64_t>(state_.c[j]) +
                        static_cast<uint64_t>(A[j]) +
                        static_cast<uint64_t>(state_.carry);
        state_.carry = static_cast<uint32_t>(temp >> 32);
        state_.c[j]   = static_cast<uint32_t>(temp);
    }

    // -------------------------------------------------------------------------
    // STEP 2: Non-linear G-Function Computation (RFC 4503 Section 2.6)
    // -------------------------------------------------------------------------
    // Computes G[j] = g(X[j], C[j]) for each of the 8 state variables.
    uint32_t g[8];
    for (int j = 0; j < 8; ++j) {
        g[j] = Utils::gFunc(state_.x[j], state_.c[j]);
    }

    // -------------------------------------------------------------------------
    // STEP 3: State Update with Bitwise Left Rotation (RFC 4503 Section 2.6)
    // -------------------------------------------------------------------------
    // [OPTIMIZATION: Bitwise Rotations & Natural Modulo 2^32 Addition]
    // The state update mixes G-function outputs using left rotation by 8 and 16 bits.
    // All 32-bit additions wrap modulo 2^32 implicitly by CPU hardware rules.
    state_.x[0] = g[0] + Utils::rotl32(g[7], 16) + Utils::rotl32(g[6], 16);
    state_.x[1] = g[1] + Utils::rotl32(g[0], 8)  + g[7];
    state_.x[2] = g[2] + Utils::rotl32(g[1], 16) + Utils::rotl32(g[0], 16);
    state_.x[3] = g[3] + Utils::rotl32(g[2], 8)  + g[1];
    state_.x[4] = g[4] + Utils::rotl32(g[3], 16) + Utils::rotl32(g[2], 16);
    state_.x[5] = g[5] + Utils::rotl32(g[4], 8)  + g[3];
    state_.x[6] = g[6] + Utils::rotl32(g[5], 16) + Utils::rotl32(g[4], 16);
    state_.x[7] = g[7] + Utils::rotl32(g[6], 8)  + g[5];
}

void RabbitCipher::setupKey(const uint8_t key[RABBIT_KEY_SIZE]) {
    // -------------------------------------------------------------------------
    // Key Expansion: Split 128-bit key into eight 16-bit subkeys K0..K7
    // -------------------------------------------------------------------------
    // [RFC 4503 OS2IP Big-Endian Conversion]
    // K0 = key[14..15], K1 = key[12..13], ..., K7 = key[0..1]
    uint16_t k[8];
    for (int i = 0; i < 8; ++i) {
        k[i] = (static_cast<uint16_t>(key[14 - 2 * i]) << 8) |
                static_cast<uint16_t>(key[15 - 2 * i]);
    }

    // [OPTIMIZATION: Bitwise Index Masking `& 7` and Parity Masking `& 1`]
    // Instead of `(j % 2)` and `(j + offset) % 8`, we use `(j & 1)` and `(j + offset) & 7`.
    // Since 8 is a power of 2, bitwise AND replaces multi-cycle integer division.
    for (int j = 0; j < 8; ++j) {
        if ((j & 1) == 0) {
            state_.x[j] = (static_cast<uint32_t>(k[(j + 1) & 7]) << 16) | static_cast<uint32_t>(k[j]);
            state_.c[j] = (static_cast<uint32_t>(k[(j + 4) & 7]) << 16) | static_cast<uint32_t>(k[(j + 5) & 7]);
        } else {
            state_.x[j] = (static_cast<uint32_t>(k[(j + 5) & 7]) << 16) | static_cast<uint32_t>(k[(j + 4) & 7]);
            state_.c[j] = (static_cast<uint32_t>(k[j]) << 16)           | static_cast<uint32_t>(k[(j + 1) & 7]);
        }
    }

    // Reset carry bit b to 0
    state_.carry = 0;

    // [WARMUP PHASE: 4 Iterations for Key Diffusion]
    for (int i = 0; i < 4; ++i) {
        nextState();
    }

    // [COUNTER RE-INITIALIZATION: XOR Counters with Shifted State Variables]
    for (int j = 0; j < 8; ++j) {
        state_.c[j] ^= state_.x[(j + 4) & 7];
    }

    // [OPTIMIZATION: Master State Caching]
    // Cache the post-key-setup state into masterState_. This allows instant re-IV
    // without repeating key derivation and the 4 initial warmup iterations.
    masterState_ = state_;
    hasKey_      = true;
    hasIV_       = false;
    bufferPos_   = RABBIT_BLOCK_SIZE; // Keystream buffer empty, ready for generation
}

void RabbitCipher::setupKey(const std::vector<uint8_t>& key) {
    if (key.size() != RABBIT_KEY_SIZE) {
        throw std::invalid_argument("Rabbit key length must be exactly 16 bytes (128 bits).");
    }
    setupKey(key.data());
}

void RabbitCipher::setupIV(const uint8_t iv[RABBIT_IV_SIZE]) {
    if (!hasKey_) {
        throw std::logic_error("Key must be set up via setupKey() before calling setupIV()!");
    }

    // [RFC 4503 Section 2.4: Always restore from Master State]
    state_ = masterState_;

    // Parse 64-bit IV words according to big-endian OS2IP
    uint32_t iv_31_0  = (static_cast<uint32_t>(iv[4]) << 24) |
                        (static_cast<uint32_t>(iv[5]) << 16) |
                        (static_cast<uint32_t>(iv[6]) << 8)  |
                        static_cast<uint32_t>(iv[7]);

    uint32_t iv_63_32 = (static_cast<uint32_t>(iv[0]) << 24) |
                        (static_cast<uint32_t>(iv[1]) << 16) |
                        (static_cast<uint32_t>(iv[2]) << 8)  |
                        static_cast<uint32_t>(iv[3]);

    uint32_t iv_63_48 = (static_cast<uint32_t>(iv[0]) << 8) | static_cast<uint32_t>(iv[1]);
    uint32_t iv_47_32 = (static_cast<uint32_t>(iv[2]) << 8) | static_cast<uint32_t>(iv[3]);
    uint32_t iv_31_16 = (static_cast<uint32_t>(iv[4]) << 8) | static_cast<uint32_t>(iv[5]);
    uint32_t iv_15_0  = (static_cast<uint32_t>(iv[6]) << 8) | static_cast<uint32_t>(iv[7]);

    // XOR IV bits into Counter system
    state_.c[0] ^= iv_31_0;
    state_.c[1] ^= (iv_63_48 << 16) | iv_31_16;
    state_.c[2] ^= iv_63_32;
    state_.c[3] ^= (iv_47_32 << 16) | iv_15_0;
    state_.c[4] ^= iv_31_0;
    state_.c[5] ^= (iv_63_48 << 16) | iv_31_16;
    state_.c[6] ^= iv_63_32;
    state_.c[7] ^= (iv_47_32 << 16) | iv_15_0;

    // [WARMUP PHASE: 4 Iterations for IV Diffusion]
    for (int i = 0; i < 4; ++i) {
        nextState();
    }

    hasIV_     = true;
    bufferPos_ = RABBIT_BLOCK_SIZE; // Flush stale keystream buffer
}

void RabbitCipher::setupIV(const std::vector<uint8_t>& iv) {
    if (iv.size() != RABBIT_IV_SIZE) {
        throw std::invalid_argument("Rabbit IV length must be exactly 8 bytes (64 bits).");
    }
    setupIV(iv.data());
}

void RabbitCipher::resetToMaster() {
    if (!hasKey_) {
        throw std::logic_error("No master key has been set to restore from!");
    }
    state_     = masterState_;
    hasIV_     = false;
    bufferPos_ = RABBIT_BLOCK_SIZE;
}

void RabbitCipher::generateBlock(uint8_t output[RABBIT_BLOCK_SIZE]) {
    // Advance to next state
    nextState();

    // -------------------------------------------------------------------------
    // Extraction Scheme: Extract 128-bit keystream S[127..0] (RFC 4503 Section 2.7)
    // -------------------------------------------------------------------------
    uint16_t s7 = static_cast<uint16_t>((state_.x[6] >> 16) ^ state_.x[1]);
    uint16_t s6 = static_cast<uint16_t>(state_.x[6] ^ (state_.x[3] >> 16));
    uint16_t s5 = static_cast<uint16_t>((state_.x[4] >> 16) ^ state_.x[7]);
    uint16_t s4 = static_cast<uint16_t>(state_.x[4] ^ (state_.x[1] >> 16));
    uint16_t s3 = static_cast<uint16_t>((state_.x[2] >> 16) ^ state_.x[5]);
    uint16_t s2 = static_cast<uint16_t>(state_.x[2] ^ (state_.x[7] >> 16));
    uint16_t s1 = static_cast<uint16_t>((state_.x[0] >> 16) ^ state_.x[3]);
    uint16_t s0 = static_cast<uint16_t>(state_.x[0] ^ (state_.x[5] >> 16));

    // [RFC 4503 I2OSP Big-Endian Output Formatting]
    // S[127..112] is placed at output[0..1], S[15..0] is placed at output[14..15].
    output[0]  = static_cast<uint8_t>(s7 >> 8);
    output[1]  = static_cast<uint8_t>(s7);
    output[2]  = static_cast<uint8_t>(s6 >> 8);
    output[3]  = static_cast<uint8_t>(s6);
    output[4]  = static_cast<uint8_t>(s5 >> 8);
    output[5]  = static_cast<uint8_t>(s5);
    output[6]  = static_cast<uint8_t>(s4 >> 8);
    output[7]  = static_cast<uint8_t>(s4);
    output[8]  = static_cast<uint8_t>(s3 >> 8);
    output[9]  = static_cast<uint8_t>(s3);
    output[10] = static_cast<uint8_t>(s2 >> 8);
    output[11] = static_cast<uint8_t>(s2);
    output[12] = static_cast<uint8_t>(s1 >> 8);
    output[13] = static_cast<uint8_t>(s1);
    output[14] = static_cast<uint8_t>(s0 >> 8);
    output[15] = static_cast<uint8_t>(s0);
}

void RabbitCipher::process(const uint8_t* input, uint8_t* output, size_t length) {
    if (!hasKey_) {
        throw std::logic_error("Cannot encrypt/decrypt: cipher key is not initialized!");
    }

    // -------------------------------------------------------------------------
    // [OPTIMIZATION: Buffered Keystream Stream Processing]
    // -------------------------------------------------------------------------
    // Maintains an internal 16-byte block cache `buffer_`. Whenever all 16 bytes are
    // consumed (`bufferPos_ >= 16`), a new block is generated. This allows arbitrary-length
    // streaming XORing without padding or discarding remaining keystream bytes.
    for (size_t i = 0; i < length; ++i) {
        if (bufferPos_ >= RABBIT_BLOCK_SIZE) {
            generateBlock(buffer_);
            bufferPos_ = 0;
        }
        output[i] = input[i] ^ buffer_[bufferPos_++];
    }
}

void RabbitCipher::process(std::vector<uint8_t>& data) {
    if (!data.empty()) {
        // [OPTIMIZATION: Zero-copy In-place Encryption]
        // In-place buffer pass: source and destination pointers point to data.data()
        process(data.data(), data.data(), data.size());
    }
}

bool RabbitCipher::isKeySet() const {
    return hasKey_;
}

bool RabbitCipher::isIVSet() const {
    return hasIV_;
}
