#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace FileIO {

    // Doc toan bo file vao `buffer`. Tra ve false neu loi.
    bool readBinaryFile(const std::string& filePath, std::vector<uint8_t>& buffer);

    // Ghi `data` ra file (ghi de). Tra ve false neu loi.
    bool writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data);

    // Kich thuoc file (byte), hoac -1 neu khong mo duoc file.
    int64_t getFileSize(const std::string& filePath);

    // True neu file ton tai va mo duoc de doc.
    bool fileExists(const std::string& filePath);

} // namespace FileIO
