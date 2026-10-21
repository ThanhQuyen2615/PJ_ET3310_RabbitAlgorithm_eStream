#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace FileIO {

    // Doc toan bo file vao `data`. Tra ve false neu khong mo duoc file.
    bool readBinaryFile(const std::string& path, std::vector<uint8_t>& data);

    // Ghi `data` ra file (ghi de). Tra ve false neu loi.
    bool writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data);

} // namespace FileIO
