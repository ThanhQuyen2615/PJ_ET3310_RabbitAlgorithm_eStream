#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace FileIO {

    /**
     * @brief Reads an entire binary file into memory buffer safely.
     * @param filePath Path to target file.
     * @param buffer Output byte vector container.
     * @return true on success, false if file fails to open or read.
     * 
     * [OPTIMIZATION & SAFETY DETAIL]
     * 1. Opens file with `std::ios::binary | std::ios::ate` to seek directly to the end.
     * 2. Retrieves exact file size with `file.tellg()` and pre-allocates buffer memory
     *    in a SINGLE call via `buffer.resize(fileSize)`. This eliminates dynamic vector
     *    reallocations and memory copies.
     * 3. Reads the entire stream in one contiguous block, achieving near-disk-speed I/O.
     */
    bool readBinaryFile(const std::string& filePath, std::vector<uint8_t>& buffer);

    /**
     * @brief Writes a byte buffer to a binary file safely.
     * @param filePath Path to destination file.
     * @param data Byte vector containing payload.
     * @return true on success, false on I/O error.
     * 
     * [SAFETY DETAIL]
     * Opens with `std::ios::binary | std::ios::trunc`. Ensures binary byte streams (e.g. PDF,
     * executable files, images) are never corrupted by newline translations (`\r\n` vs `\n`).
     */
    bool writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data);

    /**
     * @brief Returns size of file in bytes without reading full file content into RAM.
     * @param filePath Path to file.
     * @return File size in bytes, or -1 if file cannot be opened.
     */
    int64_t getFileSize(const std::string& filePath);

    /**
     * @brief Checks if a file exists and is accessible for reading.
     * @param filePath Path to file.
     * @return true if file exists, false otherwise.
     */
    bool fileExists(const std::string& filePath);

} // namespace FileIO
