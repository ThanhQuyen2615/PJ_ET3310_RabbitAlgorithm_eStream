#pragma once
#include "init.h"

//ham khoi tao lai gia tri ban dau
void init(){
    for (int i = 0; i < 8; ++i){
        X[i] = C[i] = 0;
    }
    A[0] = A[3] = A[6] = 0x4D34D34D;
	A[1] = A[4] = A[7] = 0xD34D34D3;
	A[2] = A[5] = 0x34D34D3;
}