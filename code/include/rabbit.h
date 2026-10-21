#pragma once

#include <cstdint>
#include <cstddef>
#include <ostream>

// Thu tu byte khi doi 16 byte khoa / 8 byte IV / 16 byte keystream <-> cac tu 32 bit.
// Hai cach chi khac nhau o cach viet byte; trang thai ben trong giong het nhau.
enum class ByteOrder {
    RFC,   // RFC 4503: byte cao viet truoc (big-endian)
    PDF    // Bai bao FSE 2003 (Phu luc A, B): byte thap viet truoc (little-endian)
};

// Trang thai trong: 8 tu trang thai + 8 bo dem + 1 bit nho = 513 bit.
struct RabbitState {
    uint32_t x[8];    // Bien trang thai X0..X7
    uint32_t c[8];    // Bien dem C0..C7
    uint32_t carry;   // Bit nho cua bo dem (0 hoac 1)
};

// Ma dong Rabbit. Ma hoa va giai ma la cung mot phep toan.
// Thu tu goi: setupKey() -> setupIV() (khong bat buoc) -> process().
class RabbitCipher {
public:
    explicit RabbitCipher(ByteOrder byteOrder = ByteOrder::RFC);

    // Ghi tung buoc tinh toan vao `out` (vd mot file .txt). Chi ghi chi tiet
    // `maxBlocks` khoi keystream dau tien de file log khong qua lon.
    void setLog(std::ostream* out, int maxBlocks = 4);

    // Nap khoa 16 byte.
    void setupKey(const uint8_t key[16]);

    // Tron IV 8 byte vao trang thai. Goi sau setupKey().
    void setupIV(const uint8_t iv[8]);

    // Sinh khoi keystream 16 byte tiep theo.
    void generateBlock(uint8_t output[16]);

    // XOR du lieu voi keystream. input va output co the la cung mot mang.
    void process(const uint8_t* input, uint8_t* output, size_t length);

private:
    ByteOrder order;
    RabbitState state;          // Trang thai hien tai
    RabbitState master;         // Trang thai luu lai sau setupKey(), setupIV() bat dau tu day
    uint8_t keystream[16];      // Khoi keystream dang dung
    int used = 16;              // So byte da dung trong keystream (16 = da dung het)

    std::ostream* log = nullptr; // Noi ghi log (nullptr = khong ghi)
    int maxLogBlocks = 4;        // So khoi keystream dau tien duoc ghi chi tiet
    int rounds = 0;              // So lan da chay next_state
    int blocks = 0;              // So khoi keystream da sinh

    // Cap nhat bo dem va trang thai them mot vong.
    void nextState();

    // Co dang ghi log chi tiet khong.
    bool logging() const;

    // Ghi X0..X7, C0..C7, carry vao log.
    void logState() const;
};
