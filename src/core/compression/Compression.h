#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace rf::compression {

enum class CompressionType {
    Unknown,
    Gzip,
    Bzip2
};

CompressionType detectCompression(
    const std::vector<uint8_t>& payload
);

std::vector<uint8_t> decompressGzip(
    const std::vector<uint8_t>& payload
);

std::vector<uint8_t> decompressBzip2(
    const std::vector<uint8_t>& payload,
    std::size_t expectedSize
);

}
