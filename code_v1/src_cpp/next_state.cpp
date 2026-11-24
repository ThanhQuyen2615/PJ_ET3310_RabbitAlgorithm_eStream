#include "../include/next_state.h"
#include "../include/counter.h"
#include "../include/init.h"
#include "../include/rotate_left.h"

void next_state() {
    counter_system();

    // Tinh het g tu trang thai cu truoc khi ghi de X.
    uint32_t g[8];
    for (int i = 0; i < 8; i++) {
        uint32_t sum = X[i] + C[i];
        uint64_t square = uint64_t(sum) * sum;
        // XOR nua cao va nua thap cua binh phuong.
        g[i] = uint32_t(square >> 32) ^ uint32_t(square);
    }

    for (int i = 0; i < 8; i++) {
        // Lay hai vi tri truoc i theo vong tron.
        int prev = (i + 7) % 8;
        int prev2 = (i + 6) % 8;
        if (i % 2 == 0) {
            X[i] = g[i] + rotate_left(g[prev], 16) + rotate_left(g[prev2], 16);
        } else {
            X[i] = g[i] + rotate_left(g[prev], 8) + g[prev2];
        }
    }
}
