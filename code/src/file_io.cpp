#include "file_io.h"
#include <fstream>
#include <iostream>

namespace FileIO {

    bool readBinaryFile(const std::string& filePath, std::vector<uint8_t>& buffer) {
        buffer.clear();

        // Mo o che do nhi phan de byte khong bi doi; mo o cuoi file de lay kich thuoc.
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "[FileIO Error] Failed to open file for reading: " << filePath << "\n";
            return false;
        }

        std::streamsize fileSize = file.tellg();
        if (fileSize < 0) {
            std::cerr << "[FileIO Error] Failed to determine file size: " << filePath << "\n";
            return false;
        }

        file.seekg(0, std::ios::beg);

        if (fileSize == 0) {
            return true; // File rong
        }

        // Cap phat mot lan va doc het trong mot lan goi.
        buffer.resize(static_cast<size_t>(fileSize));
        if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
            std::cerr << "[FileIO Error] Failed to read stream data from file: " << filePath << "\n";
            buffer.clear();
            return false;
        }

        return true;
    }

    bool writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data) {
        // Che do nhi phan de ky tu xuong dong duoc ghi nguyen ven.
        std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "[FileIO Error] Failed to open file for writing: " << filePath << "\n";
            return false;
        }

        if (!data.empty()) {
            file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        }

        if (!file.good()) {
            std::cerr << "[FileIO Error] An error occurred while writing to file: " << filePath << "\n";
            return false;
        }

        return true;
    }

    int64_t getFileSize(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return -1;
        }
        return static_cast<int64_t>(file.tellg());
    }

    bool fileExists(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        return file.is_open();
    }

} // namespace FileIO
