#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

constexpr size_t RABBIT_KEY_SIZE   = 16;  // Khoa 128 bit
constexpr size_t RABBIT_IV_SIZE    = 8;   // IV 64 bit
constexpr size_t RABBIT_BLOCK_SIZE = 16;  // Khoi keystream 128 bit

// Thu tu byte khi nap khoa/IV va khi xuat keystream.
enum class EndianMode {
    RFC4503,      // RFC 4503, big-endian (mac dinh)
    FSE2003_LE    // Bai bao goc FSE 2003, little-endian
};

// Trang thai trong: 8 tu trang thai + 8 bo dem + 1 bit nho = 513 bit.
struct RabbitState {
    uint32_t x[8];    // Bien trang thai X0..X7
    uint32_t c[8];    // Bien dem C0..C7
    uint32_t carry;   // Bit nho cua bo dem (0 hoac 1)
};

// Ma dong Rabbit. Ma hoa va giai ma la cung mot phep toan.
class RabbitCipher {
public:
    explicit RabbitCipher(EndianMode mode = EndianMode::RFC4503);
    ~RabbitCipher() = default;

    void setEndianMode(EndianMode mode);
    EndianMode getEndianMode() const;

    // Nap khoa 16 byte. Phai goi dau tien.
    void setupKey(const uint8_t key[RABBIT_KEY_SIZE]);
    void setupKey(const std::vector<uint8_t>& key);

    // Tron IV 8 byte vao trang thai da nap khoa (khong bat buoc).
    void setupIV(const uint8_t iv[RABBIT_IV_SIZE]);
    void setupIV(const std::vector<uint8_t>& iv);

    // Quay ve trang thai ngay sau setupKey().
    void resetToMaster();

    // Sinh khoi keystream 16 byte tiep theo.
    void generateBlock(uint8_t output[RABBIT_BLOCK_SIZE]);

    // XOR du lieu voi keystream. input va output co the la cung mot vung nho.
    void process(const uint8_t* input, uint8_t* output, size_t length);
    void process(std::vector<uint8_t>& data);

    bool isKeySet() const;
    bool isIVSet() const;

private:
    RabbitState state_;       // Trang thai hien tai
    RabbitState masterState_; // Trang thai luu lai sau setupKey()
    bool hasKey_;
    bool hasIV_;
    EndianMode endianMode_;

    uint8_t buffer_[RABBIT_BLOCK_SIZE]; // Khoi keystream hien tai
    size_t bufferPos_;                  // Vi tri byte chua dung tiep theo trong buffer_

    // Cap nhat bo dem va trang thai them mot vong.
    void nextState();
};
