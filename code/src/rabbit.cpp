#include "rabbit.h"
#include "utils.h" // gFunc va rotl32: cac phep toan tren tu 32 bit
#include <cstdio>
#include <string>

// Hang so cua bo dem (RFC 4503 muc 2.5).
static const uint32_t A[8] = {
    0x4D34D34D, 0xD34D34D3, 0x34D34D34, 0x4D34D34D,
    0xD34D34D3, 0x34D34D34, 0x4D34D34D, 0xD34D34D3
};

// ===================== Buoc 1 va Buoc 3: doi byte <-> tu 32 bit =====================

// Doi mang byte thanh `count` tu 32 bit (tu 0 la tu thap nhat).
// PDF: moi 4 byte lien tiep la mot tu, byte thap truoc; tu 0 nam o dau mang.
// RFC: ca mang la mot so lon viet byte cao truoc; tu 0 nam o 4 byte cuoi mang.
static void bytesToWords(ByteOrder order, const uint8_t* bytes, uint32_t* words, int count) {
    for (int i = 0; i < count; ++i) {
        if (order == ByteOrder::PDF) {
            const uint8_t* p = bytes + 4 * i;
            words[i] = p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
        } else {
            const uint8_t* p = bytes + 4 * (count - 1 - i);
            words[i] = ((uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
        }
    }
}

// Nguoc lai cua bytesToWords: doi `count` tu 32 bit thanh mang byte.
static void wordsToBytes(ByteOrder order, const uint32_t* words, uint8_t* bytes, int count) {
    for (int i = 0; i < count; ++i) {
        uint32_t w = words[i];
        if (order == ByteOrder::PDF) {
            uint8_t* p = bytes + 4 * i;
            p[0] = w;
            p[1] = w >> 8;
            p[2] = w >> 16;
            p[3] = w >> 24;
        } else {
            uint8_t* p = bytes + 4 * (count - 1 - i);
            p[0] = w >> 24;
            p[1] = w >> 16;
            p[2] = w >> 8;
            p[3] = w;
        }
    }
}

// ===================== Ham ho tro ghi log =====================

// Mot tu 32 bit dang hex 8 chu so, vd "4D34D34D".
static std::string hex32(uint32_t value) {
    char text[9];
    std::snprintf(text, sizeof(text), "%08X", value);
    return text;
}

// Day tu 32 bit cach nhau dau cach.
static std::string hexWords(const uint32_t* words, int count) {
    std::string text;
    for (int i = 0; i < count; ++i) {
        if (i > 0) text += " ";
        text += hex32(words[i]);
    }
    return text;
}

// Day byte cach nhau dau cach, vd "02 F7 4A".
static std::string hexBytes(const uint8_t* bytes, int count) {
    std::string text;
    for (int i = 0; i < count; ++i) {
        if (i > 0) text += " ";
        text += Utils::bytesToHex(bytes + i, 1);
    }
    return text;
}

static const char* orderName(ByteOrder order) {
    return order == ByteOrder::PDF ? "PDF (byte thap truoc)" : "RFC (byte cao truoc)";
}

RabbitCipher::RabbitCipher(ByteOrder byteOrder) {
    order = byteOrder;
}

void RabbitCipher::setLog(std::ostream* out, int maxBlocks) {
    log = out;
    maxLogBlocks = maxBlocks;
}

bool RabbitCipher::logging() const {
    return log != nullptr && blocks <= maxLogBlocks;
}

void RabbitCipher::logState() const {
    *log << "    X0..X7 : " << hexWords(state.x, 8) << "\n";
    *log << "    C0..C7 : " << hexWords(state.c, 8) << "\n";
    *log << "    carry  : " << state.carry << "\n";
}

// ===================== Thuat toan =====================

void RabbitCipher::nextState() {
    rounds++;

    // 1. Cap nhat bo dem: C[j] = C[j] + A[j] + carry.
    //    Cong bang so 64 bit; phan tran qua 32 bit chinh la carry tiep theo.
    for (int j = 0; j < 8; ++j) {
        uint64_t sum = state.c[j];
        sum += A[j];
        sum += state.carry;
        state.c[j]  = sum;          // Giu 32 bit thap
        state.carry = sum >> 32;    // Bit thu 33
    }

    // 2. Tinh ham g (phi tuyen) cho tung tu.
    uint32_t g[8];
    for (int j = 0; j < 8; ++j) {
        g[j] = Utils::gFunc(state.x[j], state.c[j]);
    }

    // 3. Trang thai moi (PDF Phu luc A / RFC 4503 muc 2.6). Phep cong tu dong lay mod 2^32.
    state.x[0] = g[0] + Utils::rotl32(g[7], 16) + Utils::rotl32(g[6], 16);
    state.x[1] = g[1] + Utils::rotl32(g[0], 8)  + g[7];
    state.x[2] = g[2] + Utils::rotl32(g[1], 16) + Utils::rotl32(g[0], 16);
    state.x[3] = g[3] + Utils::rotl32(g[2], 8)  + g[1];
    state.x[4] = g[4] + Utils::rotl32(g[3], 16) + Utils::rotl32(g[2], 16);
    state.x[5] = g[5] + Utils::rotl32(g[4], 8)  + g[3];
    state.x[6] = g[6] + Utils::rotl32(g[5], 16) + Utils::rotl32(g[4], 16);
    state.x[7] = g[7] + Utils::rotl32(g[6], 8)  + g[5];

    if (logging()) {
        *log << "  -- next_state lan " << rounds << " --\n";
        *log << "    Bo dem moi C0..C7 : " << hexWords(state.c, 8) << "   carry = " << state.carry << "\n";
        *log << "    g0..g7            : " << hexWords(g, 8) << "\n";
        *log << "    Trang thai moi X  : " << hexWords(state.x, 8) << "\n";
    }
}

void RabbitCipher::setupKey(const uint8_t key[16]) {
    rounds = 0;
    blocks = 0;

    // Buoc 1: doi 16 byte khoa thanh 4 tu 32 bit k0..k3.
    uint32_t k[4];
    bytesToWords(order, key, k, 4);
    uint32_t k0 = k[0], k1 = k[1], k2 = k[2], k3 = k[3];

    if (logging()) {
        *log << "\n================ KEY SETUP (key_setup - PDF Phu luc A) ================\n";
        *log << "[Buoc 1] Doi khoa thanh 4 tu 32 bit, thu tu byte " << orderName(order) << "\n";
        *log << "    key (16 byte) : " << hexBytes(key, 16) << "\n";
        *log << "    k0 k1 k2 k3   : " << hexWords(k, 4) << "\n";
    }

    // Buoc 2: tao trang thai ban dau tu khoa.
    state.x[0] = k0;
    state.x[2] = k1;
    state.x[4] = k2;
    state.x[6] = k3;
    state.x[1] = (k3 << 16) | (k2 >> 16);
    state.x[3] = (k0 << 16) | (k3 >> 16);
    state.x[5] = (k1 << 16) | (k0 >> 16);
    state.x[7] = (k2 << 16) | (k1 >> 16);

    // Tao bo dem ban dau tu khoa.
    state.c[0] = Utils::rotl32(k2, 16);
    state.c[2] = Utils::rotl32(k3, 16);
    state.c[4] = Utils::rotl32(k0, 16);
    state.c[6] = Utils::rotl32(k1, 16);
    state.c[1] = (k0 & 0xFFFF0000) | (k1 & 0x0000FFFF);
    state.c[3] = (k1 & 0xFFFF0000) | (k2 & 0x0000FFFF);
    state.c[5] = (k2 & 0xFFFF0000) | (k3 & 0x0000FFFF);
    state.c[7] = (k3 & 0xFFFF0000) | (k0 & 0x0000FFFF);
    state.carry = 0;

    if (logging()) {
        *log << "[Buoc 2] Trang thai ban dau tao tu khoa\n";
        logState();
        *log << "  Chay next_state 4 lan:\n";
    }

    // Chay 4 vong, roi tron trang thai vao bo dem.
    for (int i = 0; i < 4; ++i) {
        nextState();
    }
    for (int i = 0; i < 8; ++i) {
        state.c[(i + 4) % 8] ^= state.x[i];
    }

    if (logging()) {
        *log << "  Sua bo dem C[(i+4) mod 8] ^= X[i]  =>  master state:\n";
        logState();
    }

    master = state; // Luu lai de setupIV() dung
    used = 16;      // Chua co keystream
}

void RabbitCipher::setupIV(const uint8_t iv[8]) {
    blocks = 0;

    // Luon bat dau tu trang thai ngay sau setupKey() (RFC 4503 muc 2.4).
    state = master;

    // Buoc 1: doi 8 byte IV thanh 2 tu 32 bit: v0 = bit 31..0, v1 = bit 63..32.
    uint32_t v[2];
    bytesToWords(order, iv, v, 2);

    // 4 tu dung de XOR vao bo dem.
    uint32_t i0 = v[0];                                 // IV bit 31..0
    uint32_t i1 = (v[1] & 0xFFFF0000) | (v[0] >> 16);   // IV bit 63..48 va 31..16
    uint32_t i2 = v[1];                                 // IV bit 63..32
    uint32_t i3 = (v[1] << 16) | (v[0] & 0x0000FFFF);   // IV bit 47..32 va 15..0

    state.c[0] ^= i0;
    state.c[1] ^= i1;
    state.c[2] ^= i2;
    state.c[3] ^= i3;
    state.c[4] ^= i0;
    state.c[5] ^= i1;
    state.c[6] ^= i2;
    state.c[7] ^= i3;

    if (logging()) {
        uint32_t ivWords[4] = {i0, i1, i2, i3};
        *log << "\n================ IV SETUP (RFC 4503 muc 2.4) ================\n";
        *log << "[Buoc 1] Doi IV thanh 2 tu 32 bit, thu tu byte " << orderName(order) << "\n";
        *log << "    iv (8 byte)   : " << hexBytes(iv, 8) << "\n";
        *log << "    v0 v1         : " << hexWords(v, 2) << "\n";
        *log << "[Buoc 2] Bat dau tu master state, XOR vao bo dem:\n";
        *log << "    i0 i1 i2 i3   : " << hexWords(ivWords, 4) << "\n";
        *log << "    C0..C7 moi    : " << hexWords(state.c, 8) << "\n";
        *log << "  Chay next_state 4 lan:\n";
    }

    // Chay 4 vong de IV lan khap trang thai.
    for (int i = 0; i < 4; ++i) {
        nextState();
    }

    if (logging()) {
        *log << "  Trang thai sau IV setup:\n";
        logState();
    }

    used = 16; // Bo keystream cu
}

void RabbitCipher::generateBlock(uint8_t output[16]) {
    blocks++;
    if (log != nullptr && blocks == maxLogBlocks + 1) {
        *log << "\n(Tu khoi keystream thu " << blocks
             << " tro di khong ghi chi tiet de file log khong qua lon)\n";
    }
    if (logging()) {
        *log << "\n================ KHOI KEYSTREAM s[" << blocks - 1
             << "] (cipher - PDF Phu luc A) ================\n";
    }

    nextState();

    // Buoc 2: 4 tu dau ra w0..w3 tinh tu trang thai X (PDF Phu luc A).
    uint32_t* x = state.x;
    uint32_t w[4];
    w[0] = x[0] ^ (x[5] >> 16) ^ (x[3] << 16);
    w[1] = x[2] ^ (x[7] >> 16) ^ (x[5] << 16);
    w[2] = x[4] ^ (x[1] >> 16) ^ (x[7] << 16);
    w[3] = x[6] ^ (x[3] >> 16) ^ (x[1] << 16);

    // Buoc 3: doi 4 tu thanh 16 byte theo thu tu byte da chon.
    wordsToBytes(order, w, output, 4);

    if (logging()) {
        uint8_t pdfBytes[16], rfcBytes[16];
        wordsToBytes(ByteOrder::PDF, w, pdfBytes, 4);
        wordsToBytes(ByteOrder::RFC, w, rfcBytes, 4);
        *log << "[Buoc 2] w0 w1 w2 w3       : " << hexWords(w, 4) << "\n";
        *log << "         So 128 bit w3..w0 : " << hex32(w[3]) << hex32(w[2]) << hex32(w[1]) << hex32(w[0]) << "\n";
        *log << "[Buoc 3] Dang PDF          : " << hexBytes(pdfBytes, 16) << "\n";
        *log << "         Dang RFC          : " << hexBytes(rfcBytes, 16) << "\n";
        *log << "         Keystream dung    : " << hexBytes(output, 16) << "   (" << orderName(order) << ")\n";
    }
}

void RabbitCipher::process(const uint8_t* input, uint8_t* output, size_t length) {
    size_t i = 0;
    while (i < length) {
        // Dung het khoi keystream thi sinh khoi moi.
        if (used == 16) {
            generateBlock(keystream);
            used = 0;
        }

        // So byte xu ly bang phan keystream con lai cua khoi hien tai.
        int n = 16 - used;
        if (length - i < (size_t)n) {
            n = length - i;
        }

        // XOR tung byte. Giu lai du lieu vao de ghi log (input co the trung output).
        uint8_t in[16];
        for (int k = 0; k < n; ++k) {
            in[k] = input[i + k];
            output[i + k] = input[i + k] ^ keystream[used + k];
        }

        if (logging()) {
            *log << "  XOR byte " << i << " -> " << i + n - 1 << ":\n";
            *log << "    Du lieu vao : " << hexBytes(in, n) << "\n";
            *log << "    Keystream   : " << hexBytes(keystream + used, n) << "\n";
            *log << "    Du lieu ra  : " << hexBytes(output + i, n) << "\n";
        }

        i += n;
        used += n;
    }
}
