#include "../include/next_state.h"
#include "../include/init.h"
#include <iostream>
using namespace std;

bool runTest(const char* name, int type, int rounds,
             const uint32_t expected[8]) {
    init();

    for (int i = 0; i < 8; i++) {
        if (type == 1) X[i] = 0xFFFFFFFF;
        if (type == 2) X[i] = i + 1;
    }

    for (int i = 0; i < rounds; i++)
        next_state();

    cout << name << endl;
    int passed = 0;

    for (int i = 0; i < 8; i++) {
        cout << "X[" << i << "] ";
        cout << "Expected: 0x" << hex << expected[i];
        cout << " Actual: 0x" << X[i];

        if (X[i] == expected[i]) {
            cout << " PASS" << endl;
            passed++;
        }
        else
            cout << " FAIL" << endl;
    }

    cout << dec << "Passed: " << passed << "/8\n\n";
    return passed == 8;
}

int main() {
    uint32_t t1[8] = {
        0xD1D9A5DF, 0x144F9F2A, 0x82C7A609, 0xB58C6651,
        0xBC3D6C93, 0xEF19638D, 0xD20356CD, 0x787A3CCA
    };

    uint32_t t2[8] = {
        0x56BF09F1, 0xB52FD266, 0x486FBB4B, 0x3C9E18CB,
        0x1DCFE5EB, 0x976DE8D1, 0x0818FBA2, 0x7D9D7034
    };

    uint32_t t3[8] = {
        0x9618F871, 0x27AB1BFB, 0x9C4068C9, 0xC823CAC7,
        0x3C6C3D54, 0x0449E6D1, 0xA6E38A48, 0x5012FB1A
    };

    uint32_t t4[8] = {
        0x2824F5C9, 0x4A33BAD6, 0x87333564, 0x05ECE506,
        0x697EEC69, 0xFF210ACB, 0xF08EB82C, 0xEECF5C87
    };

    int failed = 0;
    if (!runTest("Test 1: X = 0", 0, 1, t1)) failed++;
    if (!runTest("Test 2: X lon nhat", 1, 1, t2)) failed++;
    if (!runTest("Test 3: X[i] = i + 1", 2, 1, t3)) failed++;
    if (!runTest("Test 4: Cap nhat 2 lan", 0, 2, t4)) failed++;

    return failed == 0 ? 0 : 1;
}

