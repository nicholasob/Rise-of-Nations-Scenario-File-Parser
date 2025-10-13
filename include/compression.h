#pragma once
#include <vector>
#include <cstdint>

class GzipHelper {
public:
    static bool isGzip(const std::vector<uint8_t>& data);
    static bool decompressGzip(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);
    static bool compressGzip(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);
};
