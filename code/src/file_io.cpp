#include "file_io.h"
#include <fstream>
#include <iostream>

namespace FileIO {

    bool readBinaryFile(const std::string& filePath, std::vector<uint8_t>& buffer) {
        buffer.clear();

        // [OPTIMIZATION: Single-pass file sizing]
        // Open file directly in binary mode with the read pointer at the end (ios::ate).
        // This instantly reveals the exact file size via tellg() without reading any bytes.
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

        // Rewind read pointer back to beginning of file
        file.seekg(0, std::ios::beg);

        if (fileSize == 0) {
            // File is empty; return success with empty buffer
            return true;
        }

        // [OPTIMIZATION: Single buffer memory allocation]
        // Resize buffer exactly once. Reading the whole file in a single contiguous block
        // minimizes filesystem syscalls and eliminates vector reallocations.
        buffer.resize(static_cast<size_t>(fileSize));
        if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
            std::cerr << "[FileIO Error] Failed to read stream data from file: " << filePath << "\n";
            buffer.clear();
            return false;
        }

        return true;
    }

    bool writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data) {
        // [SAFETY: Pure Binary Mode]
        // Open file in binary mode with truncation (ios::binary | ios::trunc).
        // This guarantees no carriage-return translations occur, preserving PDF/media integrity.
        std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "[FileIO Error] Failed to open file for writing: " << filePath << "\n";
            return false;
        }

        // [OPTIMIZATION: Contiguous block write]
        // Direct write from the underlying contiguous buffer memory via data.data()
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
