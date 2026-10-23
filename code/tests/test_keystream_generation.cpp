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

    for(int i = 0; i < 16; i++) {
        if(s0[i] != s1[i]) return true;
    }

    return false;

}

int main() {
    if(test_khoi_lien_tiep()) {
        std::cout << "OK\n";
        return 0;
    }

    std::cout << "Not ok\n";
}