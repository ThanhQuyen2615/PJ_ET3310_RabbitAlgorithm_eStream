#include "rabbit.h"
#include "utils.h"
#include <cstring>
#include <stdexcept>

// [ALGORITHM SPECIFICATION: RFC 4503 Section 2.5 / FSE 2003 Section 2.1]
// Fixed counter constants A[0..7]. These asymmetric constants guarantee that the counter
// periods do not synchronize and prevent slide attacks or related-key attacks.
static const uint32_t A[8] = {
    0x4D34D34D, 0xD34D34D3, 0x34D34D34, 0x4D34D34D,
    0xD34D34D3, 0x34D34D34, 0x4D34D34D, 0xD34D34D3
};

RabbitCipher::RabbitCipher(EndianMode mode)
    : hasKey_(false), hasIV_(false), endianMode_(mode), bufferPos_(RABBIT_BLOCK_SIZE) {
    std::memset(&state_, 0, sizeof(state_));
    std::memset(&masterState_, 0, sizeof(masterState_));
    std::memset(buffer_, 0, sizeof(buffer_));
}

void RabbitCipher::setEndianMode(EndianMode mode) {
    endianMode_ = mode;
}

EndianMode RabbitCipher::getEndianMode() const {
    return endianMode_;
}

void RabbitCipher::nextState() {
    // -------------------------------------------------------------------------
    // STEP 1: Counter System Update (RFC 4503 Section 2.5 / FSE 2003)
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
    // STEP 2: Non-linear G-Function Computation (RFC 4503 Section 2.6 / FSE 2003)
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
    if (endianMode_ == EndianMode::FSE2003_LE) {
        // ---------------------------------------------------------------------
        // FSE 2003 Little-Endian Key Setup (PDF 978-3-540-39887-5_23.pdf App A)
        // ---------------------------------------------------------------------
        uint32_t k0 = static_cast<uint32_t>(key[0])  | (static_cast<uint32_t>(key[1])  << 8) |
                      (static_cast<uint32_t>(key[2])  << 16) | (static_cast<uint32_t>(key[3])  << 24);
        uint32_t k1 = static_cast<uint32_t>(key[4])  | (static_cast<uint32_t>(key[5])  << 8) |
                      (static_cast<uint32_t>(key[6])  << 16) | (static_cast<uint32_t>(key[7])  << 24);
        uint32_t k2 = static_cast<uint32_t>(key[8])  | (static_cast<uint32_t>(key[9])  << 8) |
                      (static_cast<uint32_t>(key[10]) << 16) | (static_cast<uint32_t>(key[11]) << 24);
        uint32_t k3 = static_cast<uint32_t>(key[12]) | (static_cast<uint32_t>(key[13]) << 8) |
                      (static_cast<uint32_t>(key[14]) << 16) | (static_cast<uint32_t>(key[15]) << 24);

        state_.x[0] = k0;
        state_.x[2] = k1;
        state_.x[4] = k2;
        state_.x[6] = k3;
        state_.x[1] = (k3 << 16) | (k2 >> 16);
        state_.x[3] = (k0 << 16) | (k3 >> 16);
        state_.x[5] = (k1 << 16) | (k0 >> 16);
        state_.x[7] = (k2 << 16) | (k1 >> 16);

        state_.c[0] = Utils::rotl32(k2, 16);
        state_.c[2] = Utils::rotl32(k3, 16);
        state_.c[4] = Utils::rotl32(k0, 16);
        state_.c[6] = Utils::rotl32(k1, 16);
        state_.c[1] = (k0 & 0xFFFF0000) | (k1 & 0x0000FFFF);
        state_.c[3] = (k1 & 0xFFFF0000) | (k2 & 0x0000FFFF);
        state_.c[5] = (k2 & 0xFFFF0000) | (k3 & 0x0000FFFF);
        state_.c[7] = (k3 & 0xFFFF0000) | (k0 & 0x0000FFFF);

        state_.carry = 0;
        for (int i = 0; i < 4; ++i) nextState();
        for (int i = 0; i < 8; ++i) state_.c[(i + 4) & 7] ^= state_.x[i];
    } else {
        // ---------------------------------------------------------------------
        // RFC 4503 Big-Endian OS2IP Key Setup (RFC 4503 Section 2.3)
        // ---------------------------------------------------------------------
        // K0 = key[14..15], K1 = key[12..13], ..., K7 = key[0..1]
        uint16_t k[8];
        for (int i = 0; i < 8; ++i) {
            k[i] = (static_cast<uint16_t>(key[14 - 2 * i]) << 8) |
                    static_cast<uint16_t>(key[15 - 2 * i]);
        }

        // [OPTIMIZATION: Bitwise Index Masking `& 7` and Parity Masking `& 1`]
        for (int j = 0; j < 8; ++j) {
            if ((j & 1) == 0) {
                state_.x[j] = (static_cast<uint32_t>(k[(j + 1) & 7]) << 16) | static_cast<uint32_t>(k[j]);
                state_.c[j] = (static_cast<uint32_t>(k[(j + 4) & 7]) << 16) | static_cast<uint32_t>(k[(j + 5) & 7]);
            } else {
                state_.x[j] = (static_cast<uint32_t>(k[(j + 5) & 7]) << 16) | static_cast<uint32_t>(k[(j + 4) & 7]);
                state_.c[j] = (static_cast<uint32_t>(k[j]) << 16)           | static_cast<uint32_t>(k[(j + 1) & 7]);
            }
        }

        state_.carry = 0;
        for (int i = 0; i < 4; ++i) nextState();
        for (int j = 0; j < 8; ++j) state_.c[j] ^= state_.x[(j + 4) & 7];
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

    // Parse 64-bit IV words
    uint32_t iv_31_0, iv_63_32, iv_63_48, iv_47_32, iv_31_16, iv_15_0;

    if (endianMode_ == EndianMode::FSE2003_LE) {
        // Little-endian IV mapping
        iv_31_0  = static_cast<uint32_t>(iv[0]) | (static_cast<uint32_t>(iv[1]) << 8) |
                   (static_cast<uint32_t>(iv[2]) << 16) | (static_cast<uint32_t>(iv[3]) << 24);
        iv_63_32 = static_cast<uint32_t>(iv[4]) | (static_cast<uint32_t>(iv[5]) << 8) |
                   (static_cast<uint32_t>(iv[6]) << 16) | (static_cast<uint32_t>(iv[7]) << 24);
        iv_63_48 = static_cast<uint32_t>(iv[6]) | (static_cast<uint32_t>(iv[7]) << 8);
        iv_47_32 = static_cast<uint32_t>(iv[4]) | (static_cast<uint32_t>(iv[5]) << 8);
        iv_31_16 = static_cast<uint32_t>(iv[2]) | (static_cast<uint32_t>(iv[3]) << 8);
        iv_15_0  = static_cast<uint32_t>(iv[0]) | (static_cast<uint32_t>(iv[1]) << 8);
    } else {
        // RFC 4503 OS2IP big-endian mapping
        iv_31_0  = (static_cast<uint32_t>(iv[4]) << 24) | (static_cast<uint32_t>(iv[5]) << 16) |
                   (static_cast<uint32_t>(iv[6]) << 8)  | static_cast<uint32_t>(iv[7]);
        iv_63_32 = (static_cast<uint32_t>(iv[0]) << 24) | (static_cast<uint32_t>(iv[1]) << 16) |
                   (static_cast<uint32_t>(iv[2]) << 8)  | static_cast<uint32_t>(iv[3]);
        iv_63_48 = (static_cast<uint32_t>(iv[0]) << 8) | static_cast<uint32_t>(iv[1]);
        iv_47_32 = (static_cast<uint32_t>(iv[2]) << 8) | static_cast<uint32_t>(iv[3]);
        iv_31_16 = (static_cast<uint32_t>(iv[4]) << 8) | static_cast<uint32_t>(iv[5]);
        iv_15_0  = (static_cast<uint32_t>(iv[6]) << 8) | static_cast<uint32_t>(iv[7]);
    }

    // XOR IV bits into Counter system
    state_.c[0] ^= iv_31_0;
    state_.c[1] ^= (iv_63_48 << 16) | iv_31_16;
    state_.c[2] ^= iv_63_32;
    state_.c[3] ^= (iv_47_32 << 16) | iv_15_0;
    state_.c[4] ^= iv_31_0;
    state_.c[5] ^= (iv_63_48 << 16) | iv_31_16;
    state_.c[6] ^= iv_63_32;
    state_.c[7] ^= (iv_47_32 << 16) | iv_15_0;

    // 4 Iterations for IV Diffusion
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

    if (endianMode_ == EndianMode::FSE2003_LE) {
        // ---------------------------------------------------------------------
        // FSE 2003 Little-Endian Extraction (PDF 978-3-540-39887-5_23.pdf App A)
        // ---------------------------------------------------------------------
        uint32_t w0 = state_.x[0] ^ (state_.x[5] >> 16) ^ (state_.x[3] << 16);
        uint32_t w1 = state_.x[2] ^ (state_.x[7] >> 16) ^ (state_.x[5] << 16);
        uint32_t w2 = state_.x[4] ^ (state_.x[1] >> 16) ^ (state_.x[7] << 16);
        uint32_t w3 = state_.x[6] ^ (state_.x[3] >> 16) ^ (state_.x[1] << 16);

        output[0]  = static_cast<uint8_t>(w0);
        output[1]  = static_cast<uint8_t>(w0 >> 8);
        output[2]  = static_cast<uint8_t>(w0 >> 16);
        output[3]  = static_cast<uint8_t>(w0 >> 24);
        output[4]  = static_cast<uint8_t>(w1);
        output[5]  = static_cast<uint8_t>(w1 >> 8);
        output[6]  = static_cast<uint8_t>(w1 >> 16);
        output[7]  = static_cast<uint8_t>(w1 >> 24);
        output[8]  = static_cast<uint8_t>(w2);
        output[9]  = static_cast<uint8_t>(w2 >> 8);
        output[10] = static_cast<uint8_t>(w2 >> 16);
        output[11] = static_cast<uint8_t>(w2 >> 24);
        output[12] = static_cast<uint8_t>(w3);
        output[13] = static_cast<uint8_t>(w3 >> 8);
        output[14] = static_cast<uint8_t>(w3 >> 16);
        output[15] = static_cast<uint8_t>(w3 >> 24);
    } else {
        // ---------------------------------------------------------------------
        // RFC 4503 Big-Endian I2OSP Extraction (RFC 4503 Section 2.7)
        // ---------------------------------------------------------------------
        uint16_t s7 = static_cast<uint16_t>((state_.x[6] >> 16) ^ state_.x[1]);
        uint16_t s6 = static_cast<uint16_t>(state_.x[6] ^ (state_.x[3] >> 16));
        uint16_t s5 = static_cast<uint16_t>((state_.x[4] >> 16) ^ state_.x[7]);
        uint16_t s4 = static_cast<uint16_t>(state_.x[4] ^ (state_.x[1] >> 16));
        uint16_t s3 = static_cast<uint16_t>((state_.x[2] >> 16) ^ state_.x[5]);
        uint16_t s2 = static_cast<uint16_t>(state_.x[2] ^ (state_.x[7] >> 16));
        uint16_t s1 = static_cast<uint16_t>((state_.x[0] >> 16) ^ state_.x[3]);
        uint16_t s0 = static_cast<uint16_t>(state_.x[0] ^ (state_.x[5] >> 16));

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
}

void RabbitCipher::process(const uint8_t* input, uint8_t* output, size_t length) {
    if (!hasKey_) {
        throw std::logic_error("Cannot encrypt/decrypt: cipher key is not initialized!");
    }

    // -------------------------------------------------------------------------
    // [OPTIMIZATION: Buffered Keystream Stream Processing]
    // -------------------------------------------------------------------------
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
        process(data.data(), data.data(), data.size());
    }
}

bool RabbitCipher::isKeySet() const {
    return hasKey_;
}

bool RabbitCipher::isIVSet() const {
    return hasIV_;
}
