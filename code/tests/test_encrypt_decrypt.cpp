#include "rabbit.h"
#include "utils.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// So test da chay va so test that bai, dung chung cho moi ham test.
static int totalTests = 0;
static int failedTests = 0;

// In PASS/FAIL cho mot truong hop kiem tra.
static void check(const std::string& name, bool ok) {
    totalTests++;
    if (!ok) failedTests++;
    std::cout << "  " << name << ": " << (ok ? "[PASS]" : "[FAIL]") << "\n";
}

int main() {
    std::cout << "=== TEST MA HOA / GIAI MA ===\n";

    // Cac test se them vao day.

    std::cout << "\nTong: " << totalTests - failedTests << "/" << totalTests << " PASS\n";
    return failedTests == 0 ? 0 : 1;
}
