#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace eld::binary {

enum class CompressionType {
    Unknown,
    Gzip,
    Bzip2
};

CompressionType detectCompression(
    const std::vector<std::uint8_t>& payload
);

std::vector<std::uint8_t> decompressGzip(
    const std::vector<std::uint8_t>& payload
);

std::vector<std::uint8_t> decompressBzip2(
    const std::vector<std::uint8_t>& payload,
    std::size_t expectedSize
);

}
