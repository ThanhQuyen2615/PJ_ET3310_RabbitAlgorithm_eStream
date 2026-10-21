#include "file_io.h"
#include <fstream>

namespace FileIO {

    bool readBinaryFile(const std::string& path, std::vector<uint8_t>& data) {
        // Mo o che do nhi phan de byte khong bi doi.
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return false;
        }

        // Nhay toi cuoi file de biet kich thuoc, roi quay ve dau file.
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Doc ca file trong mot lan.
        data.resize(size);
        file.read((char*)data.data(), size);
        return true;
    }

    bool writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data) {
        // Mo o che do nhi phan; file cu (neu co) bi ghi de.
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            return false;
        }

        file.write((const char*)data.data(), data.size());
        return file.good();
    }

} // namespace FileIO
