#include "rabbit.h"
#include "utils.h"
#include <cstring>
#include <stdexcept>

// Hang so cua bo dem (RFC 4503 muc 2.5).
static const uint32_t A[8] = {
    0x4D34D34D, 0xD34D34D3, 0x34D34D34, 0x4D34D34D,
    0xD34D34D3, 0x34D34D34, 0x4D34D34D, 0xD34D34D3
};

// Doc 2 hoac 4 byte thanh mot so big-endian, vi du {0x12, 0x34} -> 0x1234.
static uint32_t load16(const uint8_t* p) {
    return (p[0] << 8) | p[1];
}

static uint32_t load32(const uint8_t* p) {
    return (load16(p) << 16) | load16(p + 2);
}

// Ghi 16 bit thap cua `value` thanh 2 byte big-endian.
static void store16(uint8_t* p, uint32_t value) {
    p[0] = value >> 8;
    p[1] = value;
}

// Thuat toan ben duoi dung thu tu byte cua RFC 4503. Bai bao FSE 2003 dung thu tu
// nguoc lai hoan toan, nen o che do FSE chi can dao nguoc khoa, IV va dau ra.
static void copyInByteOrder(EndianMode mode, const uint8_t* in, uint8_t* out, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        out[i] = (mode == EndianMode::FSE2003_LE) ? in[n - 1 - i] : in[i];
    }
}

//Tao ham gFunc va rotl32 duoc dinh nghia trong file utils.h, duoc su dung de thuc hien cac phep toan tren cac tu 32-bit trong cipher.
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
    // 1. Cap nhat bo dem: C[j] = C[j] + A[j] + carry.
    //    Cong bang so 64 bit; phan tran qua 32 bit chinh la carry tiep theo.
    for (int j = 0; j < 8; ++j) {
        uint64_t sum = state_.c[j];
        sum += A[j];
        sum += state_.carry;
        state_.c[j]  = static_cast<uint32_t>(sum);
        state_.carry = static_cast<uint32_t>(sum >> 32);
    }

    // 2. Tinh ham g (phi tuyen) cho tung tu.
    uint32_t g[8];
    for (int j = 0; j < 8; ++j) {
        g[j] = Utils::gFunc(state_.x[j], state_.c[j]);
    }

    // 3. Trang thai moi (RFC 4503 muc 2.6). Phep cong tu dong lay mod 2^32.
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
    uint8_t keyBytes[RABBIT_KEY_SIZE];
    copyInByteOrder(endianMode_, key, keyBytes, RABBIT_KEY_SIZE);

    // Chia khoa thanh 8 tu 16 bit.
    // K0 = 2 byte cuoi, K1 = 2 byte lien truoc, ..., K7 = 2 byte dau.
    uint32_t k[8];
    for (int i = 0; i < 8; ++i) {
        k[i] = load16(keyBytes + 14 - 2 * i);
    }

    // Nap trang thai va bo dem tu cac tu khoa (RFC 4503 muc 2.3).
    for (int j = 0; j < 8; ++j) {
        if (j % 2 == 0) {
            state_.x[j] = (k[(j + 1) % 8] << 16) | k[j];
            state_.c[j] = (k[(j + 4) % 8] << 16) | k[(j + 5) % 8];
        } else {
            state_.x[j] = (k[(j + 5) % 8] << 16) | k[(j + 4) % 8];
            state_.c[j] = (k[j] << 16)           | k[(j + 1) % 8];
        }
    }

    // Chay 4 vong, roi tron trang thai nguoc vao bo dem.
    state_.carry = 0;
    for (int i = 0; i < 4; ++i) {
        nextState();
    }
    for (int j = 0; j < 8; ++j) {
        state_.c[j] ^= state_.x[(j + 4) % 8];
    }

    // Luu trang thai nay de setupIV() bat dau tu day, khong phai nap lai khoa.
    masterState_ = state_;
    hasKey_      = true;
    hasIV_       = false;
    bufferPos_   = RABBIT_BLOCK_SIZE; // Bo dem keystream rong
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

    // Luon bat dau tu trang thai chi co khoa (RFC 4503 muc 2.4).
    state_ = masterState_;

    uint8_t ivBytes[RABBIT_IV_SIZE];
    copyInByteOrder(endianMode_, iv, ivBytes, RABBIT_IV_SIZE);

    // 4 tu 32 bit tao tu IV 64 bit, XOR vao C0..C3 va lap lai cho C4..C7.
    const uint32_t ivWords[4] = {
        load32(ivBytes + 4),                                // Bit 31..0 cua IV
        (load16(ivBytes) << 16)     | load16(ivBytes + 4),  // Bit 63..48 va 31..16 cua IV
        load32(ivBytes),                                    // Bit 63..32 cua IV
        (load16(ivBytes + 2) << 16) | load16(ivBytes + 6)   // Bit 47..32 va 15..0 cua IV
    };
    for (int j = 0; j < 8; ++j) {
        state_.c[j] ^= ivWords[j % 4];
    }

    // Chay 4 vong de IV lan khap trang thai.
    for (int i = 0; i < 4; ++i) {
        nextState();
    }

    hasIV_     = true;
    bufferPos_ = RABBIT_BLOCK_SIZE; // Bo keystream cu
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
    nextState();

    // 8 tu dau ra 16 bit s0..s7 (RFC 4503 muc 2.7).
    // Chi dung 16 bit thap cua moi gia tri.
    const uint32_t* x = state_.x;
    uint32_t s[8];
    s[0] = x[0] ^ (x[5] >> 16);
    s[1] = (x[0] >> 16) ^ x[3];
    s[2] = x[2] ^ (x[7] >> 16);
    s[3] = (x[2] >> 16) ^ x[5];
    s[4] = x[4] ^ (x[1] >> 16);
    s[5] = (x[4] >> 16) ^ x[7];
    s[6] = x[6] ^ (x[3] >> 16);
    s[7] = (x[6] >> 16) ^ x[1];

    // Xuat big-endian: s7 dung dau, s0 dung cuoi.
    uint8_t block[RABBIT_BLOCK_SIZE];
    for (int i = 0; i < 8; ++i) {
        store16(block + 2 * i, s[7 - i]);
    }
    copyInByteOrder(endianMode_, block, output, RABBIT_BLOCK_SIZE);
}

void RabbitCipher::process(const uint8_t* input, uint8_t* output, size_t length) {
    if (!hasKey_) {
        throw std::logic_error("Cannot encrypt/decrypt: cipher key is not initialized!");
    }

    // XOR tung byte; cu het 16 byte thi sinh khoi keystream moi.
    // Keystream con thua duoc giu lai cho lan goi sau.
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
        process(data.data(), data.data(), data.size()); // Xu ly tai cho
    }
}

bool RabbitCipher::isKeySet() const {
    return hasKey_;
}

bool RabbitCipher::isIVSet() const {
    return hasIV_;
}
