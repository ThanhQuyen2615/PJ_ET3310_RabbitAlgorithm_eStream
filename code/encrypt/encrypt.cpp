#include "rabbit.h"
#include "utils.h"
#include "file_io.h"

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

static void printUsage(const char* progName) {
    std::cout << "========================================================\n";
    std::cout << "   Rabbit Stream Cipher (RFC 4503) - ENCRYPTION CLI\n";
    std::cout << "========================================================\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  Option 1 (Using Flags):\n";
    std::cout << "    " << progName << " -k <KEY_HEX> [-iv <IV_HEX>] -i <INPUT_FILE> -o <OUTPUT_FILE>\n\n";
    std::cout << "  Option 2 (Positional Arguments):\n";
    std::cout << "    " << progName << " <INPUT_FILE> <OUTPUT_FILE> <KEY_HEX> [IV_HEX]\n\n";
    std::cout << "PARAMETERS:\n";
    std::cout << "  - KEY_HEX    : 32 hex characters (128 bits = 16 bytes). Mandatory.\n";
    std::cout << "  - IV_HEX     : 16 hex characters (64 bits = 8 bytes). Optional (omit for non-IV mode).\n";
    std::cout << "  - INPUT_FILE : Path to plaintext file (text, pdf, image, binary, etc.).\n";
    std::cout << "  - OUTPUT_FILE: Path to output ciphertext file.\n\n";
    std::cout << "EXAMPLE:\n";
    std::cout << "  " << progName << " -k 912813292E3D36FE3BFC62F1DC51C3AC -iv C373F575C1267E59 -i input/sample.txt -o output/sample.enc\n";
    std::cout << "========================================================\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string keyHex;
    std::string ivHex;
    std::string inputFile;
    std::string outputFile;

    // Check for help flags
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help" || arg == "/?") {
            printUsage(argv[0]);
            return 0;
        }
    }

    // Parse named flags
    bool parsedByFlags = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-k" || arg == "--key") && i + 1 < argc) {
            keyHex = argv[++i];
            parsedByFlags = true;
        } else if ((arg == "-iv" || arg == "--iv") && i + 1 < argc) {
            ivHex = argv[++i];
            parsedByFlags = true;
        } else if ((arg == "-i" || arg == "-in" || arg == "--input") && i + 1 < argc) {
            inputFile = argv[++i];
            parsedByFlags = true;
        } else if ((arg == "-o" || arg == "-out" || arg == "--output") && i + 1 < argc) {
            outputFile = argv[++i];
            parsedByFlags = true;
        }
    }

    // No flags: use positional arguments
    if (!parsedByFlags) {
        if (argc >= 4) {
            inputFile  = argv[1];
            outputFile = argv[2];
            keyHex     = argv[3];
            if (argc >= 5) {
                ivHex = argv[4];
            }
        } else {
            std::cerr << "[Error] Missing required arguments!\n\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    // Validate parameter completeness
    if (inputFile.empty() || outputFile.empty() || keyHex.empty()) {
        std::cerr << "[Error] Please provide Input file, Output file, and Key Hex string!\n\n";
        printUsage(argv[0]);
        return 1;
    }

    // Verify input file existence
    if (!FileIO::fileExists(inputFile)) {
        std::cerr << "[Error] Input file not found: " << inputFile << "\n";
        return 1;
    }

    // Parse and validate 128-bit key (16 bytes)
    std::vector<uint8_t> keyBytes;
    if (!Utils::hexToBytes(keyHex, keyBytes) || keyBytes.size() != RABBIT_KEY_SIZE) {
        std::cerr << "[Error] Invalid Key! Key must be a 32-character Hex string (16 bytes = 128 bits).\n";
        std::cerr << "        Actual bytes parsed: " << keyBytes.size() << " bytes.\n";
        return 1;
    }

    // Parse and validate 64-bit IV (8 bytes) if provided
    std::vector<uint8_t> ivBytes;
    bool useIV = false;
    if (!ivHex.empty()) {
        if (!Utils::hexToBytes(ivHex, ivBytes) || ivBytes.size() != RABBIT_IV_SIZE) {
            std::cerr << "[Error] Invalid IV! IV must be a 16-character Hex string (8 bytes = 64 bits).\n";
            std::cerr << "        Actual bytes parsed: " << ivBytes.size() << " bytes.\n";
            return 1;
        }
        useIV = true;
    }

    std::cout << "--------------------------------------------------------\n";
    std::cout << "STARTING ENCRYPTION PROCESS\n";
    std::cout << "  - Input File  : " << inputFile << "\n";
    std::cout << "  - Output File : " << outputFile << "\n";
    std::cout << "  - Key (Hex)   : " << Utils::bytesToHex(keyBytes) << "\n";
    if (useIV) {
        std::cout << "  - IV (Hex)    : " << Utils::bytesToHex(ivBytes) << "\n";
    } else {
        std::cout << "  - IV          : None (RFC 4503 Non-IV Mode)\n";
    }
    std::cout << "--------------------------------------------------------\n";

    // Read binary input file into memory
    std::vector<uint8_t> data;
    if (!FileIO::readBinaryFile(inputFile, data)) {
        std::cerr << "[Error] Failed to read input file!\n";
        return 1;
    }

    size_t fileSize = data.size();
    std::cout << "-> Successfully read " << fileSize << " bytes from " << inputFile << "\n";

    // Initialize Rabbit cipher instance
    RabbitCipher cipher;
    cipher.setupKey(keyBytes);
    if (useIV) {
        cipher.setupIV(ivBytes);
    }

    // Encrypt and time it
    auto startTime = std::chrono::high_resolution_clock::now();
    cipher.process(data);
    auto endTime = std::chrono::high_resolution_clock::now();

    auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    double durationMs = durationUs / 1000.0;

    // Write encrypted ciphertext to destination
    if (!FileIO::writeBinaryFile(outputFile, data)) {
        std::cerr << "[Error] Failed to write encrypted output file!\n";
        return 1;
    }

    std::cout << "-> Encryption completed. Saved to: " << outputFile << "\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "PERFORMANCE BENCHMARK:\n";
    std::cout << "  - Execution Time : " << std::fixed << std::setprecision(3) << durationMs << " ms (" << durationUs << " us)\n";
    if (durationMs > 0.0 && fileSize > 0) {
        double throughputMBs = (static_cast<double>(fileSize) / (1024.0 * 1024.0)) / (durationMs / 1000.0);
        std::cout << "  - Throughput     : " << std::fixed << std::setprecision(2) << throughputMBs << " MB/s\n";
    }
    std::cout << "========================================================\n";

    return 0;
}
