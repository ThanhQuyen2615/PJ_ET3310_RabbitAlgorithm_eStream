#include "rabbit.h"
#include "utils.h"
#include "file_io.h"

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>

// Cach dung: encrypt_app <file_vao> <file_ra> <khoa_hex> [iv_hex]
// Tung buoc tinh toan duoc ghi vao file <file_ra>.log.txt, khong in ra man hinh.
int main(int argc, char* argv[]) {

    // Kiem tra so luong tham so dau vao
    if (argc < 4) {
        std::cout << "Usage:   " << argv[0] << " <input_file> <output_file> <key_hex> [iv_hex]\n";
        std::cout << "  key_hex: 32 hex characters (16 bytes), iv_hex: 16 hex characters (8 bytes)\n";
        std::cout << "Example: " << argv[0] << " input/sample.txt output/sample.enc "
                  << "912813292E3D36FE3BFC62F1DC51C3AC C373F575C1267E59\n";
        return 1;
    }
    std::string inputFile  = argv[1];
    std::string outputFile = argv[2];

    // Doc va kiem tra khoa 128 bit (16 byte)
    std::vector<uint8_t> key;
    if (!Utils::hexToBytes(argv[3], key) || key.size() != 16) {
        std::cerr << "[Error] Key must be 32 hex characters (16 bytes).\n";
        return 1;
    }

    // Doc va kiem tra IV 64 bit (8 byte) neu co
    std::vector<uint8_t> iv;
    if (argc >= 5 && (!Utils::hexToBytes(argv[4], iv) || iv.size() != 8)) {
        std::cerr << "[Error] IV must be 16 hex characters (8 bytes).\n";
        return 1;
    }

    // Doc file dau vao vao bo nho
    std::vector<uint8_t> data;
    if (!FileIO::readBinaryFile(inputFile, data)) {
        std::cerr << "[Error] Cannot read input file: " << inputFile << "\n";
        return 1;
    }

    // Mo file log
    std::string logPath = outputFile + ".log.txt";
    std::ofstream logFile(logPath);
    logFile << "RABBIT - MA HOA\n";
    logFile << "File vao : " << inputFile << " (" << data.size() << " byte)\n";
    logFile << "File ra  : " << outputFile << "\n";
    logFile << "Khoa     : " << argv[3] << "\n";
    logFile << "IV       : " << (iv.empty() ? "khong dung IV" : argv[4]) << "\n";

    // Khoi tao bo ma Rabbit (ghi log 4 khoi keystream dau tien)
    RabbitCipher cipher;
    cipher.setLog(&logFile);
    cipher.setupKey(key.data());
    if (!iv.empty()) {
        cipher.setupIV(iv.data());
    }

    // Ma hoa va do thoi gian
    std::clock_t start = std::clock();
    cipher.process(data.data(), data.data(), data.size());
    double ms = 1000.0 * (std::clock() - start) / CLOCKS_PER_SEC;

    // Ghi ban ma ra file
    if (!FileIO::writeBinaryFile(outputFile, data)) {
        std::cerr << "[Error] Cannot write output file: " << outputFile << "\n";
        return 1;
    }

    logFile << "\nTONG KET: da ma hoa " << data.size() << " byte = " << (data.size() + 15) / 16
            << " khoi keystream, thoi gian " << ms << " ms\n";

    std::cout << "Encrypted " << data.size() << " bytes: " << inputFile << " -> " << outputFile << "\n";
    std::cout << "IV: " << (iv.empty() ? "none" : Utils::bytesToHex(iv.data(), iv.size())) << "\n";
    std::cout << "Time: " << ms << " ms\n";
    std::cout << "Log: " << logPath << "\n";
    return 0;
}
