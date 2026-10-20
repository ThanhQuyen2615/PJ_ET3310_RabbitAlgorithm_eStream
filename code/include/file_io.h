#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace FileIO {

    // Read a whole file into `buffer`. Returns false on error.
    bool readBinaryFile(const std::string& filePath, std::vector<uint8_t>& buffer);

    // Write `data` to a file, overwriting it. Returns false on error.
    bool writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data);

    // File size in bytes, or -1 if the file cannot be opened.
    int64_t getFileSize(const std::string& filePath);

    // True if the file exists and can be opened for reading.
    bool fileExists(const std::string& filePath);

} // namespace FileIO
