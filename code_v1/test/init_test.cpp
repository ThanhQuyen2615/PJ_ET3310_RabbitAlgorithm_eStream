#include "../include/init.h"
#include <ctime>
int main() {

    //A dung, C va X mac dinh = 0
    const unsigned int A_check[8] = {
        0x4D34D34D, 0xD34D34D3, 0x34D34D34, 0x4D34D34D,
        0xD34D34D3, 0x34D34D34, 0x4D34D34D, 0xD34D34D3
    };

    //sinh gia tri random
    for (int i = 0; i < 8; ++i){
        A[i] = rand() % 0xFFFFFFFF;
        C[i] = rand() % 0xFFFF;
        X[i] = rand() % 0xFFFF;
    }

    for (int i = 0; i < 8; ++i) {
        cout << "A [" << i << "] =" << hex << A[i] << ", C [" << i << "] = " << hex << C[i] << ", X [" << i << "] = " << hex << X[i] << endl;
    }

    init();

    int loi = 0;
    for (int i = 0; i < 8; ++i) {
        if (A[i] != A_check[i]) { printf("FAIL A[%d] = %08X, dung la %08X\n", i, A[i], A_check[i]); loi++; }
        if (X[i] != 0)         { printf("FAIL X[%d] = %08X\n", i, X[i]); loi++; }
        if (C[i] != 0)         { printf("FAIL C[%d] = %08X\n", i, C[i]); loi++; }
    }

    printf(loi == 0 ? "init: PASS\n" : "init: FAIL\n");
    return loi == 0 ? 0 : 1;
}
