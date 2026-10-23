#include <iostream>
#include <cstdint>

#include "rabbit.h"
#include "utils.h"

bool test_khoi_lien_tiep() {
    uint8_t key[16] = {0};
    uint8_t s0[16];
    uint8_t s1[16];

    RabbitCipher rabbit(ByteOrder::PDF);
    rabbit.setupKey(key);
    rabbit.generateBlock(s0);
    rabbit.generateBlock(s1);

    Utils::printHex("s[0]", s0, 16);
    Utils::printHex("s[1]", s1, 16);

    for(int i = 0; i < 16; i++) {
        if(s0[i] != s1[i]) return true;
    }

    return false;

}

bool test_same_secretKey() {
    uint8_t key[16] = {
        0xC2, 0x1F, 0xCF, 0x38, 0x81, 0xCD, 0x5E, 0xE8,
        0x62, 0x8A, 0xCC, 0xB0, 0xA9, 0x89, 0x0D, 0xF8
    };
    
    uint8_t s1[16];
    uint8_t s2[16];

    RabbitCipher rabbit1(ByteOrder::PDF);
    RabbitCipher rabbit2(ByteOrder::PDF);

    rabbit1.setupKey(key);
    rabbit2.setupKey(key);

    for(int i = 0; i < 5; i++) {
        rabbit1.generateBlock(s1);
        rabbit2.generateBlock(s2);
        for(int j = 0; j < 16; j++) {
            if(s1[j] != s2[j]) {
                std::cout << "Khac nhau tai khoi " << i << ", byte " << j << "\n";
                Utils::printHex("Mong doi theo rabbit1", s1, 16);
                Utils::printHex("Thuc te cua rabbit2", s2, 16);

                return false;
            }
        }
    }

    return true;
}

int main() {
    if(test_khoi_lien_tiep()) {
        std::cout << "2 khoi keystream lien tiep da khac nhau !\n";
    }
    else std::cout << "Not ok! 2 khoi keystream lien tiep giong nhau!\n";

    if(test_same_secretKey()) {
        std::cout << "Cung sesret-key da sinh giong nhau 5 khoi! OK\n";

    }
    else std::cout << "Cung secret-key ma sinh khac nhau! Not ok !\n";
}